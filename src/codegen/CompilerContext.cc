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
  reserveSpace(LENGTH_INSTRUCTION);
  *reinterpret_cast<opcode*>(m_buf + m_len) = code;
  m_len += LENGTH_OPCODE;
  m_argsStart = m_len;
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
}

void BytecodeWriter::writeInstructionCounter(const uint64 offset) const {
  *reinterpret_cast<uint32*>(m_buf + offset) = m_instrCount;
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

StringPoolAddress ConstStringPoolWriter::emplace(stringid id) {
  if (m_idToOffset.contains(id)) {
    return m_idToOffset[id];
  }

  int32 len = m_table.getlen(id);
  uint64 requiredcap = len + sizeof(uint32) + m_len;

  if (requiredcap > m_cap) {
    uint8* ndata = static_cast<uint8*>(malloc(requiredcap));
    if (!ndata) {
      throw std::runtime_error("Failed to resize string const pool buffer");
    }

    m_data = ndata;
    m_cap = requiredcap;
  }

  uint8* writeptr = m_data + m_len;

  *reinterpret_cast<uint32*>(writeptr) = len;
  writeptr += sizeof(uint32);

  char* charptr = reinterpret_cast<char*>(writeptr);
  m_table.copychars(id, charptr, len);

  uint64 off = m_len;
  m_len += sizeof(uint32) + len;
  m_idToOffset.emplace(id, off);

  return off;
}

uint64 ConstStringPoolWriter::getLength() const {
  return m_len;
}

uint8* ConstStringPoolWriter::getData() const {
  return m_data;
}

CompilerContext::CompilerContext(SemanticContext& ctx, uint64* registryBitset)
  : m_stringPool(ctx.getStrings()), m_semantics(ctx), m_registersInUse(registryBitset)
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

int32 CompilerContext::findFunctionIndex(stringid name, FunctionSignature* sign) {
  for (int32 i = 0; i < m_compiledFuncs.size(); i++) {
    CompiledFunction& func = m_compiledFuncs[i];
    if (func.name != name || func.signature != sign) {
      continue;
    }
    return i;
  }
  return -1;
}

uint32 CompilerContext::getStructConstructorIndex(ScriptStructType* type) {
  return m_structConstructors[type];
}

void CompilerContext::pushStructConstructor(ScriptStructType* type, uint32 idx) {
  m_structConstructors[type] = idx;
}

void CompilerContext::pushIncompleteCall(stringid name, FunctionSignature* sign, uint64 off) {
  m_incompleteCalls.emplace_back(name, sign, off);
}

uint32 CompilerContext::pushCompiledFunction(stringid name, uint32 start, FunctionSignature* sign) {
  uint32 idx = m_compiledFuncs.size();
  StringPoolAddress off = m_stringPool.emplace(name);

  CompiledFunction& comp = m_compiledFuncs.emplace_back(name, off, start, sign);

  for (auto it = m_incompleteCalls.begin(); it != m_incompleteCalls.end(); ) {
    IncompleteFunctonCall& icall = *it;

    if (comp.name != icall.name || comp.signature != sign) {
      ++it;
      continue;
    }

    m_writer.writeU32(icall.writeOffset, idx);
    m_incompleteCalls.erase(it);
  }

  return idx;
}

registeridopt CompilerContext::acquireRegister() const {
  uint64 used = *m_registersInUse;

  if (used == ALL_REGISTERS_USED) {
    return NO_REGISTER;
  }
  if (used == 0) {
    return 0;
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

bool CompilerContext::registerInUse(const registerid reg) const {
  const uint64 m = 1L << reg;
  return *m_registersInUse & m;
}

void CompilerContext::useRegister(const registerid reg) const {
  *m_registersInUse |= (1L << reg);
}

void CompilerContext::freeRegister(const registerid reg) const {
  *m_registersInUse &= ~(1L << reg);
}

BytecodeWriter& CompilerContext::getWriter() {
  return m_writer;
}

ConstStringPoolWriter& CompilerContext::getStringPool() {
  return m_stringPool;
}

SemanticContext& CompilerContext::getSemantics() const {
  return m_semantics;
}
