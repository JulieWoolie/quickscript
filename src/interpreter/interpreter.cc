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

void InstructionBuf::getInstruction(opcode* code, uint8** args, uint32 instrIndex) const {
  uint64 offset = instrIndex;
  offset *= LENGTH_INSTRUCTION;

  if (offset > m_len) {
    *code = OP_NOP;
    return;
  }

  *args = m_buf + offset + LENGTH_OPCODE;
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

static void addFunctionEntries(
  const TypeTable& types,
  const BytecodeFile& file,
  const InstructionRewrite& rewrites,
  std::vector<ScriptFunction>& functions
) {
  FunctionTableEntry* funcTable = file.funcTable;
  const uint32 funcCount = file.funcTableEntries;

  for (uint32 i = 0; i < funcCount; i++) {
    FunctionTableEntry* fte = &funcTable[i];

    const uint32 firstInstr = fte->startingInstruction + rewrites.jumpAddrOffset;
    const uint64 nameAddr = rewrites.stringRewrites.findReplacement(fte->nameOffset);

    FunctionSignature* sign = static_cast<FunctionSignature*>(types.lookupByIndex(rewrites.typeRewrites.findRewritten(fte->signatureIndex)));

    functions.emplace_back(firstInstr, nameAddr, fte->stackSize, sign);
  }
}

uint32 VirtualMachine::addBytecodeFile(const BytecodeFile& file) {
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

  addFunctionEntries(m_types, file, instrRewrite, m_functions);
  rewriteInstructions(m_instrBuf.getBuffer() + jumpAddrOff, file.instructionsSize, instrRewrite);

  return file.entryPointIndex + funcIdxOffset;
}

int32 VirtualMachine::beginExecution(const uint32 funcEntryIdx, const ProgramArgs& args) {
  const ScriptFunction& func = m_functions[funcEntryIdx];

  uint64 argsAddr;

  if (args.count == 0) {
    argsAddr = 0;
  } else {
    QsArray arr = m_heap.allocConstArray(args.count, POINTER_SIZE);
    argsAddr = arr.address();

    for (uint32 i = 0; i < args.count; i++) {
      const uint32 start = args.starts[i];
      const uint32 len = args.lengths[i];

      QsArray str = m_heap.allocConstString(len);
      memcpy(str.data, args.cdata + start, len);

      arr.u64At(i) = str.address();
    }
  }

  Interpreter interp = Interpreter(*this);
  return interp.beginExecution(func, argsAddr);
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

#define READ_I8ARG(off) *reinterpret_cast<int8*>(args + off)
#define READ_U8ARG(off) args[off]
#define READ_I16ARG(off) *reinterpret_cast<int16*>(args + off)
#define READ_U16ARG(off) *reinterpret_cast<uint16*>(args + off)
#define READ_I32ARG(off) *reinterpret_cast<int32*>(args + off)
#define READ_U32ARG(off) *reinterpret_cast<uint32*>(args + off)
#define READ_I64ARG(off) *reinterpret_cast<int64*>(args + off)
#define READ_U64ARG(off) *reinterpret_cast<uint64*>(args + off)
#define READ_F32ARG(off) *reinterpret_cast<float32*>(args + off)
#define READ_F64ARG(off) *reinterpret_cast<float64*>(args + off)

#define READ_U8(buf, off) buf[off]
#define WRITE_U8(buf, off, val) *reinterpret_cast<uint8*>(buf + off) = val
#define READ_U16(buf, off) *reinterpret_cast<uint16*>(buf + off)
#define WRITE_U16(buf, off, val) *reinterpret_cast<uint16*>(buf + off) = val
#define READ_U32(buf, off) *reinterpret_cast<uint32*>(buf + off)
#define WRITE_U32(buf, off, val) *reinterpret_cast<uint32*>(buf + off) = val
#define READ_U64(buf, off) *reinterpret_cast<uint64*>(buf + off)
#define WRITE_U64(buf, off, val) *reinterpret_cast<uint64*>(buf + off) = val

#define REG_AS(reg, type) *reinterpret_cast<type*>(&m_registers[reg])

Interpreter::Interpreter(VirtualMachine& vm) : m_vm(vm) {

}

Interpreter::~Interpreter() {

}

CallFrame* Interpreter::getCallFrame(const uint32 off) {
  if (off > m_frameCount) {
    return nullptr;
  }

  const uint32 idx = m_frameCount - (off + 1);
  return &m_callFrames[idx];
}

CallFrame* Interpreter::pushNewFrame() {
  if (m_frameCount >= MAX_CALL_DEPTH) {
    return nullptr;
  }

  CallFrame* frame = &m_callFrames[m_frameCount];
  m_frameCount++;

  frame->line = 0;
  frame->allocatedSize = 0;
  frame->returnAddr = NO_RETURN_ADDR;
  frame->stackBase = nullptr;
  frame->filename = "";
  frame->name = "";

  return frame;
}

void Interpreter::popCallFrame() {
  if (m_frameCount == 0) {
    return;
  }

  CallFrame* frame = getCallFrame();

  m_stack.popFrame(frame->allocatedSize);

  frame->allocatedSize = 0;
  frame->stackBase = nullptr;

  m_frameCount--;
}

VirtualMachine& Interpreter::getVirtualMachine() const {
  return m_vm;
}

int32 Interpreter::beginExecution(const ScriptFunction& func, const uint64 argsArrayAddr) {
  const StringPool& strPool = m_vm.getStringPool();
  const uint32 nameLen = strPool.getLength(func.nameOffset);
  const int8* nameContent = strPool.getCharacterData(func.nameOffset);

  CallFrame* frame = pushNewFrame();
  frame->name = std::string(nameContent, nameLen);

  uint8* stackFrame = m_stack.allocateFrame(func.stackSize);
  frame->stackBase = stackFrame;
  frame->allocatedSize = func.stackSize;

  // Write to offset 4, since offset 0..3 is the int32 return value
  WRITE_U64(stackFrame, 4, argsArrayAddr);

  m_instrCount = func.firstInstrIndex;

  run();

  const int32 retVal = READ_U32(stackFrame, 0);
  popCallFrame();

  return retVal;
}

static uint64 stringRepeat(void* strAddr, const uint32 repeats, HeapMemory& heap) {
  const QsArray baseString = castToQsArray(strAddr);
  const uint32 len = baseString.length;
  const uint32 newLen = len * repeats;

  const QsArray newArr = heap.allocString(newLen);
  for (uint32 i = 0; i < repeats; i++) {
    memcpy(newArr.data + (len * i), baseString.data, len);
  }

  return newArr.address();
}

void Interpreter::run() {
  opcode code = OP_NOP;
  uint8* args = nullptr;

  uint8* stack = nullptr;
  uint8* global = m_vm.getGlobalMemory().getData();

  CallFrame* frame = nullptr;

  begin:
  m_vm.getInstructions().getInstruction(&code, &args, m_instrCount);
  frame = getCallFrame();

  if (!frame) {
    return;
  }

  stack = frame->stackBase;

  switch (code) {
    /*
     * TODO:
     *   - X PUSHLINE
     *   - X RET
     *   - X JMP
     *   - X JMPI0
     *   - X JMPN0
     *   - _ LOADCONSTSTR
     *   - _ HEAPALLOC
     *   - _ HEAPFREE
     *   - _ READIDX8
     *   - _ READIDX16
     *   - _ READIDX32
     *   - _ READIDX64
     *   - _ WRITEIDX8
     *   - _ WRITEIDX16
     *   - _ WRITEIDX32
     *   - _ WRITEIDX64
     *   - _ SETARGTYPE
     *   - _ LFUNCLOOKUP
     *   - _ NFUNCLOOKUP
     *   - _ INVOKEV
     *   - _ INVOKE8
     *   - _ INVOKE16
     *   - _ INVOKE32
     *   - _ INVOKE64
     *   - _ EQARR
     *   - _ EQSTRUCT
     *   - _ NEQARR
     *   - _ NEQSTRUCT
     *   - _ GTARR
     *   - _ GTEARR
     *   - _ LTARR
     *   - _ LTEARR
     *   - _ STRCONCAT
     */

    case OP_RET:
      if (frame->returnAddr == NO_RETURN_ADDR) {
        return;
      }
      m_instrCount = frame->returnAddr;
      popCallFrame();
      goto begin;

    case OP_PUSHLINE:
      frame->line = READ_U32ARG(0);
      break;
    case OP_JMP:
      m_instrCount = READ_U32ARG(0);
      goto begin;
    case OP_JMPI0:
      if (m_registers[args[4]]) {
        break;
      }
      m_instrCount = READ_U32ARG(0);
      goto begin;
    case OP_JMPN0:
      if (!m_registers[args[4]]) {
        break;
      }
      m_instrCount = READ_U32ARG(0);
      goto begin;

    // region SIMPLE_INSTRUCTIONS
#include "evaluator.cc"
    // endregion
    default:
      break;
  }

  m_instrCount++;
  goto begin;
}