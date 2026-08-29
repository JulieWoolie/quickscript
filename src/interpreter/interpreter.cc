#include "interpreter.h"

#include <format>

#include "script_error.h"

#define TO_POINTER(expr) reinterpret_cast<void*>(expr)
#define qsArrayFromAddr(expr) castToQsArray(TO_POINTER(expr))
#define qsObjectFromAddr(expr) castToQsObject(TO_POINTER(expr))

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

void InstructionBuf::getInstruction(opcode* code, uint8 args[], uint32 instrIndex) const {
  uint64 offset = instrIndex;
  offset *= LENGTH_INSTRUCTION;

  if (offset > m_len) {
    *code = OP_NOP;
    return;
  }

  *code = *reinterpret_cast<opcode*>(m_buf + offset);
  memcpy(args, m_buf + offset + LENGTH_OPCODE, LENGTH_ARGS);
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

functype LocalScriptFunction::ftype() const {
  return FUNCTYPE_LOCAL;
}

functype NativeScriptFunction::ftype() const {
  return FUNCTYPE_NATIVE;
}

VirtualMachine::VirtualMachine() {

}

VirtualMachine::~VirtualMachine() {

}

void VirtualMachine::addBindings(const BindingsObject* object) {
  const std::vector<NativeBinding*>& bindings = object->getBindings();
  for (NativeBinding* bind : bindings) {
    if (bind->btype() != BINDTYPE_FUNCTION) {
      continue;
    }

    const NativeFunctionBinding* nfb = static_cast<NativeFunctionBinding*>(bind);
    FunctionSignature* sign = m_types.copySignatureIntoTable(nfb->getSignature());

    NativeScriptFunction nFunc = NativeScriptFunction();
    nFunc.signature = sign;
    nFunc.name = nfb->getName();
    nFunc.callback = nfb->getFunction();

    m_nativeFunctions.push_back(nFunc);
  }
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
        continue;
      }
      return rewrite.replaceWith;
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
        uint32* funcIdxPtr = reinterpret_cast<uint32*>(buf + i + LENGTH_OPCODE);
        *funcIdxPtr += rewrite.funcIdxOffset;
        break;
      }

      case OP_STRCONCAT: {
        uint32* typeIdxPtr = reinterpret_cast<uint32*>(buf + i + LENGTH_OPCODE + 2);
        *typeIdxPtr = rewrite.typeRewrites.findRewritten(*typeIdxPtr);
        break;
      }

      case OP_ARRAYALLOC: {
        uint32* typeIdxPtr = reinterpret_cast<uint32*>(buf + i + LENGTH_OPCODE + 5);
        *typeIdxPtr = rewrite.typeRewrites.findRewritten(*typeIdxPtr);
        break;
      }

      case OP_GTARR:
      case OP_GTEARR:
      case OP_LTARR:
      case OP_LTEARR:
      case OP_EQARR:
      case OP_NEQARR: {
        uint32* typeIdxPtr = reinterpret_cast<uint32*>(buf + i + LENGTH_OPCODE + 3);
        *typeIdxPtr = rewrite.typeRewrites.findRewritten(*typeIdxPtr);
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
    const typeindex newIdx = table.emplaceType(scriptType);

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

        ScriptType* retType = table.lookupByIndex(out.findRewritten(sign->returnType));
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

static void toStandardFilename(const std::string& fname, std::string& out) {
  for (uint32 i = 0; i < fname.length(); i++) {
    const int8 ch = fname[i];
    if (ch == '\\') {
      out.push_back('/');
      continue;
    }
    out.push_back(ch);
  }
}

static void addFunctionEntries(
  const TypeTable& types,
  const BytecodeFile& file,
  const InstructionRewrite& rewrites,
  std::vector<LocalScriptFunction>& functions,
  const std::string& filename
) {
  FunctionTableEntry* funcTable = file.funcTable;
  const uint32 funcCount = file.funcTableEntries;

  for (uint32 i = 0; i < funcCount; i++) {
    FunctionTableEntry* fte = &funcTable[i];

    const uint32 firstInstr = fte->startingInstruction + rewrites.jumpAddrOffset;
    const uint64 nameAddr = rewrites.stringRewrites.findReplacement(fte->nameOffset);

    FunctionSignature* sign = static_cast<FunctionSignature*>(types.lookupByIndex(rewrites.typeRewrites.findRewritten(fte->signatureIndex)));

    LocalScriptFunction sf = LocalScriptFunction();
    sf.firstInstrIndex = firstInstr;
    sf.nameOffset = nameAddr;
    sf.stackSize = fte->stackSize;
    sf.signature = sign;

    toStandardFilename(filename, sf.filename);

    functions.push_back(sf);
  }
}

uint32 VirtualMachine::addBytecodeFile(const BytecodeFile& file, const std::string& filename) {
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

  addFunctionEntries(m_types, file, instrRewrite, m_functions, filename);
  rewriteInstructions(m_instrBuf.getBuffer() + jumpAddrOff, file.instructionsSize, instrRewrite);

  return file.entryPointIndex + funcIdxOffset;
}

int32 VirtualMachine::beginExecution(const uint32 funcEntryIdx, const ProgramArgs& args) {
  const LocalScriptFunction& func = m_functions[funcEntryIdx];

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

      arr.setU64(i, str.address());
    }
  }

  Interpreter interp = Interpreter(*this);
  return interp.beginExecution(func, argsAddr);
}

void VirtualMachine::toString(std::string& out, typeindex type, uint64 value) {
  switch (type) {
    case TI_STRING: {
      QsArray rString = castToQsArray(TO_POINTER(value));
      out.append(reinterpret_cast<int8*>(rString.data), rString.length);
      return;
    }

    case TI_BOOL:
      out.append(value ? "true" : "false");
      return;

    case TI_UINT8:
    case TI_UINT16:
    case TI_UINT32:
    case TI_UINT64:
      out.append(std::to_string(value));
      return;

    case TI_INT8:
      out.append(std::to_string(*reinterpret_cast<int8*>(&value)));
      return;
    case TI_INT16:
      out.append(std::to_string(*reinterpret_cast<int16*>(&value)));
      return;
    case TI_INT32:
      out.append(std::to_string(*reinterpret_cast<int32*>(&value)));
      return;
    case TI_INT64:
      out.append(std::to_string(std::bit_cast<int64>(value)));
      return;

    case TI_FLOAT32:
      out.append(std::to_string(*reinterpret_cast<float32*>(&value)));
      return;
    case TI_FLOAT64:
      out.append(std::to_string(std::bit_cast<float64>(value)));
      return;

    case TI_VOID:
      // ???
      out.append("void");
      return;
    case TI_CLOSURE:
      out.append("#closure");
      return;

    default:
      break;
  }

  ScriptType* scriptType = m_types.lookupByIndex(type);
  if (!scriptType) {
    // Should be impossible
    out.append("NIL");
    return;
  }

  const typekind tk = scriptType->kind();
  switch (tk) {
    case TK_FUNC:
      out.append(scriptType->getTypeName());
      return;

    case TK_STRUCT: {
      ScriptStructType* structType = static_cast<ScriptStructType*>(scriptType);
      out.append(structType->getTypeName());
      out.append("{");

      QsObject obj = castToQsObject(TO_POINTER(value));

      uint64 off = 0;
      const uint32 propCount = structType->getPropertyCount();

      for (uint32 i = 0; i < propCount; i++) {
        if (i != 0) {
          out.append(", ");
        }

        StructProperty* prop = structType->getProperty(i);
        out.append(prop->name);
        out.append(": ");

        ScriptType* propType = prop->type;
        typeindex propTypeIndex = m_types.findIndex(propType);

        uint64 value;
        switch (propType->stackSizeBytes()) {
          case 1:
            value = obj.getU8Property(off);
            break;
          case 2:
            value = obj.getU16Property(off);
            break;
          case 4:
            value = obj.getU32Property(off);
            break;
          default:
            value = obj.getU64Property(off);
            break;
        }

        toString(out, propTypeIndex, value);
      }

      out.append("}");
    }

    case TK_ARRAY: {
      ScriptArrayType* arrType = static_cast<ScriptArrayType*>(scriptType);
      ScriptType* compType = arrType->getComponentType();

      typeindex compIndex = m_types.findIndex(compType);

      out.append("[");

      if (value) {
        QsArray arr = castToQsArray(TO_POINTER(value));
        const uint32 len = arr.length;

        for (uint32 i = 0; i < len; i++) {
          uint64 value;
          switch (compType->stackSizeBytes()) {
            case 1:
              value = arr.getU8(i);
              break;
            case 2:
              value = arr.getU16(i);
              break;
            case 4:
              value = arr.getU32(i);
              break;
            default:
              value = arr.getU64(i);
              break;
          }

          if (i != 0) {
            out.append(", ");
          }

          toString(out, compIndex, value);
        }
      }

      out.append("]");
      return;
    }

    default:
      return;
  }
}

bool VirtualMachine::objectEquals(const uint64 leftPtr, const uint64 rightPtr, const ScriptStructType* structType) {
  if (leftPtr == rightPtr) {
    return true;
  }

  const uint32 props = structType->getPropertyCount();
  uint64 off = 0;

  uint64 leftProp = 0;
  uint64 rightProp = 0;

  const uint8* left = reinterpret_cast<uint8*>(leftPtr + REFCOUNT_PREFIX_SIZE);
  const uint8* right = reinterpret_cast<uint8*>(rightPtr + REFCOUNT_PREFIX_SIZE);

  for (uint32 i = 0; i < props; i++) {
    const StructProperty* prop = structType->getProperty(i);
    const ScriptType* propType = prop->type;

    const typeindex propTypeIndex = m_types.findIndex(propType);
    const uint64 memSize = propType->stackSizeBytes();

    leftProp = 0;
    rightProp = 0;

    memcpy(&leftProp, left + off, memSize);
    memcpy(&rightProp, right + off, memSize);

    if (!equals(leftProp, rightProp, propTypeIndex)) {
      return false;
    }

    off += memSize;
  }

  return true;
}

#define ARRAY_EQUALITY_CHECKING_LOOP(size) \
  for (uint32 i = 0; i < length; i++) {\
    if (left.getU##size(i) == right.getU##size(i)) {\
      continue;\
    }\
    return false;\
  }

bool VirtualMachine::arrayEquals(const uint64 leftPtr, const uint64 rightPtr, const ScriptArrayType* arrayType) {
  if (leftPtr == rightPtr) {
    return true;
  }

  const QsArray left = qsArrayFromAddr(leftPtr);
  const QsArray right = qsArrayFromAddr(rightPtr);

  const uint32 length = left.length;

  if (length != right.length) {
    return false;
  }

  const ScriptType* componentType = arrayType->getComponentType();
  const typeindex componentIndex = m_types.findIndex(componentType);

  switch (componentIndex) {
    case TI_VOID:
      return false;
    case TI_BOOL:
    case TI_UINT8:
    case TI_INT8:
      ARRAY_EQUALITY_CHECKING_LOOP(8)
      break;
    case TI_UINT16:
    case TI_INT16:
      ARRAY_EQUALITY_CHECKING_LOOP(16)
      break;
    case TI_UINT32:
    case TI_INT32:
      ARRAY_EQUALITY_CHECKING_LOOP(32)
      break;
    case TI_UINT64:
    case TI_INT64:
    case TI_CLOSURE:
      ARRAY_EQUALITY_CHECKING_LOOP(64)
      break;

    case TI_STRING:
      for (uint32 i = 0; i < length; i++) {
        const uint64 l = left.getU64(i);
        const uint64 r = right.getU64(i);
        if (stringEquals(l, r)) {
          continue;
        }
        return false;
      }
      break;

    default: {
      const typekind kind = componentType->kind();

      if (kind == TK_ARRAY) {
        const ScriptArrayType* cArrayType = static_cast<const ScriptArrayType*>(componentType);

        for (uint32 i = 0; i < length; i++) {
          const uint64 l = left.getU64(i);
          const uint64 r = right.getU64(i);

          if (l == r || arrayEquals(l, r, cArrayType)) {
            continue;
          }

          return false;
        }
      }

      if (kind == TK_STRUCT) {
        const ScriptStructType* sType = static_cast<const ScriptStructType*>(componentType);

        for (uint32 i = 0; i < length; i++) {
          const uint64 l = left.getU64(i);
          const uint64 r = right.getU64(i);

          if (l == r || objectEquals(l, r, sType)) {
            continue;
          }

          return false;
        }
      }

      break;
    }
  }

  return true;
}

bool VirtualMachine::stringEquals(const uint64 leftAddr, const uint64 rightAddr) {
  if (leftAddr == rightAddr) {
    return true;
  }
  if (!leftAddr || !rightAddr) {
    return false;
  }

  const QsArray left = qsArrayFromAddr(leftAddr);
  const QsArray right = qsArrayFromAddr(rightAddr);

  const uint32 length = left.length;
  if (length != right.length) {
    return false;
  }

  ARRAY_EQUALITY_CHECKING_LOOP(8)

  return true;
}

bool VirtualMachine::equals(const uint64 left, const uint64 right, const typeindex idx) {
  switch (idx) {
    case TI_VOID:
      return false;
    case TI_BOOL:
    case TI_UINT8:
    case TI_INT8:
      return static_cast<uint8>(left) == static_cast<uint8>(right);
    case TI_UINT16:
    case TI_INT16:
      return static_cast<uint16>(left) == static_cast<uint16>(right);
    case TI_UINT32:
    case TI_INT32:
      return static_cast<uint32>(left) == static_cast<uint32>(right);
    case TI_UINT64:
    case TI_INT64:
    case TI_CLOSURE:
      return left == right;
    case TI_FLOAT32:
      return *reinterpret_cast<const float32*>(&left) == *reinterpret_cast<const float32*>(&right);
    case TI_FLOAT64:
      return std::bit_cast<float64>(left) == std::bit_cast<float64>(right);
    case TI_STRING:
      return stringEquals(left, right);

    default: {
      const ScriptType* scrType = m_types.lookupByIndex(idx);

      if (scrType->kind() == TK_ARRAY) {
        if (left == right) {
          return true;
        }
        return arrayEquals(left, right, static_cast<const ScriptArrayType*>(scrType));
      }

      if (scrType->kind() == TK_STRUCT) {
        if (left == right) {
          return true;
        }
        return objectEquals(left, right, static_cast<const ScriptStructType*>(scrType));
      }

      break;
    }
  }

  return false;
}

int8 VirtualMachine::compareString(const uint64 leftPtr, const uint64 rightPtr) {
  if (leftPtr == rightPtr) {
    return LEFT_EQ_RIGHT;
  }
  if (!leftPtr) {
    return LEFT_LT_RIGHT;
  }
  if (!rightPtr) {
    return LEFT_GT_RIGHT;
  }

  void* lPtr = reinterpret_cast<void*>(leftPtr);
  void* rPtr = reinterpret_cast<void*>(rightPtr);

  const QsArray lArr = castToQsArray(lPtr);
  const QsArray rArr = castToQsArray(rPtr);
  const uint32 len = lArr.length < rArr.length  ? lArr.length : rArr.length;

  return memcmp(lArr.data, rArr.data, len);
}

#define ARRAY_COMPARE_LOOP(type, func) \
  for (uint32 i = 0; i < len; i++) {\
    type l = lArr.func(i);\
    type r = rArr.func(i);\
    if (l == r) {\
      continue;\
    }\
    return l < r ? LEFT_LT_RIGHT : LEFT_GT_RIGHT;\
  }

int8 VirtualMachine::compareArray(const uint64 leftPtr, const uint64 rightPtr, const ScriptArrayType* arrType) {
  if (leftPtr == rightPtr) {
    return LEFT_EQ_RIGHT;
  }
  if (!leftPtr) {
    return LEFT_LT_RIGHT;
  }
  if (!rightPtr) {
    return LEFT_GT_RIGHT;
  }

  const QsArray lArr = qsArrayFromAddr(leftPtr);
  const QsArray rArr = qsArrayFromAddr(rightPtr);
  const uint32 len = lArr.length < rArr.length  ? lArr.length : rArr.length;

  const ScriptType* componentType = arrType->getComponentType();
  const typeindex cIdx = m_types.findIndex(componentType);

  switch (cIdx) {
    case TI_VOID:
      break;

    case TI_BOOL:
    case TI_UINT8:
      return memcmp(lArr.data, rArr.data, len);

    case TI_INT8:
      ARRAY_COMPARE_LOOP(int8, getI8)
      break;
    case TI_UINT16:
      ARRAY_COMPARE_LOOP(uint16, getU16)
      break;
    case TI_INT16:
      ARRAY_COMPARE_LOOP(int16, getI16)
      break;
    case TI_UINT32:
      ARRAY_COMPARE_LOOP(int32, getU32)
      break;
    case TI_INT32:
      ARRAY_COMPARE_LOOP(int32, getI32)
      break;
    case TI_CLOSURE:
    case TI_UINT64:
      ARRAY_COMPARE_LOOP(int64, getU64)
      break;
    case TI_INT64:
      ARRAY_COMPARE_LOOP(int64, getI64)
      break;
    case TI_FLOAT32:
      ARRAY_COMPARE_LOOP(float32, getF32)
      break;
    case TI_FLOAT64:
      ARRAY_COMPARE_LOOP(float64, getF64)
      break;

    case TI_STRING: {
      for (uint32 i = 0; i < len; i++) {
        const uint64 l = lArr.getU64(i);
        const uint64 r = rArr.getU64(i);

        const int8 cmp = compareString(l, r);

        if (cmp == LEFT_EQ_RIGHT) {
          continue;
        }

        return cmp;
      }
      break;
    }

    default:
      if (componentType->kind() != TK_ARRAY) {
        break;
      }

      const ScriptArrayType* cArrayType = static_cast<const ScriptArrayType*>(componentType);

      for (uint32 i = 0; i < len; i++) {
        const uint64 l = lArr.getU64(i);
        const uint64 r = rArr.getU64(i);

        const int8 cmp = compareArray(l, r, cArrayType);

        if (cmp == LEFT_EQ_RIGHT) {
          continue;
        }

        return cmp;
      }

      break;
  }

  return LEFT_EQ_RIGHT;
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

std::vector<LocalScriptFunction>& VirtualMachine::getFunctions() {
  return m_functions;
}

std::vector<NativeScriptFunction>& VirtualMachine::getNativeFunctions() {
  return m_nativeFunctions;
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
  if (off >= m_frameCount) {
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

static void appendCallStack(std::string& out, CallFrame frames[], const uint32 count) {
  for (uint32 i = count; i != 0; i--) {
    const CallFrame* frame = &frames[i - 1];

    if (i != count) {
      out.append("\n  ");
    }

    out.append("[");
    out.append(std::to_string(i));
    out.append("] ");

    out.append("at ");
    out.append(frame->name);

    out.append(" (");
    out.append(frame->filename);
    out.append(":");
    out.append(std::to_string(frame->line));
    out.append(")");
  }
}

void Interpreter::throwScriptError(const std::string& message) {
  std::string callStack;
  appendCallStack(callStack, m_callFrames, m_frameCount);

  throw ScriptError(message, callStack);
}

void Interpreter::moveExecutionTo(const LocalScriptFunction& func) {
  const CallFrame* oldFrame = getCallFrame();

  const StringPool& strPool = m_vm.getStringPool();
  const uint32 nameLen = strPool.getLength(func.nameOffset);
  const int8* nameContent = strPool.getCharacterData(func.nameOffset);

  CallFrame* frame = pushNewFrame();
  if (!frame) {
    throwScriptError("Maximum call depth reached");
  }

  frame->name = std::string(nameContent, nameLen);
  if (!func.filename.empty()) {
    frame->filename = func.filename;
  }

  uint64 stackSize = func.stackSize;
  uint64 stackOffset = 0;

  if (oldFrame) {
    uint64 paramsSize = 0;
    const FunctionSignature* sign = func.signature;

    for (uint32 i = 0; i < sign->getArgumentsLength(); i++) {
      paramsSize += sign->getArgumentType(i)->stackSizeBytes();
    }

    stackSize -= paramsSize;
    stackOffset = paramsSize;
  }

  uint8* stackFrame = m_stack.allocateFrame(stackSize);
  frame->stackBase = stackFrame - stackOffset;
  frame->allocatedSize = stackSize;

  if (!oldFrame) {
    frame->returnAddr = NO_RETURN_ADDR;
  } else {
    frame->returnAddr = m_registers[REGISTER_INSTR_COUNTER] + 1;
  }

  m_registers[REGISTER_INSTR_COUNTER] = func.firstInstrIndex;
}

int32 Interpreter::beginExecution(const LocalScriptFunction& func, const uint64 argsArrayAddr) {
  moveExecutionTo(func);

  const CallFrame* frame = getCallFrame();
  uint8* stackFrame = frame->stackBase;

  // Write the args array to the start of the stack frame
  *reinterpret_cast<uint64*>(stackFrame) = argsArrayAddr;

  run();

  return m_registers[REGISTER_RETURN_VALUE];
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

uint64 Interpreter::strConcat(QsArray& lString, const uint64 rightObj, typeindex rType) {
  if (rType == TI_STRING) {
    QsArray rString = castToQsArray(TO_POINTER(rightObj));
    uint32 newSize = lString.length + rString.length;

    QsArray result = m_vm.getHeap().allocString(newSize);
    memcpy(result.data, lString.data, lString.length);
    memcpy(result.data + lString.length, rString.data, rString.length);

    return result.address();
  }

  std::string str = "";
  str.append(reinterpret_cast<int8*>(lString.data), lString.length);
  m_vm.toString(str, rType, rightObj);

  const QsArray result = m_vm.getHeap().allocString(str.length());
  memcpy(result.data, str.data(), str.length());

  return result.address();
}

void Interpreter::run() {
  opcode code = OP_NOP;
  uint8 args[LENGTH_ARGS];

  uint8* stack = nullptr;
  uint8* global = m_vm.getGlobalMemory().getData();

  CallFrame* frame = nullptr;

  begin:
  m_vm.getInstructions().getInstruction(&code, args, m_registers[REGISTER_INSTR_COUNTER]);
  frame = getCallFrame();

  if (!frame) {
    return;
  }

  stack = frame->stackBase;

  switch (code) {
    case OP_RET:
      if (frame->returnAddr == NO_RETURN_ADDR) {
        popCallFrame();
        return;
      }
      m_registers[REGISTER_INSTR_COUNTER] = frame->returnAddr;
      popCallFrame();
      goto begin;

    case OP_PUSHLINE:
      frame->line = READ_U32ARG(0);
      break;
    case OP_JMP:
      m_registers[REGISTER_INSTR_COUNTER] = READ_U32ARG(0);
      goto begin;
    case OP_JMPI0:
      if (m_registers[args[4]]) {
        break;
      }
      m_registers[REGISTER_INSTR_COUNTER] = READ_U32ARG(0);
      goto begin;
    case OP_JMPN0:
      if (!m_registers[args[4]]) {
        break;
      }
      m_registers[REGISTER_INSTR_COUNTER] = READ_U32ARG(0);
      goto begin;

    case OP_LFUNCLOOKUP:
      m_registers[args[4]] = reinterpret_cast<uint64>(&m_vm.getFunctions().at(READ_U32ARG(0)));
      break;

    case OP_INVOKE: {
      ScriptFunction* sf = REG_AS(args[0], ScriptFunction*);

      if (sf->ftype() == FUNCTYPE_LOCAL) {
        moveExecutionTo(*REG_AS(args[0], LocalScriptFunction*));
        goto begin;
      }


    }

    case OP_LOADCONSTSTR: {
      const uint64 strOffset = READ_U64ARG(1);
      uint8* strPtr = m_vm.getStringPool().getPointer(strOffset);
      m_registers[args[0]] = reinterpret_cast<uint64>(strPtr);
      break;
    }

    case OP_ASSERT: {
      uint64 cond = m_registers[args[0]];
      void* strPtr = REG_AS(args[1], void*);

      if (cond) {
        break;
      }

      std::string errorMsg = "Assert ";

      if (frame->filename.empty()) {
        errorMsg.append("on line ");
      } else {
        errorMsg.append(frame->filename);
        errorMsg.append(":");
      }

      errorMsg.append(std::to_string(frame->line));
      errorMsg.append(" failed!");

      if (strPtr) {
        const QsArray arr = castToQsArray(strPtr);
        errorMsg.append("\n  ");
        errorMsg.append(reinterpret_cast<int8*>(arr.data), arr.length);
      }

      throwScriptError(errorMsg);
      break;
    }

    case OP_OBJALLOC: {
      const uint32 typeIdx = READ_U32ARG(1);
      const ScriptStructType* structType = static_cast<ScriptStructType*>(m_vm.getTypes().lookupByIndex(typeIdx));
      const uint64 memSize = structType->getHeapSize();

      QsObject obj = m_vm.getHeap().allocObject(memSize);

      m_registers[args[0]] = obj.address();
      break;
    }

    case OP_ARRAYALLOC: {
      const uint8 outReg = args[0];
      const uint32 elements = READ_U32ARG(1);
      const uint32 typeIndex = READ_U32ARG(5);

      ScriptArrayType* cType = static_cast<ScriptArrayType*>(m_vm.getTypes().lookupByIndex(typeIndex));
      const uint64 elemSize = cType->getComponentType()->stackSizeBytes();

      QsArray array = m_vm.getHeap().allocArray(elements, elemSize);
      m_registers[outReg] = array.address();

      array.setRefCount(1);

      break;
    }

    case OP_STRCONCAT: {
      QsArray lString = castToQsArray(REG_AS(args[0], void*));
      m_registers[args[6]] = strConcat(lString, m_registers[args[1]], READ_U32ARG(2));
      break;
    }

    case OP_SETARGTYPE: {
      m_argTypeIndexes[READ_U32ARG(0)] = READ_U32ARG(4);
      break;
    }

    // region Generated Instructions
    // 
    // This part was automatically generated by a deno script in
    // deno-scripts/evaluator-gen.ts
    //
    case OP_NOP:
      break;
    case OP_MOV:
      m_registers[args[1]] = m_registers[args[0]];
      break;
    case OP_LOADCONST8:
      m_registers[args[0]] = READ_U8ARG(1);
      break;
    case OP_LOADCONST16:
      m_registers[args[0]] = READ_U16ARG(1);
      break;
    case OP_LOADCONST32:
      m_registers[args[0]] = READ_U32ARG(1);
      break;
    case OP_LOADCONST64:
      m_registers[args[0]] = READ_U64ARG(1);
      break;
    case OP_SREAD8:
      m_registers[args[0]] = READ_U8(stack, READ_U8ARG(1));
      break;
    case OP_SREAD16:
      m_registers[args[0]] = READ_U16(stack, READ_U16ARG(1));
      break;
    case OP_SREAD32:
      m_registers[args[0]] = READ_U32(stack, READ_U32ARG(1));
      break;
    case OP_SREAD64:
      m_registers[args[0]] = READ_U64(stack, READ_U64ARG(1));
      break;
    case OP_SWRITE8:
      WRITE_U8(stack, READ_U64ARG(1), m_registers[args[0]]);
      break;
    case OP_SWRITE16:
      WRITE_U16(stack, READ_U64ARG(1), m_registers[args[0]]);
      break;
    case OP_SWRITE32:
      WRITE_U32(stack, READ_U64ARG(1), m_registers[args[0]]);
      break;
    case OP_SWRITE64:
      WRITE_U64(stack, READ_U64ARG(1), m_registers[args[0]]);
      break;
    case OP_STORECONST8:
      WRITE_U8(stack, READ_U32ARG(0), READ_U8ARG(4));
      break;
    case OP_STORECONST16:
      WRITE_U16(stack, READ_U32ARG(0), READ_U16ARG(4));
      break;
    case OP_STORECONST32:
      WRITE_U32(stack, READ_U32ARG(0), READ_U32ARG(4));
      break;
    case OP_STORECONST64:
      WRITE_U64(stack, READ_U32ARG(0), READ_U64ARG(4));
      break;
    case OP_GREAD8:
      m_registers[args[0]] = READ_U8(global, READ_U8ARG(1));
      break;
    case OP_GREAD16:
      m_registers[args[0]] = READ_U16(global, READ_U16ARG(1));
      break;
    case OP_GREAD32:
      m_registers[args[0]] = READ_U32(global, READ_U32ARG(1));
      break;
    case OP_GREAD64:
      m_registers[args[0]] = READ_U64(global, READ_U64ARG(1));
      break;
    case OP_GWRITE8:
      WRITE_U8(global, READ_U64ARG(1), m_registers[args[0]]);
      break;
    case OP_GWRITE16:
      WRITE_U16(global, READ_U64ARG(1), m_registers[args[0]]);
      break;
    case OP_GWRITE32:
      WRITE_U32(global, READ_U64ARG(1), m_registers[args[0]]);
      break;
    case OP_GWRITE64:
      WRITE_U64(global, READ_U64ARG(1), m_registers[args[0]]);
      break;
    case OP_GSTORECONST8:
      WRITE_U8(global, READ_U32ARG(0), READ_U8ARG(4));
      break;
    case OP_GSTORECONST16:
      WRITE_U16(global, READ_U32ARG(0), READ_U16ARG(4));
      break;
    case OP_GSTORECONST32:
      WRITE_U32(global, READ_U32ARG(0), READ_U32ARG(4));
      break;
    case OP_GSTORECONST64:
      WRITE_U64(global, READ_U32ARG(0), READ_U64ARG(4));
      break;
    case OP_GETSTACKPTR:
      m_registers[args[0]] = reinterpret_cast<uint64>(stack);
      break;
    case OP_CREAD8:
      m_registers[args[9]] = *reinterpret_cast<uint8*>(m_registers[args[0]] + READ_U32ARG(1));
      break;
    case OP_CREAD16:
      m_registers[args[9]] = *reinterpret_cast<uint16*>(m_registers[args[0]] + READ_U32ARG(1));
      break;
    case OP_CREAD32:
      m_registers[args[9]] = *reinterpret_cast<uint32*>(m_registers[args[0]] + READ_U32ARG(1));
      break;
    case OP_CREAD64:
      m_registers[args[9]] = *reinterpret_cast<uint64*>(m_registers[args[0]] + READ_U32ARG(1));
      break;
    case OP_CWRITE8:
      *reinterpret_cast<uint8*>(m_registers[args[0]] + READ_U32ARG(1)) = REG_AS(args[9], uint8);
      break;
    case OP_CWRITE16:
      *reinterpret_cast<uint16*>(m_registers[args[0]] + READ_U32ARG(1)) = REG_AS(args[9], uint16);
      break;
    case OP_CWRITE32:
      *reinterpret_cast<uint32*>(m_registers[args[0]] + READ_U32ARG(1)) = REG_AS(args[9], uint32);
      break;
    case OP_CWRITE64:
      *reinterpret_cast<uint64*>(m_registers[args[0]] + READ_U32ARG(1)) = REG_AS(args[9], uint64);
      break;
    case OP_READOBJ8:
      m_registers[args[1]] = *reinterpret_cast<uint8*>(m_registers[args[0]] + READ_U32ARG(2) + REFCOUNT_PREFIX_SIZE);
      break;
    case OP_READOBJ16:
      m_registers[args[1]] = *reinterpret_cast<uint16*>(m_registers[args[0]] + READ_U32ARG(2) + REFCOUNT_PREFIX_SIZE);
      break;
    case OP_READOBJ32:
      m_registers[args[1]] = *reinterpret_cast<uint32*>(m_registers[args[0]] + READ_U32ARG(2) + REFCOUNT_PREFIX_SIZE);
      break;
    case OP_READOBJ64:
      m_registers[args[1]] = *reinterpret_cast<uint64*>(m_registers[args[0]] + READ_U32ARG(2) + REFCOUNT_PREFIX_SIZE);
      break;
    case OP_WRITEOBJ8:
      *reinterpret_cast<uint8*>(m_registers[args[0]] + READ_U32ARG(2) + REFCOUNT_PREFIX_SIZE) = REG_AS(args[1], uint8);
      break;
    case OP_WRITEOBJ16:
      *reinterpret_cast<uint16*>(m_registers[args[0]] + READ_U32ARG(2) + REFCOUNT_PREFIX_SIZE) = REG_AS(args[1], uint16);
      break;
    case OP_WRITEOBJ32:
      *reinterpret_cast<uint32*>(m_registers[args[0]] + READ_U32ARG(2) + REFCOUNT_PREFIX_SIZE) = REG_AS(args[1], uint32);
      break;
    case OP_WRITEOBJ64:
      *reinterpret_cast<uint64*>(m_registers[args[0]] + READ_U32ARG(2) + REFCOUNT_PREFIX_SIZE) = REG_AS(args[1], uint64);
      break;
    case OP_READIDX8: {
      QsArray arr = castToQsArray(REG_AS(args[0], void*));
      const uint32 idx = REG_AS(args[2], uint32);
      
      if (idx >= arr.length) {;
        throwScriptError("Index out of bounds");
      };
      
      m_registers[args[1]] = arr.getU8(idx);
      break;
    }
    case OP_READIDX16: {
      QsArray arr = castToQsArray(REG_AS(args[0], void*));
      const uint32 idx = REG_AS(args[2], uint32);
      
      if (idx >= arr.length) {;
        throwScriptError("Index out of bounds");
      };
      
      m_registers[args[1]] = arr.getU16(idx);
      break;
    }
    case OP_READIDX32: {
      QsArray arr = castToQsArray(REG_AS(args[0], void*));
      const uint32 idx = REG_AS(args[2], uint32);
      
      if (idx >= arr.length) {;
        throwScriptError("Index out of bounds");
      };
      
      m_registers[args[1]] = arr.getU32(idx);
      break;
    }
    case OP_READIDX64: {
      QsArray arr = castToQsArray(REG_AS(args[0], void*));
      const uint32 idx = REG_AS(args[2], uint32);
      
      if (idx >= arr.length) {;
        throwScriptError("Index out of bounds");
      };
      
      m_registers[args[1]] = arr.getU64(idx);
      break;
    }
    case OP_WRITEIDX8: {
      QsArray arr = castToQsArray(REG_AS(args[0], void*));
      const uint32 idx = REG_AS(args[2], uint32);
      
      if (idx >= arr.length) {;
        throwScriptError("Index out of bounds");
      };
      
      const uint8 value = REG_AS(args[1], uint8);
      arr.setU8(idx, value);
      break;
    }
    case OP_WRITEIDX16: {
      QsArray arr = castToQsArray(REG_AS(args[0], void*));
      const uint32 idx = REG_AS(args[2], uint32);
      
      if (idx >= arr.length) {;
        throwScriptError("Index out of bounds");
      };
      
      const uint16 value = REG_AS(args[1], uint16);
      arr.setU16(idx, value);
      break;
    }
    case OP_WRITEIDX32: {
      QsArray arr = castToQsArray(REG_AS(args[0], void*));
      const uint32 idx = REG_AS(args[2], uint32);
      
      if (idx >= arr.length) {;
        throwScriptError("Index out of bounds");
      };
      
      const uint32 value = REG_AS(args[1], uint32);
      arr.setU32(idx, value);
      break;
    }
    case OP_WRITEIDX64: {
      QsArray arr = castToQsArray(REG_AS(args[0], void*));
      const uint32 idx = REG_AS(args[2], uint32);
      
      if (idx >= arr.length) {;
        throwScriptError("Index out of bounds");
      };
      
      const uint64 value = REG_AS(args[1], uint64);
      arr.setU64(idx, value);
      break;
    }
    case OP_ARRLEN:
      m_registers[args[1]] = readQsArrayLength(REG_AS(args[0], void*));
      break;
    case OP_I8TU8:
      REG_AS(args[1], uint8) = static_cast<uint8>(REG_AS(args[0], int8));
      break;
    case OP_I8TI16:
      REG_AS(args[1], int16) = static_cast<int16>(REG_AS(args[0], int8));
      break;
    case OP_I8TU16:
      REG_AS(args[1], uint16) = static_cast<uint16>(REG_AS(args[0], int8));
      break;
    case OP_I8TI32:
      REG_AS(args[1], int32) = static_cast<int32>(REG_AS(args[0], int8));
      break;
    case OP_I8TU32:
      REG_AS(args[1], uint32) = static_cast<uint32>(REG_AS(args[0], int8));
      break;
    case OP_I8TI64:
      REG_AS(args[1], int64) = static_cast<int64>(REG_AS(args[0], int8));
      break;
    case OP_I8TU64:
      REG_AS(args[1], uint64) = static_cast<uint64>(REG_AS(args[0], int8));
      break;
    case OP_I8TF32:
      REG_AS(args[1], float32) = static_cast<float32>(REG_AS(args[0], int8));
      break;
    case OP_I8TF64:
      REG_AS(args[1], float64) = static_cast<float64>(REG_AS(args[0], int8));
      break;
    case OP_U8TI8:
      REG_AS(args[1], int8) = static_cast<int8>(REG_AS(args[0], uint8));
      break;
    case OP_U8TI16:
      REG_AS(args[1], int16) = static_cast<int16>(REG_AS(args[0], uint8));
      break;
    case OP_U8TI32:
      REG_AS(args[1], int32) = static_cast<int32>(REG_AS(args[0], uint8));
      break;
    case OP_U8TI64:
      REG_AS(args[1], int64) = static_cast<int64>(REG_AS(args[0], uint8));
      break;
    case OP_U8TF32:
      REG_AS(args[1], float32) = static_cast<float32>(REG_AS(args[0], uint8));
      break;
    case OP_U8TF64:
      REG_AS(args[1], float64) = static_cast<float64>(REG_AS(args[0], uint8));
      break;
    case OP_I16TI8:
      REG_AS(args[1], int8) = static_cast<int8>(REG_AS(args[0], int16));
      break;
    case OP_I16TU8:
      REG_AS(args[1], uint8) = static_cast<uint8>(REG_AS(args[0], int16));
      break;
    case OP_I16TU16:
      REG_AS(args[1], uint16) = static_cast<uint16>(REG_AS(args[0], int16));
      break;
    case OP_I16TI32:
      REG_AS(args[1], int32) = static_cast<int32>(REG_AS(args[0], int16));
      break;
    case OP_I16TU32:
      REG_AS(args[1], uint32) = static_cast<uint32>(REG_AS(args[0], int16));
      break;
    case OP_I16TI64:
      REG_AS(args[1], int64) = static_cast<int64>(REG_AS(args[0], int16));
      break;
    case OP_I16TU64:
      REG_AS(args[1], uint64) = static_cast<uint64>(REG_AS(args[0], int16));
      break;
    case OP_I16TF32:
      REG_AS(args[1], float32) = static_cast<float32>(REG_AS(args[0], int16));
      break;
    case OP_I16TF64:
      REG_AS(args[1], float64) = static_cast<float64>(REG_AS(args[0], int16));
      break;
    case OP_U16TI8:
      REG_AS(args[1], int8) = static_cast<int8>(REG_AS(args[0], uint16));
      break;
    case OP_U16TI16:
      REG_AS(args[1], int16) = static_cast<int16>(REG_AS(args[0], uint16));
      break;
    case OP_U16TI32:
      REG_AS(args[1], int32) = static_cast<int32>(REG_AS(args[0], uint16));
      break;
    case OP_U16TI64:
      REG_AS(args[1], int64) = static_cast<int64>(REG_AS(args[0], uint16));
      break;
    case OP_U16TF32:
      REG_AS(args[1], float32) = static_cast<float32>(REG_AS(args[0], uint16));
      break;
    case OP_U16TF64:
      REG_AS(args[1], float64) = static_cast<float64>(REG_AS(args[0], uint16));
      break;
    case OP_I32TI8:
      REG_AS(args[1], int8) = static_cast<int8>(REG_AS(args[0], int32));
      break;
    case OP_I32TU8:
      REG_AS(args[1], uint8) = static_cast<uint8>(REG_AS(args[0], int32));
      break;
    case OP_I32TI16:
      REG_AS(args[1], int16) = static_cast<int16>(REG_AS(args[0], int32));
      break;
    case OP_I32TU16:
      REG_AS(args[1], uint16) = static_cast<uint16>(REG_AS(args[0], int32));
      break;
    case OP_I32TU32:
      REG_AS(args[1], uint32) = static_cast<uint32>(REG_AS(args[0], int32));
      break;
    case OP_I32TI64:
      REG_AS(args[1], int64) = static_cast<int64>(REG_AS(args[0], int32));
      break;
    case OP_I32TU64:
      REG_AS(args[1], uint64) = static_cast<uint64>(REG_AS(args[0], int32));
      break;
    case OP_I32TF32:
      REG_AS(args[1], float32) = static_cast<float32>(REG_AS(args[0], int32));
      break;
    case OP_I32TF64:
      REG_AS(args[1], float64) = static_cast<float64>(REG_AS(args[0], int32));
      break;
    case OP_U32TI8:
      REG_AS(args[1], int8) = static_cast<int8>(REG_AS(args[0], uint32));
      break;
    case OP_U32TI16:
      REG_AS(args[1], int16) = static_cast<int16>(REG_AS(args[0], uint32));
      break;
    case OP_U32TI32:
      REG_AS(args[1], int32) = static_cast<int32>(REG_AS(args[0], uint32));
      break;
    case OP_U32TI64:
      REG_AS(args[1], int64) = static_cast<int64>(REG_AS(args[0], uint32));
      break;
    case OP_U32TF32:
      REG_AS(args[1], float32) = static_cast<float32>(REG_AS(args[0], uint32));
      break;
    case OP_U32TF64:
      REG_AS(args[1], float64) = static_cast<float64>(REG_AS(args[0], uint32));
      break;
    case OP_I64TI8:
      REG_AS(args[1], int8) = static_cast<int8>(REG_AS(args[0], int64));
      break;
    case OP_I64TU8:
      REG_AS(args[1], uint8) = static_cast<uint8>(REG_AS(args[0], int64));
      break;
    case OP_I64TI16:
      REG_AS(args[1], int16) = static_cast<int16>(REG_AS(args[0], int64));
      break;
    case OP_I64TU16:
      REG_AS(args[1], uint16) = static_cast<uint16>(REG_AS(args[0], int64));
      break;
    case OP_I64TI32:
      REG_AS(args[1], int32) = static_cast<int32>(REG_AS(args[0], int64));
      break;
    case OP_I64TU32:
      REG_AS(args[1], uint32) = static_cast<uint32>(REG_AS(args[0], int64));
      break;
    case OP_I64TU64:
      REG_AS(args[1], uint64) = static_cast<uint64>(REG_AS(args[0], int64));
      break;
    case OP_I64TF32:
      REG_AS(args[1], float32) = static_cast<float32>(REG_AS(args[0], int64));
      break;
    case OP_I64TF64:
      REG_AS(args[1], float64) = static_cast<float64>(REG_AS(args[0], int64));
      break;
    case OP_U64TI8:
      REG_AS(args[1], int8) = static_cast<int8>(REG_AS(args[0], uint64));
      break;
    case OP_U64TI16:
      REG_AS(args[1], int16) = static_cast<int16>(REG_AS(args[0], uint64));
      break;
    case OP_U64TI32:
      REG_AS(args[1], int32) = static_cast<int32>(REG_AS(args[0], uint64));
      break;
    case OP_U64TI64:
      REG_AS(args[1], int64) = static_cast<int64>(REG_AS(args[0], uint64));
      break;
    case OP_U64TF32:
      REG_AS(args[1], float32) = static_cast<float32>(REG_AS(args[0], uint64));
      break;
    case OP_U64TF64:
      REG_AS(args[1], float64) = static_cast<float64>(REG_AS(args[0], uint64));
      break;
    case OP_F32TI8:
      REG_AS(args[1], int8) = static_cast<int8>(REG_AS(args[0], float32));
      break;
    case OP_F32TU8:
      REG_AS(args[1], uint8) = static_cast<uint8>(REG_AS(args[0], float32));
      break;
    case OP_F32TI16:
      REG_AS(args[1], int16) = static_cast<int16>(REG_AS(args[0], float32));
      break;
    case OP_F32TU16:
      REG_AS(args[1], uint16) = static_cast<uint16>(REG_AS(args[0], float32));
      break;
    case OP_F32TI32:
      REG_AS(args[1], int32) = static_cast<int32>(REG_AS(args[0], float32));
      break;
    case OP_F32TU32:
      REG_AS(args[1], uint32) = static_cast<uint32>(REG_AS(args[0], float32));
      break;
    case OP_F32TI64:
      REG_AS(args[1], int64) = static_cast<int64>(REG_AS(args[0], float32));
      break;
    case OP_F32TU64:
      REG_AS(args[1], uint64) = static_cast<uint64>(REG_AS(args[0], float32));
      break;
    case OP_F32TF64:
      REG_AS(args[1], float64) = static_cast<float64>(REG_AS(args[0], float32));
      break;
    case OP_F64TI8:
      REG_AS(args[1], int8) = static_cast<int8>(REG_AS(args[0], float64));
      break;
    case OP_F64TU8:
      REG_AS(args[1], uint8) = static_cast<uint8>(REG_AS(args[0], float64));
      break;
    case OP_F64TI16:
      REG_AS(args[1], int16) = static_cast<int16>(REG_AS(args[0], float64));
      break;
    case OP_F64TU16:
      REG_AS(args[1], uint16) = static_cast<uint16>(REG_AS(args[0], float64));
      break;
    case OP_F64TI32:
      REG_AS(args[1], int32) = static_cast<int32>(REG_AS(args[0], float64));
      break;
    case OP_F64TU32:
      REG_AS(args[1], uint32) = static_cast<uint32>(REG_AS(args[0], float64));
      break;
    case OP_F64TI64:
      REG_AS(args[1], int64) = static_cast<int64>(REG_AS(args[0], float64));
      break;
    case OP_F64TU64:
      REG_AS(args[1], uint64) = static_cast<uint64>(REG_AS(args[0], float64));
      break;
    case OP_F64TF32:
      REG_AS(args[1], float32) = static_cast<float32>(REG_AS(args[0], float64));
      break;
    case OP_BNEGATE:
      m_registers[args[1]] = ~m_registers[args[0]];
      break;
    case OP_LNEGATE:
      m_registers[args[1]] = m_registers[args[0]] ? 0 : 1;
      break;
    case OP_NEGI8:
      REG_AS(args[1], int8) = -REG_AS(args[0], int8);
      break;
    case OP_NEGU8:
      REG_AS(args[1], int8) = -REG_AS(args[0], int8);
      break;
    case OP_NEGI16:
      REG_AS(args[1], int16) = -REG_AS(args[0], int16);
      break;
    case OP_NEGU16:
      REG_AS(args[1], int16) = -REG_AS(args[0], int16);
      break;
    case OP_NEGI32:
      REG_AS(args[1], int32) = -REG_AS(args[0], int32);
      break;
    case OP_NEGU32:
      REG_AS(args[1], int32) = -REG_AS(args[0], int32);
      break;
    case OP_NEGI64:
      REG_AS(args[1], int64) = -REG_AS(args[0], int64);
      break;
    case OP_NEGU64:
      REG_AS(args[1], int64) = -REG_AS(args[0], int64);
      break;
    case OP_NEGF32:
      REG_AS(args[1], float32) = -REG_AS(args[0], float32);
      break;
    case OP_NEGF64:
      REG_AS(args[1], float64) = -REG_AS(args[0], float64);
      break;
    case OP_INCI8:
      REG_AS(args[1], int8) = REG_AS(args[0], int8) + 1;
      break;
    case OP_INCU8:
      REG_AS(args[1], uint8) = REG_AS(args[0], uint8) + 1;
      break;
    case OP_INCI16:
      REG_AS(args[1], int16) = REG_AS(args[0], int16) + 1;
      break;
    case OP_INCU16:
      REG_AS(args[1], uint16) = REG_AS(args[0], uint16) + 1;
      break;
    case OP_INCI32:
      REG_AS(args[1], int32) = REG_AS(args[0], int32) + 1;
      break;
    case OP_INCU32:
      REG_AS(args[1], uint32) = REG_AS(args[0], uint32) + 1;
      break;
    case OP_INCI64:
      REG_AS(args[1], int64) = REG_AS(args[0], int64) + 1;
      break;
    case OP_INCU64:
      REG_AS(args[1], uint64) = REG_AS(args[0], uint64) + 1;
      break;
    case OP_INCF32:
      REG_AS(args[1], float32) = REG_AS(args[0], float32) + 1;
      break;
    case OP_INCF64:
      REG_AS(args[1], float64) = REG_AS(args[0], float64) + 1;
      break;
    case OP_DECI8:
      REG_AS(args[1], int8) = REG_AS(args[0], int8) - 1;
      break;
    case OP_DECU8:
      REG_AS(args[1], uint8) = REG_AS(args[0], uint8) - 1;
      break;
    case OP_DECI16:
      REG_AS(args[1], int16) = REG_AS(args[0], int16) - 1;
      break;
    case OP_DECU16:
      REG_AS(args[1], uint16) = REG_AS(args[0], uint16) - 1;
      break;
    case OP_DECI32:
      REG_AS(args[1], int32) = REG_AS(args[0], int32) - 1;
      break;
    case OP_DECU32:
      REG_AS(args[1], uint32) = REG_AS(args[0], uint32) - 1;
      break;
    case OP_DECI64:
      REG_AS(args[1], int64) = REG_AS(args[0], int64) - 1;
      break;
    case OP_DECU64:
      REG_AS(args[1], uint64) = REG_AS(args[0], uint64) - 1;
      break;
    case OP_DECF32:
      REG_AS(args[1], float32) = REG_AS(args[0], float32) - 1;
      break;
    case OP_DECF64:
      REG_AS(args[1], float64) = REG_AS(args[0], float64) - 1;
      break;
    case OP_LSHIFT:
      m_registers[args[2]] = m_registers[args[0]] << m_registers[args[1]];
      break;
    case OP_RSHIFT:
      m_registers[args[2]] = m_registers[args[0]] >> m_registers[args[1]];
      break;
    case OP_BAND:
      m_registers[args[2]] = m_registers[args[0]] & m_registers[args[1]];
      break;
    case OP_LAND:
      m_registers[args[2]] = m_registers[args[0]] && m_registers[args[1]];
      break;
    case OP_BOR:
      m_registers[args[2]] = m_registers[args[0]] | m_registers[args[1]];
      break;
    case OP_LOR:
      m_registers[args[2]] = m_registers[args[0]] || m_registers[args[1]];
      break;
    case OP_BXOR:
      m_registers[args[2]] = m_registers[args[0]] ^ m_registers[args[1]];
      break;
    case OP_LXOR:
      m_registers[args[2]] = !m_registers[args[0]] != !m_registers[args[1]];
      break;
    case OP_ADDI8:
      REG_AS(args[2], int8) = REG_AS(args[0], int8) + REG_AS(args[1], int8);
      break;
    case OP_ADDU8:
      REG_AS(args[2], uint8) = REG_AS(args[0], uint8) + REG_AS(args[1], uint8);
      break;
    case OP_ADDI16:
      REG_AS(args[2], int16) = REG_AS(args[0], int16) + REG_AS(args[1], int16);
      break;
    case OP_ADDU16:
      REG_AS(args[2], uint16) = REG_AS(args[0], uint16) + REG_AS(args[1], uint16);
      break;
    case OP_ADDI32:
      REG_AS(args[2], int32) = REG_AS(args[0], int32) + REG_AS(args[1], int32);
      break;
    case OP_ADDU32:
      REG_AS(args[2], uint32) = REG_AS(args[0], uint32) + REG_AS(args[1], uint32);
      break;
    case OP_ADDI64:
      REG_AS(args[2], int64) = REG_AS(args[0], int64) + REG_AS(args[1], int64);
      break;
    case OP_ADDU64:
      REG_AS(args[2], uint64) = REG_AS(args[0], uint64) + REG_AS(args[1], uint64);
      break;
    case OP_ADDF32:
      REG_AS(args[2], float32) = REG_AS(args[0], float32) + REG_AS(args[1], float32);
      break;
    case OP_ADDF64:
      REG_AS(args[2], float64) = REG_AS(args[0], float64) + REG_AS(args[1], float64);
      break;
    case OP_SUBI8:
      REG_AS(args[2], int8) = REG_AS(args[0], int8) - REG_AS(args[1], int8);
      break;
    case OP_SUBU8:
      REG_AS(args[2], uint8) = REG_AS(args[0], uint8) - REG_AS(args[1], uint8);
      break;
    case OP_SUBI16:
      REG_AS(args[2], int16) = REG_AS(args[0], int16) - REG_AS(args[1], int16);
      break;
    case OP_SUBU16:
      REG_AS(args[2], uint16) = REG_AS(args[0], uint16) - REG_AS(args[1], uint16);
      break;
    case OP_SUBI32:
      REG_AS(args[2], int32) = REG_AS(args[0], int32) - REG_AS(args[1], int32);
      break;
    case OP_SUBU32:
      REG_AS(args[2], uint32) = REG_AS(args[0], uint32) - REG_AS(args[1], uint32);
      break;
    case OP_SUBI64:
      REG_AS(args[2], int64) = REG_AS(args[0], int64) - REG_AS(args[1], int64);
      break;
    case OP_SUBU64:
      REG_AS(args[2], uint64) = REG_AS(args[0], uint64) - REG_AS(args[1], uint64);
      break;
    case OP_SUBF32:
      REG_AS(args[2], float32) = REG_AS(args[0], float32) - REG_AS(args[1], float32);
      break;
    case OP_SUBF64:
      REG_AS(args[2], float64) = REG_AS(args[0], float64) - REG_AS(args[1], float64);
      break;
    case OP_DIVI8:
      REG_AS(args[2], int8) = REG_AS(args[0], int8) / REG_AS(args[1], int8);
      break;
    case OP_DIVU8:
      REG_AS(args[2], uint8) = REG_AS(args[0], uint8) / REG_AS(args[1], uint8);
      break;
    case OP_DIVI16:
      REG_AS(args[2], int16) = REG_AS(args[0], int16) / REG_AS(args[1], int16);
      break;
    case OP_DIVU16:
      REG_AS(args[2], uint16) = REG_AS(args[0], uint16) / REG_AS(args[1], uint16);
      break;
    case OP_DIVI32:
      REG_AS(args[2], int32) = REG_AS(args[0], int32) / REG_AS(args[1], int32);
      break;
    case OP_DIVU32:
      REG_AS(args[2], uint32) = REG_AS(args[0], uint32) / REG_AS(args[1], uint32);
      break;
    case OP_DIVI64:
      REG_AS(args[2], int64) = REG_AS(args[0], int64) / REG_AS(args[1], int64);
      break;
    case OP_DIVU64:
      REG_AS(args[2], uint64) = REG_AS(args[0], uint64) / REG_AS(args[1], uint64);
      break;
    case OP_DIVF32:
      REG_AS(args[2], float32) = REG_AS(args[0], float32) / REG_AS(args[1], float32);
      break;
    case OP_DIVF64:
      REG_AS(args[2], float64) = REG_AS(args[0], float64) / REG_AS(args[1], float64);
      break;
    case OP_MULI8:
      REG_AS(args[2], int8) = REG_AS(args[0], int8) * REG_AS(args[1], int8);
      break;
    case OP_MULU8:
      REG_AS(args[2], uint8) = REG_AS(args[0], uint8) * REG_AS(args[1], uint8);
      break;
    case OP_MULI16:
      REG_AS(args[2], int16) = REG_AS(args[0], int16) * REG_AS(args[1], int16);
      break;
    case OP_MULU16:
      REG_AS(args[2], uint16) = REG_AS(args[0], uint16) * REG_AS(args[1], uint16);
      break;
    case OP_MULI32:
      REG_AS(args[2], int32) = REG_AS(args[0], int32) * REG_AS(args[1], int32);
      break;
    case OP_MULU32:
      REG_AS(args[2], uint32) = REG_AS(args[0], uint32) * REG_AS(args[1], uint32);
      break;
    case OP_MULI64:
      REG_AS(args[2], int64) = REG_AS(args[0], int64) * REG_AS(args[1], int64);
      break;
    case OP_MULU64:
      REG_AS(args[2], uint64) = REG_AS(args[0], uint64) * REG_AS(args[1], uint64);
      break;
    case OP_MULF32:
      REG_AS(args[2], float32) = REG_AS(args[0], float32) * REG_AS(args[1], float32);
      break;
    case OP_MULF64:
      REG_AS(args[2], float64) = REG_AS(args[0], float64) * REG_AS(args[1], float64);
      break;
    case OP_MODI8:
      REG_AS(args[2], int8) = REG_AS(args[0], int8) % REG_AS(args[1], int8);
      break;
    case OP_MODU8:
      REG_AS(args[2], uint8) = REG_AS(args[0], uint8) % REG_AS(args[1], uint8);
      break;
    case OP_MODI16:
      REG_AS(args[2], int16) = REG_AS(args[0], int16) % REG_AS(args[1], int16);
      break;
    case OP_MODU16:
      REG_AS(args[2], uint16) = REG_AS(args[0], uint16) % REG_AS(args[1], uint16);
      break;
    case OP_MODI32:
      REG_AS(args[2], int32) = REG_AS(args[0], int32) % REG_AS(args[1], int32);
      break;
    case OP_MODU32:
      REG_AS(args[2], uint32) = REG_AS(args[0], uint32) % REG_AS(args[1], uint32);
      break;
    case OP_MODI64:
      REG_AS(args[2], int64) = REG_AS(args[0], int64) % REG_AS(args[1], int64);
      break;
    case OP_MODU64:
      REG_AS(args[2], uint64) = REG_AS(args[0], uint64) % REG_AS(args[1], uint64);
      break;
    case OP_MODF32:
      REG_AS(args[2], float32) = fmod(REG_AS(args[0], float32), REG_AS(args[1], float32));
      break;
    case OP_MODF64:
      REG_AS(args[2], float64) = fmod(REG_AS(args[0], float64), REG_AS(args[1], float64));
      break;
    case OP_POWI8:
      REG_AS(args[2], int8) = std::pow(REG_AS(args[0], int8), REG_AS(args[1], int8));
      break;
    case OP_POWU8:
      REG_AS(args[2], uint8) = std::pow(REG_AS(args[0], uint8), REG_AS(args[1], uint8));
      break;
    case OP_POWI16:
      REG_AS(args[2], int16) = std::pow(REG_AS(args[0], int16), REG_AS(args[1], int16));
      break;
    case OP_POWU16:
      REG_AS(args[2], uint16) = std::pow(REG_AS(args[0], uint16), REG_AS(args[1], uint16));
      break;
    case OP_POWI32:
      REG_AS(args[2], int32) = std::pow(REG_AS(args[0], int32), REG_AS(args[1], int32));
      break;
    case OP_POWU32:
      REG_AS(args[2], uint32) = std::pow(REG_AS(args[0], uint32), REG_AS(args[1], uint32));
      break;
    case OP_POWI64:
      REG_AS(args[2], int64) = std::pow(REG_AS(args[0], int64), REG_AS(args[1], int64));
      break;
    case OP_POWU64:
      REG_AS(args[2], uint64) = std::pow(REG_AS(args[0], uint64), REG_AS(args[1], uint64));
      break;
    case OP_POWF32:
      REG_AS(args[2], float32) = std::pow(REG_AS(args[0], float32), REG_AS(args[1], float32));
      break;
    case OP_POWF64:
      REG_AS(args[2], float64) = std::pow(REG_AS(args[0], float64), REG_AS(args[1], float64));
      break;
    case OP_EQ8:
      m_registers[args[2]] = REG_AS(args[0], uint8) == REG_AS(args[1], uint8);
      break;
    case OP_EQ16:
      m_registers[args[2]] = REG_AS(args[0], uint16) == REG_AS(args[1], uint16);
      break;
    case OP_EQ32:
      m_registers[args[2]] = REG_AS(args[0], uint32) == REG_AS(args[1], uint32);
      break;
    case OP_EQ64:
      m_registers[args[2]] = REG_AS(args[0], uint64) == REG_AS(args[1], uint64);
      break;
    case OP_EQARR:
      m_registers[args[2]] = doArrayEqualityCheck(args[0], args[1], READ_U32ARG(3));
      break;
    case OP_EQSTRUCT:
      m_registers[args[2]] = doStructEqualityCheck(args[0], args[1], READ_U32ARG(3));
      break;
    case OP_NEQ8:
      m_registers[args[2]] = REG_AS(args[0], uint8) != REG_AS(args[1], uint8);
      break;
    case OP_NEQ16:
      m_registers[args[2]] = REG_AS(args[0], uint16) != REG_AS(args[1], uint16);
      break;
    case OP_NEQ32:
      m_registers[args[2]] = REG_AS(args[0], uint32) != REG_AS(args[1], uint32);
      break;
    case OP_NEQ64:
      m_registers[args[2]] = REG_AS(args[0], uint64) != REG_AS(args[1], uint64);
      break;
    case OP_NEQARR:
      m_registers[args[2]] = !doArrayEqualityCheck(args[0], args[1], READ_U32ARG(3));
      break;
    case OP_NEQSTRUCT:
      m_registers[args[2]] = !doStructEqualityCheck(args[0], args[1], READ_U32ARG(3));
      break;
    case OP_GTI8:
      m_registers[args[2]] = REG_AS(args[0], int8) > REG_AS(args[1], int8);
      break;
    case OP_GTU8:
      m_registers[args[2]] = REG_AS(args[0], uint8) > REG_AS(args[1], uint8);
      break;
    case OP_GTI16:
      m_registers[args[2]] = REG_AS(args[0], int16) > REG_AS(args[1], int16);
      break;
    case OP_GTU16:
      m_registers[args[2]] = REG_AS(args[0], uint16) > REG_AS(args[1], uint16);
      break;
    case OP_GTI32:
      m_registers[args[2]] = REG_AS(args[0], int32) > REG_AS(args[1], int32);
      break;
    case OP_GTU32:
      m_registers[args[2]] = REG_AS(args[0], uint32) > REG_AS(args[1], uint32);
      break;
    case OP_GTI64:
      m_registers[args[2]] = REG_AS(args[0], int64) > REG_AS(args[1], int64);
      break;
    case OP_GTU64:
      m_registers[args[2]] = REG_AS(args[0], uint64) > REG_AS(args[1], uint64);
      break;
    case OP_GTF32:
      m_registers[args[2]] = REG_AS(args[0], float32) > REG_AS(args[1], float32);
      break;
    case OP_GTF64:
      m_registers[args[2]] = REG_AS(args[0], float64) > REG_AS(args[1], float64);
      break;
    case OP_GTARR:
      m_registers[args[2]] = doArrayComparison(args[0], args[1], READ_U32ARG(3)) == LEFT_GT_RIGHT;
      break;
    case OP_GTEI8:
      m_registers[args[2]] = REG_AS(args[0], int8) >= REG_AS(args[1], int8);
      break;
    case OP_GTEU8:
      m_registers[args[2]] = REG_AS(args[0], uint8) >= REG_AS(args[1], uint8);
      break;
    case OP_GTEI16:
      m_registers[args[2]] = REG_AS(args[0], int16) >= REG_AS(args[1], int16);
      break;
    case OP_GTEU16:
      m_registers[args[2]] = REG_AS(args[0], uint16) >= REG_AS(args[1], uint16);
      break;
    case OP_GTEI32:
      m_registers[args[2]] = REG_AS(args[0], int32) >= REG_AS(args[1], int32);
      break;
    case OP_GTEU32:
      m_registers[args[2]] = REG_AS(args[0], uint32) >= REG_AS(args[1], uint32);
      break;
    case OP_GTEI64:
      m_registers[args[2]] = REG_AS(args[0], int64) >= REG_AS(args[1], int64);
      break;
    case OP_GTEU64:
      m_registers[args[2]] = REG_AS(args[0], uint64) >= REG_AS(args[1], uint64);
      break;
    case OP_GTEF32:
      m_registers[args[2]] = REG_AS(args[0], float32) >= REG_AS(args[1], float32);
      break;
    case OP_GTEF64:
      m_registers[args[2]] = REG_AS(args[0], float64) >= REG_AS(args[1], float64);
      break;
    case OP_GTEARR:
      m_registers[args[2]] = doArrayComparison(args[0], args[1], READ_U32ARG(3)) != LEFT_LT_RIGHT;
      break;
    case OP_LTI8:
      m_registers[args[2]] = REG_AS(args[0], int8) < REG_AS(args[1], int8);
      break;
    case OP_LTU8:
      m_registers[args[2]] = REG_AS(args[0], uint8) < REG_AS(args[1], uint8);
      break;
    case OP_LTI16:
      m_registers[args[2]] = REG_AS(args[0], int16) < REG_AS(args[1], int16);
      break;
    case OP_LTU16:
      m_registers[args[2]] = REG_AS(args[0], uint16) < REG_AS(args[1], uint16);
      break;
    case OP_LTI32:
      m_registers[args[2]] = REG_AS(args[0], int32) < REG_AS(args[1], int32);
      break;
    case OP_LTU32:
      m_registers[args[2]] = REG_AS(args[0], uint32) < REG_AS(args[1], uint32);
      break;
    case OP_LTI64:
      m_registers[args[2]] = REG_AS(args[0], int64) < REG_AS(args[1], int64);
      break;
    case OP_LTU64:
      m_registers[args[2]] = REG_AS(args[0], uint64) < REG_AS(args[1], uint64);
      break;
    case OP_LTF32:
      m_registers[args[2]] = REG_AS(args[0], float32) < REG_AS(args[1], float32);
      break;
    case OP_LTF64:
      m_registers[args[2]] = REG_AS(args[0], float64) < REG_AS(args[1], float64);
      break;
    case OP_LTARR:
      m_registers[args[2]] = doArrayComparison(args[0], args[1], READ_U32ARG(3)) == LEFT_LT_RIGHT;
      break;
    case OP_LTEI8:
      m_registers[args[2]] = REG_AS(args[0], int8) <= REG_AS(args[1], int8);
      break;
    case OP_LTEU8:
      m_registers[args[2]] = REG_AS(args[0], uint8) <= REG_AS(args[1], uint8);
      break;
    case OP_LTEI16:
      m_registers[args[2]] = REG_AS(args[0], int16) <= REG_AS(args[1], int16);
      break;
    case OP_LTEU16:
      m_registers[args[2]] = REG_AS(args[0], uint16) <= REG_AS(args[1], uint16);
      break;
    case OP_LTEI32:
      m_registers[args[2]] = REG_AS(args[0], int32) <= REG_AS(args[1], int32);
      break;
    case OP_LTEU32:
      m_registers[args[2]] = REG_AS(args[0], uint32) <= REG_AS(args[1], uint32);
      break;
    case OP_LTEI64:
      m_registers[args[2]] = REG_AS(args[0], int64) <= REG_AS(args[1], int64);
      break;
    case OP_LTEU64:
      m_registers[args[2]] = REG_AS(args[0], uint64) <= REG_AS(args[1], uint64);
      break;
    case OP_LTEF32:
      m_registers[args[2]] = REG_AS(args[0], float32) <= REG_AS(args[1], float32);
      break;
    case OP_LTEF64:
      m_registers[args[2]] = REG_AS(args[0], float64) <= REG_AS(args[1], float64);
      break;
    case OP_LTEARR:
      m_registers[args[2]] = doArrayComparison(args[0], args[1], READ_U32ARG(3)) != LEFT_GT_RIGHT;
      break;
    case OP_STRREP8:
      m_registers[args[2]] = stringRepeat(REG_AS(args[0], void*), REG_AS(args[1], uint8), m_vm.getHeap());
      break;
    case OP_STRREP16:
      m_registers[args[2]] = stringRepeat(REG_AS(args[0], void*), REG_AS(args[1], uint16), m_vm.getHeap());
      break;
    case OP_STRREP32:
      m_registers[args[2]] = stringRepeat(REG_AS(args[0], void*), REG_AS(args[1], uint32), m_vm.getHeap());
      break;
    case OP_STRREP64:
      m_registers[args[2]] = stringRepeat(REG_AS(args[0], void*), REG_AS(args[1], uint64), m_vm.getHeap());
      break;
    // endregion Generated Instructions
    default:
      break;
  }

  ++m_registers[REGISTER_INSTR_COUNTER];
  goto begin;
}

bool Interpreter::doArrayEqualityCheck(const uint8 r1, const uint8 r2, const uint32 ti) const {
  const uint64 lAddr = m_registers[r1];
  const uint64 rAddr = m_registers[r2];

  if (ti == TI_STRING) {
    return m_vm.stringEquals(lAddr, rAddr);
  }

  const ScriptArrayType* arrType = static_cast<ScriptArrayType*>(m_vm.getTypes().lookupByIndex(ti));
  return m_vm.arrayEquals(lAddr, rAddr, arrType);
}

bool Interpreter::doStructEqualityCheck(const uint8 r1, const uint8 r2, const uint32 ti) const {
  const uint64 lAddr = m_registers[r1];
  const uint64 rAddr = m_registers[r2];

  ScriptStructType* sType = static_cast<ScriptStructType*>(m_vm.getTypes().lookupByIndex(ti));
  return m_vm.objectEquals(lAddr, rAddr, sType);
}

int8 Interpreter::doArrayComparison(const uint8 lhs, const uint8 rhs, const uint32 ti) const {
  const uint64 lAddr = m_registers[lhs];
  const uint64 rAddr = m_registers[rhs];

  if (ti == TI_STRING) {
    return m_vm.compareString(lAddr, rAddr);
  }

  const ScriptArrayType* arrType = static_cast<ScriptArrayType*>(m_vm.getTypes().lookupByIndex(ti));
  return m_vm.compareArray(lAddr, rAddr, arrType);
  return m_vm.compareArray(lAddr, rAddr, arrType);
}

void Interpreter::callNativeFunction(NativeScriptFunction* nFunc) {
  CallFrame* parent = getCallFrame();

  CallFrame* frame = pushNewFrame();
  frame->filename = "<native code>";
  frame->name = nFunc->name;
  frame->allocatedSize = 0;
  frame->stackBase = nullptr;
  frame->line = 0;
  frame->returnAddr = m_registers[REGISTER_INSTR_COUNTER] + 1;

  NativeCall call = NativeCall(nullptr, nullptr, 0);
  nFunc->callback(call);

  m_registers[REGISTER_RETURN_VALUE] = call.getReturnValue();
}
