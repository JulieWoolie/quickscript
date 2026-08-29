#include "CompilerContext.h"

#include <stdexcept>

#define RUNTIME_CHECKS

#define APPEND_METHOD(name, bytes, type) \
  void BytecodeWriter::name(type v) {\
    reserveSpace(bytes);\
    *reinterpret_cast<type*>(m_buf + m_len) = v;\
    m_len += bytes;\
  }

APPEND_METHOD(appendU8, 1, uint8)
APPEND_METHOD(appendI8, 1, int8)
APPEND_METHOD(appendU16, 2, uint16)
APPEND_METHOD(appendI16, 2, int16)
APPEND_METHOD(appendU32, 4, uint32)
APPEND_METHOD(appendI32, 4, int32)
APPEND_METHOD(appendU64, 8, uint64)
APPEND_METHOD(appendI64, 8, int64)
APPEND_METHOD(appendF32, 4, float32)
APPEND_METHOD(appendF64, 8, float64)

void BytecodeWriter::reserveSpace(uint64 memsize) {
  uint64 nsize = m_len + memsize;
  if (nsize < m_cap) {
    return;
  }

  const uint64 ncap = m_cap + 1024;
  uint8* ndata = static_cast<uint8*>(realloc(m_buf, ncap));

  if (!ndata) {
    throw std::runtime_error("Failed to grow bytecode buffer");
  }

  m_buf = ndata;
  m_cap = ncap;
}

void BytecodeWriter::startInstr(opcode code) {
#ifdef RUNTIME_CHECKS
  if (m_instrStarted) {
    std::string error = "Trying to write instruction before calling end on previous one!";

    const opcode startedCode = *reinterpret_cast<opcode*>(m_buf + m_argsStart - LENGTH_OPCODE);
    error.append("\n  Started on: ");
    error.append(opcode_name(startedCode));

    throw std::runtime_error(error);
  }
#endif

  reserveSpace(LENGTH_INSTRUCTION);
  *reinterpret_cast<opcode*>(m_buf + m_len) = code;
  m_len += LENGTH_OPCODE;
  m_argsStart = m_len;
  m_instrStarted = true;
}

void BytecodeWriter::endInstr() {
  const int64 end = static_cast<int64>(m_argsStart) + LENGTH_ARGS;
  const int64 rem = end - static_cast<int64>(m_len);

#ifdef RUNTIME_CHECKS
  if (rem < 0) {
    throw std::runtime_error("Too many bytes written for instruction");
  }
#endif

  memset(m_buf + m_len, 0, rem);
  m_instrCount++;
  m_instrStarted = false;
  m_len += rem;
}

void BytecodeWriter::writeInstructionCounter(const uint64 offset) const {
  *reinterpret_cast<uint32*>(m_buf + offset) = m_instrCount;
}

void BytecodeWriter::writeInstructionCounter(uint64 offset, uint32 instrCount) const {
  *reinterpret_cast<uint32*>(m_buf + offset) = instrCount;
}

void BytecodeWriter::writeU32(uint64 offset, uint32 val) {
#ifdef RUNTIME_CHECKS
  if (offset > m_cap) {
    throw std::out_of_range("writeU32 called with offset that's out of range");
  }
#endif
  *reinterpret_cast<uint32*>(m_buf + offset) = val;
}

uint32 BytecodeWriter::getInstructionCounter() const {
  return m_instrCount;
}

uint64 BytecodeWriter::getAddress() const {
  return m_len;
}
uint8* BytecodeWriter::getBuffer() const {
  return m_buf;
}
uint32 BytecodeWriter::getLength() const {
  return m_len;
}

ConstStringPoolWriter::ConstStringPoolWriter(StringTable& table): m_table(table) {

}

StringPoolAddress ConstStringPoolWriter::emplaceData(const conststring data, const uint32 len) {
  const uint64 requiredCap = len + sizeof(uint32) + m_len;

  if (requiredCap > m_cap) {
    uint8* ndata = static_cast<uint8*>(realloc(m_data, requiredCap));
    if (!ndata) {
      throw std::runtime_error("Failed to resize string const pool buffer");
    }

    m_data = ndata;
    m_cap = requiredCap;
  }

  uint8* writePointer = m_data + m_len;

  *reinterpret_cast<uint32*>(writePointer) = len;
  writePointer += sizeof(uint32);

  char* charPointer = reinterpret_cast<char*>(writePointer);
  memcpy(charPointer, data, len);

  const uint64 off = m_len;
  m_len += sizeof(uint32) + len;

  return off;
}

StringPoolAddress ConstStringPoolWriter::emplace(stringid id) {
  if (m_idToOffset.contains(id)) {
    return m_idToOffset[id];
  }

  StringPoolAddress off = emplaceString(id->data, id->len);
  m_idToOffset.emplace(id, off);

  return off;
}

StringPoolAddress ConstStringPoolWriter::emplaceString(const conststring data, const uint32 len) {
  uint64 off = 0;

  while (off < m_len) {
    const uint32 foundLen = *reinterpret_cast<uint32*>(m_data + off);

    if (foundLen != len) {
      off += foundLen + sizeof(uint32);
      continue;
    }

    conststring emplaced = reinterpret_cast<conststring>(m_data + off + sizeof(uint32));
    bool failed = false;

    for (uint32 i = 0; i < len; i++) {
      if (data[i] == emplaced[i]) {
        continue;
      }
      failed = true;
      break;
    }

    if (failed) {
      off += foundLen + sizeof(uint32);
      continue;
    }

    return off;
  }

  return emplaceData(data, len);
}

uint64 ConstStringPoolWriter::getLength() const {
  return m_len;
}

uint8* ConstStringPoolWriter::getData() const {
  return m_data;
}

TypeReferenceCounter::TypeReferenceCounter(const uint32 size) {
  const uint64 memSize = size * sizeof(uint32);

  uint32* buf = static_cast<uint32*>(malloc(memSize));
  memset(buf, 0, memSize);

  m_counters = buf;
  m_size = size;
}

void TypeReferenceCounter::incrementCounter(const typeindex index) const {
  ++m_counters[index];
}

uint32 TypeReferenceCounter::getReferenceCount(const typeindex index) const {
  if (index >= m_size) {
    return 0;
  }
  return m_counters[index];
}

uint32 TypeReferenceCounter::getReferencedNonConstTypes() const {
  uint32 res = 0;
  for (uint32 i = FIRST_NON_RESERVED_TYPE_INDEX; i < m_size; i++) {
    res += m_counters[i] != 0;
  }
  return res;
}

CompilerContext::CompilerContext(SemanticContext& ctx, RegisterBitSet* registryBitset)
  : m_stringPool(ctx.getStrings()),
    m_typeRefCounter(ctx.getTypes().size()),
    m_semantics(ctx),
    m_registersInUse(registryBitset)
{

}

void CompilerContext::enqueueFunction(FunctionDeclStatement* stat) {
  m_funcQueue.push_back(stat);
}

FunctionDeclStatement* CompilerContext::pollQueuedFunction() {
  if (m_funcQueue.empty()) {
    return nullptr;
  }

  FunctionDeclStatement* first = m_funcQueue.front();
  m_funcQueue.erase(m_funcQueue.begin());
  return first;
}

int32 CompilerContext::findFunctionIndex(LocalFuncSymbol* sym) const {
  for (int32 i = 0; i < m_compiledFuncs.size(); i++) {
    const CompiledFunction& func = m_compiledFuncs[i];
    if (func.functionSymbol != sym) {
      continue;
    }
    return i;
  }
  return -1;
}

void CompilerContext::pushIncompleteCall(LocalFuncSymbol* lfs, uint64 off) {
  m_incompleteCalls.emplace_back(lfs, off);
}

uint32 CompilerContext::pushCompiledFunction(LocalFuncSymbol* lfs, uint32 start) {
  uint32 idx = m_compiledFuncs.size();
  StringPoolAddress off = m_stringPool.emplace(lfs->getName());

  CompiledFunction& comp = m_compiledFuncs.emplace_back(lfs, off, start);

  for (auto it = m_incompleteCalls.begin(); it != m_incompleteCalls.end(); ) {
    IncompleteFunctonCall& icall = *it;

    if (icall.functionSymbol != comp.functionSymbol) {
      ++it;
      continue;
    }

    m_writer.writeU32(icall.writeOffset, idx);
    m_incompleteCalls.erase(it);
  }

  return idx;
}

RegisterIdOpt CompilerContext::acquireRegister() const {
  uint64 used = *m_registersInUse;

  if (used == ALL_REGISTERS_USED) {
    return NO_REGISTER;
  }

  uint64 m;
  for (int8 i = 0; i < 64; i++) {
    m = 1 << i;
    if (m & used) {
      continue;
    }

    *m_registersInUse |= m;
    return i;
  }

  return NO_REGISTER;
}

bool CompilerContext::registerInUse(const RegisterId reg) const {
  const uint64 m = 1L << reg;
  return *m_registersInUse & m;
}

void CompilerContext::useRegister(const RegisterId reg) const {
  *m_registersInUse |= (1L << reg);
}

void CompilerContext::freeRegister(const RegisterId reg) const {
  *m_registersInUse &= ~(1L << reg);
}

RegisterBitSet* CompilerContext::getUsedRegistries() const {
  return m_registersInUse;
}

BytecodeWriter& CompilerContext::getWriter() {
  return m_writer;
}

ConstStringPoolWriter& CompilerContext::getStringPool() {
  return m_stringPool;
}

TypeReferenceCounter& CompilerContext::getTypeRefCounter() {
  return m_typeRefCounter;
}

SemanticContext& CompilerContext::getSemantics() const {
  return m_semantics;
}

Scope* CompilerContext::getCurrentScope() const {
  return m_currentScope;
}

void CompilerContext::setCurrentScope(Scope* scope) {
  m_currentScope = scope;
}

std::vector<ControlFlowCall>& CompilerContext::getControlFlowCalls() {
  return m_controlFlowCalls;
}

std::vector<CompiledFunction>& CompilerContext::getCompiledFunctions() {
  return m_compiledFuncs;
}

bool CompilerContext::wasReturnCalled() const {
  return m_returned;
}

void CompilerContext::setReturnCalled(bool b) {
  m_returned = b;
}

void CompilerContext::countTypeReference(const ScriptType* type) const {
  const typekind kind = type->kind();

  const int64 bIdx = m_semantics.getTypes().findIndex(type);
  if (bIdx == -1) {
    throw std::runtime_error("Type not in type table");
  }

  m_typeRefCounter.incrementCounter(bIdx);

  switch (kind) {
    case TK_ARRAY: {
      const ScriptArrayType* arrType = static_cast<const ScriptArrayType*>(type);
      countTypeReference(arrType->getComponentType());
      break;
    }
    case TK_FUNC: {
      const FunctionSignature* sign = static_cast<const FunctionSignature*>(type);
      countTypeReference(sign->getReturnType());
      for (uint32 i = 0; i < sign->getArgumentsLength(); i++) {
        countTypeReference(sign->getArgumentType(i));
      }
      break;
    }
    case TK_STRUCT: {
      const ScriptStructType* sType = static_cast<const ScriptStructType*>(type);
      const uint64 props = sType->getPropertyCount();

      for (uint32 i = 0; i < props; i++) {
        countTypeReference(sType->getProperty(i)->type);
      }

      break;
    }

    default:
      break;
  }
}
