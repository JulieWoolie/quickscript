#include "interpreter.h"

#include <format>

InstructionBuf::InstructionBuf() {

}

void InstructionBuf::insertInstructions(const uint8* instrBuf, const uint64 len) {
  const uint64 newLen = m_len + len;

  if (newLen > m_cap) {
    uint8* newBuf = static_cast<uint8*>(realloc(m_buf, newLen));

    if (!newBuf) {
      throw std::runtime_error("Failed to increase instruction buffer size");
    }

    m_buf = newBuf;
    m_cap = newLen;
  }

  memcpy(m_buf + m_len, instrBuf, len);
  m_len = newLen;
}

void InstructionBuf::getInstruction(Instruction* out, const uint32 instrIndex) const {
  uint64 offset = instrIndex;
  offset *= LENGTH_INSTRUCTION;

  if (offset > m_len) {
    out->code = OP_NOP;
    return;
  }

  memcpy(out, m_buf + offset, LENGTH_INSTRUCTION);
}

uint64 InstructionBuf::length() const {
  return m_len;
}

uint64 InstructionBuf::capacity() const {
  return m_cap;
}

uint8* InstructionBuf::getBuffer() const {
  return m_buf;
}

uint64 InstructionBuf::getInstructionCount() const {
  return m_instrCount;
}

GlobalMemorySpace::GlobalMemorySpace() {

}

void GlobalMemorySpace::grow(const uint64 bytes) {
  const uint64 newSize = m_size + bytes;
  uint8* newData = static_cast<uint8*>(realloc(m_data, newSize));

  if (!newData) {
    throw std::runtime_error("Failed to grow global memory space");
  }

  m_data = newData;
  m_size = newSize;
}

uint8* GlobalMemorySpace::getData() const {
  return m_data;
}

uint64 GlobalMemorySpace::size() const {
  return m_size;
}

VirtualMachine::VirtualMachine() {

}

VirtualMachine::~VirtualMachine() {

}

struct TypeRewrite {
  typeindex replace;
  typeindex replaceWith;
};

struct TypeReindexList {
  std::vector<TypeRewrite> rewrites;

  void submit(typeindex prev, typeindex newIdx) {
    if (prev == newIdx) {
      return;
    }
    rewrites.emplace_back(prev, newIdx);
  }

  typeindex findRewritten(const typeindex idx) const {
    for (const TypeRewrite& rewrite : rewrites) {
      if (rewrite.replace != idx) {
        return rewrite.replaceWith;
      }
    }
    return idx;
  }
};

struct InstructionRewrite {
  PoolOffsetRewrite& stringRewrites;
  TypeReindexList& typeRewrites;
  uint32 jumpAddrOffset = 0;
  uint64 globalMemOffset = 0;
  uint32 funcIdxOffset = 0;
};

static void rewriteStringRef(uint64* ptr, const PoolOffsetRewrite& offRewrite) {
  const uint64 off = *ptr;

  for (uint32 rewriteIdx = 0; rewriteIdx < offRewrite.len; rewriteIdx++) {
    const uint64 replace = offRewrite.replacedOffset[rewriteIdx];
    if (replace != off) {
      continue;
    }
    *ptr = offRewrite.replaceWith[rewriteIdx];
    break;
  }
}

static void rewriteInstructions(uint8* buf, const uint64 len, const InstructionRewrite& rewrite) {
  for (uint64 i = 0; i < len; i += LENGTH_INSTRUCTION) {
    const opcode code = *reinterpret_cast<opcode*>(buf + i);

    switch (code) {
      case OP_GREAD8:
      case OP_GREAD16:
      case OP_GREAD32:
      case OP_GREAD64:
      case OP_GWRITE8:
      case OP_GWRITE16:
      case OP_GWRITE32:
      case OP_GWRITE64: {
        uint64* offsetPtr = reinterpret_cast<uint64*>(buf + i + LENGTH_OPCODE + 1);
        *offsetPtr += rewrite.globalMemOffset;
        break;
      }

      case OP_GSTORECONST8:
      case OP_GSTORECONST16:
      case OP_GSTORECONST32:
      case OP_GSTORECONST64: {
        uint64* offsetPtr = reinterpret_cast<uint64*>(buf + i + LENGTH_OPCODE);
        *offsetPtr += rewrite.globalMemOffset;
        break;
      }

      case OP_JMP:
      case OP_JMPN0:
      case OP_JMPI0: {
        uint32* addrPtr = reinterpret_cast<uint32*>(buf + i + LENGTH_OPCODE);
        *addrPtr += rewrite.jumpAddrOffset;
        break;
      }

      case OP_LOADCONSTSTR: {
        uint64* strPtr = reinterpret_cast<uint64*>(buf + i + LENGTH_OPCODE + 1);
        rewriteStringRef(strPtr, rewrite.stringRewrites);
        break;
      }

      case OP_SETARGTYPE: {
        uint32* typeIndexPtr = reinterpret_cast<uint32*>(buf + i + LENGTH_OPCODE + 4);
        *typeIndexPtr = rewrite.typeRewrites.findRewritten(*typeIndexPtr);
        break;
      }

      case OP_NFUNCLOOKUP: {
        uint32* typeIdxPtr = reinterpret_cast<uint32*>(buf + i + LENGTH_OPCODE);
        uint64* strPtr = reinterpret_cast<uint64*>(typeIdxPtr + 1);

        *typeIdxPtr = rewrite.typeRewrites.findRewritten(*typeIdxPtr);
        rewriteStringRef(strPtr, rewrite.stringRewrites);

        break;
      }
      case OP_LFUNCLOOKUP: {
        uint32* funcIdxPtr = reinterpret_cast<uint32*>(buf + LENGTH_OPCODE);
        *funcIdxPtr += rewrite.funcIdxOffset;
        break;
      }

      default:
        break;
    }
  }
}

static void loadTypes(TypeTable& table, const BytecodeFile& file, TypeReindexList& out) {
  const uint64 typeTableSize = file.typeTableSize;
  TypeTableEntry** typeTable = file.typeTable;

  // Pass 1: struct init
  for (uint64 i = 0; i < typeTableSize; i++) {
    TypeTableEntry* entry = typeTable[i];
    if (entry->type != TYPE_TABLE_STRUCT) {
      continue;
    }

    const TypeTableStruct* structType = static_cast<TypeTableStruct*>(entry);

    const uint32 nameLen = STRPOOL_LEN(file.constStringPool, structType->nameOffset);
    const int8* nameData = STRPOOL_DATA(file.constStringPool, structType->nameOffset);

    std::string nameString = std::string(nameData, nameLen);

    if (table.lookupByName(nameString)) {
      throw std::runtime_error(std::format("Struct with name {} already exists, can't load again", nameString));
    }

    ScriptStructType* scriptType = ScriptStructType::create(nameString, nullptr, structType->propertyCount);
    typeindex newIdx = table.emplaceType(scriptType);

    if (entry->index != newIdx) {
      out.submit(entry->index, newIdx);
    }
  }

  // Pass 2: Type loading
  for (uint64 i = 0; i < typeTableSize; i++) {
    TypeTableEntry* entry = typeTable[i];

    switch (entry->type) {
      case TYPE_TABLE_SIGNATURE: {
        TypeTableFuncSign* sign = static_cast<TypeTableFuncSign*>(entry);

        ScriptType* retType = table.lookupByIndex(out.findRewritten(sign->index));
        bool variadic = sign->varargs;

        const uint32 argCount = sign->argumentCount;
        ScriptType* argTypes[argCount];

        for (uint32 arg = 0; arg < argCount; arg++) {
          argTypes[arg] = table.lookupByIndex(out.findRewritten(sign->argTypes[arg]));
        }

        FunctionSignature* signature = table.getSignature(retType, variadic, argCount, argTypes);
        typeindex newIdx = table.findIndex(signature);

        out.submit(entry->index, newIdx);
        break;
      }
      case TYPE_TABLE_ARRAY: {
        TypeTableArray* arr = static_cast<TypeTableArray*>(entry);
        ScriptType* compType = table.lookupByIndex(out.findRewritten(arr->componentType));
        ScriptType* arrType = table.getArrayType(compType);

        out.submit(entry->index, table.findIndex(arrType));
        break;
      }
      case TYPE_TABLE_STRUCT: {
        const TypeTableStruct* structType = static_cast<TypeTableStruct*>(entry);
        const typeindex newIdx = out.findRewritten(entry->index);
        const ScriptStructType* type = static_cast<ScriptStructType*>(table.lookupByIndex(newIdx));

        if (type->getPropertyCount() != structType->propertyCount) {
          break;
        }

        for (uint32 propIdx = 0; propIdx < structType->propertyCount; propIdx++) {
          StructProperty* prop = type->getProperty(propIdx);
          TypeTableStructProperty* tableProp = &structType->properties[propIdx];

          const uint32 propNameLen = STRPOOL_LEN(file.constStringPool, tableProp->nameOffset);
          const int8* propNameData = STRPOOL_DATA(file.constStringPool, tableProp->nameOffset);

          prop->name = std::string(propNameData, propNameLen);
          prop->type = table.lookupByIndex(out.findRewritten(tableProp->type));
        }

        break;
      }
      default:
        break;
    }
  }
}

void VirtualMachine::addBytecodeFile(const BytecodeFile& file) {
  const uint64 globalMemOff = m_globalMem.size();
  const uint32 jumpAddrOff = m_instrBuf.length();
  const uint32 funcIdxOffset = m_functions.size();

  m_globalMem.grow(file.globalScopeSize);
  m_instrBuf.insertInstructions(file.instructionBuf, file.instructionsSize);

  PoolOffsetRewrite rewrite = m_stringPool.emplacePoolData(file.constStringPool, file.stringPoolSize);

  TypeReindexList typeRewrites;
  loadTypes(m_types, file, typeRewrites);

  const InstructionRewrite instrRewrite = {
    .stringRewrites = rewrite,
    .typeRewrites = typeRewrites,
    .jumpAddrOffset = jumpAddrOff,
    .globalMemOffset = globalMemOff,
    .funcIdxOffset = funcIdxOffset
  };

  rewriteInstructions(m_instrBuf.getBuffer() + jumpAddrOff, file.instructionsSize, instrRewrite);
}

TypeTable& VirtualMachine::getTypes() {
  return m_types;
}

StringPool& VirtualMachine::getStringPool() {
  return m_stringPool;
}

HeapMemory& VirtualMachine::getHeap() {
  return m_heap;
}

InstructionBuf& VirtualMachine::getInstructions() {
  return m_instrBuf;
}

GlobalMemorySpace& VirtualMachine::getGlobalMemory() {
  return m_globalMem;
}

std::vector<ScriptFunction>& VirtualMachine::getFunctions() {
  return m_functions;
}

InterpreterState::InterpreterState(VirtualMachine& vm) : m_vm(vm) {

}

InterpreterState::~InterpreterState() {

}

CallFrame* InterpreterState::getCallFrame(const uint32 off) {
  if (off > m_frameCount) {
    return nullptr;
  }

  const uint32 idx = m_frameCount - (off + 1);
  return &m_callFrames[idx];
}

CallFrame* InterpreterState::pushNewFrame() {
  if (m_frameCount >= MAX_CALL_DEPTH) {
    return nullptr;
  }

  CallFrame* frame = &m_callFrames[m_frameCount];
  m_frameCount++;

  frame->line = 0;
  frame->allocatedSize = 0;
  frame->returnAddr = 0;
  frame->stackBase = nullptr;
  frame->filename = "";
  frame->name = "";

  return frame;
}

void InterpreterState::popCallFrame() {
  if (m_frameCount == 0) {
    return;
  }
  m_frameCount--;
}

VirtualMachine& InterpreterState::getVirtualMachine() const {
  return m_vm;
}
