#include "nativeinterface.h"

#include <bit>

#include "strings/stringreader.h"
#include "types/ScriptArrayType.h"

#define ARGUMENT_GET_METHOD(name, type) \
type NativeCall::name(const uint32 idx) const {\
  return *reinterpret_cast<type*>(&m_args[idx]);\
}

NativeCall::NativeCall(ScriptType** argType, uint64* args, uint32 argCount)
  : m_args(args),
    m_types(argType),
    m_argCount(argCount)
{

}

uint64 NativeCall::getReturnValue() const {
  return m_returnValue;
}

bool NativeCall::isFailedCall() const {
  return m_failedCall;
}

const std::string& NativeCall::getErrorMessage() const {
  return m_error;
}

void NativeCall::setReturn(const uint64 value) {
  m_returnValue = value;
}

void NativeCall::setF64Return(const float64 value) {
  m_returnValue = std::bit_cast<uint64>(value);
}

void NativeCall::setF32Return(const float32 value) {
  *reinterpret_cast<float32*>(&m_returnValue) = value;
}

void NativeCall::setError(const std::string& errorMessage) {
  m_error = errorMessage;
}

ARGUMENT_GET_METHOD(getBoolArgument, bool)
ARGUMENT_GET_METHOD(getU8Argument, uint8)
ARGUMENT_GET_METHOD(getI8Argument, int8)
ARGUMENT_GET_METHOD(getU16Argument, uint16)
ARGUMENT_GET_METHOD(getI16Argument, int16)
ARGUMENT_GET_METHOD(getU32Argument, uint32)
ARGUMENT_GET_METHOD(getI32Argument, int32)
ARGUMENT_GET_METHOD(getU64Argument, uint64)
ARGUMENT_GET_METHOD(getI64Argument, int64)
ARGUMENT_GET_METHOD(getF32Argument, float32)
ARGUMENT_GET_METHOD(getF64Argument, float64)

QsArray NativeCall::getArrayArgument(const uint32 idx) const {
  void* addr = reinterpret_cast<void*>(m_args[idx]);
  return castToQsArray(addr);
}

QsObject NativeCall::getObjectArgument(const uint32 idx) const {
  void* addr = reinterpret_cast<void*>(m_args[idx]);
  return castToQsObject(addr);
}
QsArray NativeCall::getScriptArray(const uint64 heapAddr) const {
  return castToQsArray(reinterpret_cast<void*>(heapAddr));
}

ScriptType* NativeCall::getArgumentType(const uint32 idx) const {
  return m_types[idx];
}

NativeBinding::NativeBinding(const conststring name) : m_name(name) {
}

conststring NativeBinding::getName() const {
  return m_name;
}

NativeFunctionBinding::NativeFunctionBinding(
  const conststring name,
  const NativeFunction func,
  const FunctionSignature* signature)
  : NativeBinding(name),
    m_func(func),
    m_signature(signature)
{

}

NativeFunctionBinding* NativeFunctionBinding::create(
  const conststring name,
  const NativeFunction func,
  const FunctionSignature* signature
) {
  return new NativeFunctionBinding(name, func, signature);
}

void NativeFunctionBinding::destroy(NativeFunctionBinding* bind) {
  delete bind;
}

bindingtype NativeFunctionBinding::btype() const {
  return BINDTYPE_FUNCTION;
}

NativeFunction NativeFunctionBinding::getFunction() const {
  return m_func;
}

const FunctionSignature* NativeFunctionBinding::getSignature() const {
  return m_signature;
}

BindingsObject::BindingsObject() {

}

BindingsObject* BindingsObject::create() {
  return new BindingsObject();
}

void BindingsObject::destroy(BindingsObject* obj) {
  delete obj;
}

const std::vector<NativeBinding*>& BindingsObject::getBindings() const {
  return m_bindings;
}

NativeFunctionBinding* BindingsObject::addFunctionBinding(
  const conststring name,
  const FunctionSignature* signature,
  const NativeFunction func
) {
  NativeFunctionBinding* binding = NativeFunctionBinding::create(name, func, signature);
  m_bindings.push_back(binding);
  return binding;
}