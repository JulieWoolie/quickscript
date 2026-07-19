#include "nativeinterface.h"

#define ARGGET(idx, type) return *((type*) (m_args + idx));

void NativeCall::setReturnValue(uint64 value) {
  m_returnValue = value;
}

void NativeCall::setF64ReturnValue(float64 value) {
  m_returnValue = std::bit_cast<uint64>(value);
}

void NativeCall::setF32ReturnValue(float32 value) {
  m_returnValue = std::bit_cast<uint32>(value);
}

bool NativeCall::getBoolArgument(uint32 idx) {
  return m_args[idx];
}

int8 NativeCall::getI8Argument(uint32 idx) {
  ARGGET(idx, int8)
}

uint8 NativeCall::getU8Argument(uint32 idx) {
  ARGGET(idx, uint8)
}

int16 NativeCall::getI16Argument(uint32 idx) {
  ARGGET(idx, int16)
}

uint16 NativeCall::getU16Argument(uint32 idx) {
  ARGGET(idx, uint16)
}

int32 NativeCall::getI32Argument(uint32 idx) {
  ARGGET(idx, int32)
}

uint32 NativeCall::getU32Argument(uint32 idx) {
  ARGGET(idx, uint32)
}

int64 NativeCall::getI64Argument(uint32 idx) {
  ARGGET(idx, int64)
}

uint64 NativeCall::getU64Argument(uint32 idx) {
  ARGGET(idx, uint64)
}

float32 NativeCall::getF32Argument(uint32 idx) {
  uint32 b = m_args[idx];
  return std::bit_cast<float32>(b);
}

float64 NativeCall::getF64Argument(uint32 idx) const {
  return std::bit_cast<float64>(m_args[idx]);
}

QsString NativeCall::getStrArgument(uint32 idx) {
  const uint64 ptr = getU64Argument(idx);
  return getScriptString(ptr);
}

QsArray NativeCall::getArrayArgument(uint32 idx) {
  uint64 ptr = getU64Argument(idx);
  uint32* addr = (uint32*) ptr;

  uint32 len = addr[0];

  QsArray str;
  str.capacity = len;
  str.data = (uint8*) (len+1);

  return str;
}

QsObject NativeCall::getObjectArgument(uint32 idx) {
  uint64 ptr = getU64Argument(idx);
  uint8* addr = (uint8*) ptr;

  QsObject obj;
  obj.data = addr;

  return obj;
}

QsString NativeCall::getScriptString(uint64 heapAddr) {
  uint32* addr = (uint32*) heapAddr;

  uint32 len = addr[0];

  QsString str;
  str.length = len;
  str.charData = (int8*) (len+1);

  return str;
}

uint8* NativeCall::allocHeap(uint64 size) {
  return (uint8*) malloc(size);
}

void NativeCall::heapFree(uint8* addr) {
  free(addr);
}

std::vector<NativeBinding>* Bindings::getBindings() {
  return &m_bindings;
}

void Bindings::putFunction(conststring name, FunctionSignature* signature, NativeFunction func) {
  m_bindings.emplace_back(name, (void*) func, signature);
}

