#include "qsni.h"

#include "qs/nativeinterface.hpp"
#include "qs/objects.hpp"
#include "qs/qsenv.hpp"
#include "qs/bytecode/bytecode_file.hpp"
#include "qs/interpreter/interpreter.hpp"
#include "qs/types/types.hpp"

#define ARRAY_READ_WRITE_FUNCS(shorthand, type) \
  type qsa_get##shorthand(const QsScriptArray array, const uint32 idx) {\
    return static_cast<type*>(getArrayDataStart(array))[idx];\
  }\
  void qsa_set##shorthand(const QsScriptArray array, const uint32 idx, const type value) {\
    static_cast<type*>(getArrayDataStart(array))[idx] = value;\
  }

#define OBJ_READ_WRITE_FUNCS(shorthand, type) \
  type qsobj_get##shorthand##Property(QsScriptObject obj, const uint64 offset) {\
    uint8* dataPtr = static_cast<uint8*>(obj) + 1 + offset;\
    return *reinterpret_cast<type*>(dataPtr);\
  }\
  void qsobj_set##shorthand##Property(QsScriptObject obj, const uint64 offset, const type value) {\
    uint8* dataPtr = static_cast<uint8*>(obj) + 1 + offset;\
    *reinterpret_cast<type*>(dataPtr) = value;\
  }

qstypekind qst_getTypeKind(const QsScriptType type) {
  return type->kind();
}

uint8 qst_getStackSize(const QsScriptType type) {
  return type->stackSizeBytes();
}

conststring qst_getTypeName(const QsScriptType type) {
  return type->getTypeName();
}

QsScriptType qsat_getComponentType(const QsScriptType type) {
  return static_cast<const ScriptArrayType*>(type)->getComponentType();
}

QsScriptType qsst_getPropertyType(const QsScriptType type, const uint32 propertyIndex) {
  return static_cast<const ScriptStructType*>(type)->getProperty(propertyIndex)->type;
}

conststring qsst_getPropertyName(const QsScriptType type, const uint32 propertyIndex) {
  return static_cast<const ScriptStructType*>(type)->getProperty(propertyIndex)->name.c_str();
}

uint64 qsst_getPropertyOffset(const QsScriptType type, const uint32 propertyIndex) {
  return static_cast<const ScriptStructType*>(type)->getProperty(propertyIndex)->offset;
}

int32 qsst_getPropertyIndex(const QsScriptType type, const conststring propName) {
  const ScriptStructType* structType = static_cast<const ScriptStructType*>(type);
  const uint32 propCount = structType->getPropertyCount();

  if (propCount == 0) {
    return PROPERTY_NOT_FOUND;
  }

  for (uint32 i = 0; i < propCount; i++) {
    if (structType->getProperty(i)->name != propName) {
      continue;
    }
    return i;
  }

  return PROPERTY_NOT_FOUND;
}

uint32 qsst_getPropertyCount(const QsScriptType type) {
  return static_cast<const ScriptStructType*>(type)->getPropertyCount();
}

uint64 qsst_getSize(const QsScriptType type) {
  return static_cast<const ScriptStructType*>(type)->getHeapSize();
}

uint8 qsst_getAlignment(const QsScriptType type) {
  return static_cast<const ScriptStructType*>(type)->getAlignment();
}

uint32 qfs_getArgumentCount(const QsScriptType type) {
  return static_cast<const FunctionSignature*>(type)->getArgumentsLength();
}

QsScriptType qfs_getArgumentType(const QsScriptType type, const uint32 argIndex) {
  return static_cast<const FunctionSignature*>(type)->getArgumentType(argIndex);
}

boolean qfs_isVariadic(const QsScriptType type) {
  return static_cast<const FunctionSignature*>(type)->isVariadic();
}

QsScriptType qfs_getReturnType(const QsScriptType type) {
  return static_cast<const FunctionSignature*>(type)->getReturnType();
}

uint64 qsa_getLength(const QsScriptArray array) {
  return readQsArrayLength(array);
}

uint8* qsa_getData(const QsScriptArray array) {
  return static_cast<uint8*>(getArrayDataStart(array));
}

int8* qsa_getCharData(const QsScriptArray array) {
  return static_cast<int8*>(getArrayDataStart(array));
}

uint32 qsa_getRefCounter(const QsScriptArray array) {
  return getArrayRefCounter(array);
}

void qsa_setRefCounter(const QsScriptArray array, const uint32 refCounter) {
  setArrayRefCounter(array, refCounter);
}

ARRAY_READ_WRITE_FUNCS(I8, int8)
ARRAY_READ_WRITE_FUNCS(U8, uint8)
ARRAY_READ_WRITE_FUNCS(I16, int16)
ARRAY_READ_WRITE_FUNCS(U16, uint16)
ARRAY_READ_WRITE_FUNCS(I32, int32)
ARRAY_READ_WRITE_FUNCS(U32, uint32)
ARRAY_READ_WRITE_FUNCS(I64, int64)
ARRAY_READ_WRITE_FUNCS(U64, uint64)
ARRAY_READ_WRITE_FUNCS(F32, float32)
ARRAY_READ_WRITE_FUNCS(F64, float64)
ARRAY_READ_WRITE_FUNCS(Object, QsScriptObject)
ARRAY_READ_WRITE_FUNCS(Array, QsScriptArray)

uint32 qsobj_getRefCounter(const QsScriptObject obj) {
  return getArrayRefCounter(obj);
}

void qsobj_setRefCounter(const QsScriptObject obj, const uint32 refCounter) {
  setArrayRefCounter(obj, refCounter);
}

OBJ_READ_WRITE_FUNCS(I8, int8)
OBJ_READ_WRITE_FUNCS(U8, uint8)
OBJ_READ_WRITE_FUNCS(I16, int16)
OBJ_READ_WRITE_FUNCS(U16, uint16)
OBJ_READ_WRITE_FUNCS(I32, int32)
OBJ_READ_WRITE_FUNCS(U32, uint32)
OBJ_READ_WRITE_FUNCS(I64, int64)
OBJ_READ_WRITE_FUNCS(U64, uint64)
OBJ_READ_WRITE_FUNCS(F32, float32)
OBJ_READ_WRITE_FUNCS(F64, float64)
OBJ_READ_WRITE_FUNCS(Object, QsScriptObject)
OBJ_READ_WRITE_FUNCS(Array, QsScriptArray)

uint64 qsc_getReturnValue(const QsNativeCall call) {
  return call->getReturnValue();
}
boolean qsc_isFailedCall(const QsNativeCall call) {
  return call->isFailedCall();
}

conststring qsc_getErrorMessage(const QsNativeCall call) {
  return call->getErrorMessage().c_str();
}

void qsc_setReturn(const QsNativeCall call, const uint64 value) {
  call->setReturn(value);
}

void qsc_setF64Return(const QsNativeCall call, const float64 value) {
  call->setF64Return(value);
}

void qsc_setF32Return(const QsNativeCall call, const float32 value) {
  call->setF32Return(value);
}

void qsc_throwError(const QsNativeCall call, const conststring errorMessage) {
  call->setError(errorMessage);
}

QsScriptType qsc_getArgumentType(const QsNativeCall call, const uint32 idx) {
  return call->getArgumentType(idx);
}

boolean qsc_getBoolArgument(const QsNativeCall call, const uint32 idx) {
  return call->getBoolArgument(idx);
}

uint8 qsc_getU8Argument(const QsNativeCall call, const uint32 idx) {
  return call->getU8Argument(idx);
}

int8 qsc_getI8Argument(const QsNativeCall call, const uint32 idx) {
  return call->getI8Argument(idx);
}

uint16 qsc_getU16Argument(const QsNativeCall call, const uint32 idx) {
  return call->getU16Argument(idx);
}

int16 qsc_getI16Argument(const QsNativeCall call, const uint32 idx) {
  return call->getI16Argument(idx);
}

uint32 qsc_getU32Argument(const QsNativeCall call, const uint32 idx) {
  return call->getU32Argument(idx);
}

int32 qsc_getI32Argument(const QsNativeCall call, const uint32 idx) {
  return call->getI32Argument(idx);
}

uint64 qsc_getU64Argument(const QsNativeCall call, const uint32 idx) {
  return call->getU64Argument(idx);
}

int64 qsc_getI64Argument(const QsNativeCall call, const uint32 idx) {
  return call->getI64Argument(idx);
}

float32 qsc_getF32Argument(const QsNativeCall call, const uint32 idx) {
  return call->getF32Argument(idx);
}

float64 qsc_getF64Argument(const QsNativeCall call, const uint32 idx) {
  return call->getF64Argument(idx);
}

QsScriptArray qsc_getArrayArgument(const QsNativeCall call, const uint32 idx) {
  return reinterpret_cast<QsScriptArray>(call->getU64Argument(idx));
}

QsScriptObject qsc_getObjectArgument(QsNativeCall call, uint32 idx) {
  return reinterpret_cast<QsScriptObject>(call->getU64Argument(idx));
}

uint32 qsbf_loadBytecodeFile(const void* buf, const uint64 bufferSize, QsBytecodeFile* fileOut) {
  BytecodeFile& bf = BytecodeFile::create();
  const BytecodeReadResult result = deserializeBytecodeFile(static_cast<const uint8*>(buf), bufferSize, bf);

  if (result != IR_RESULT_OK) {
    BytecodeFile::destroy(bf);
  } else {
    *fileOut = &bf;
  }

  return result;
}

conststring qsbf_getResultString(const uint32 result) {
  return getReadResultMessage(result);
}

void qsbf_freeBytecodeFile(QsBytecodeFile bFile) {
  BytecodeFile::destroy(*bFile);
}

QsEnv qse_createEnv() {
  return new QsEnvironment();
}

void qse_freeEnv(const QsEnv env) {
  delete env;
}

void qse_addStandardLibraries(QsEnv env) {
  // idk man
}

void qse_addLibraryDirectory(const QsEnv env, const conststring dirPath) {
  env->getLibraryDirectories().push_back(dirPath);
}

void qse_registerNative(
  const QsEnv env,
  const conststring ns,
  const conststring funcName,
  const conststring signature,
  const QsNativeFunction func
) {
  env->registerNative(ns, funcName, signature, func);
}

void qse_registerNatives(const QsEnv env, const conststring ns, const uint32 n, ...) {
  va_list l;
  va_start(l, n);

  for (uint32 i = 0; i < n; i++) {
    const conststring fName = va_arg(l, conststring);
    const conststring sig = va_arg(l, conststring);
    const QsNativeFunction func = va_arg(l, QsNativeFunction);
    env->registerNative(ns, fName, sig, func);
  }

  va_end(l);
}

void qse_setStatementInlining(const QsEnv env, const boolean state) {
  env->getOptions().statOptimizing = state;
}

void qse_setExpressionInlining(const QsEnv env, const boolean state) {
  env->getOptions().exprOptimizing = state;
}

void qse_setAssertsCompiled(const QsEnv env, const boolean state) {
  env->getOptions().includeAsserts = state;
}

QsVirtualMachine qsvm_createVirtualMachine(QsEnv env) {
  return new VirtualMachine();
}

void qsvm_freeVirtualMachine(const QsVirtualMachine vm) {
  delete vm;
}

void qsvm_loadBytecodeFile(QsVirtualMachine vm, QsBytecodeFile bFile, conststring name) {

}

int32 qsvm_beginExecution(QsVirtualMachine vm, uint32 argc, cstring* argv) {

}

uint64 qsvm_callFunction(QsVirtualMachine vm, QsFunction sf) {

}

conststring qsvm_toString(const QsVirtualMachine vm, const uint64 qsValue, const QsScriptType type) {
  std::string result;
  const typeindex idx = vm->getTypes().findIndex(type);

  vm->toString(result, idx, qsValue);

  if (result.empty()) {
    return "";
  }

  int8* buf = static_cast<int8*>(malloc(result.length() + 1));
  memcpy(buf, result.data(), result.length());
  buf[result.length()] = '\0';

  return buf;
}

boolean qsvm_equals(QsVirtualMachine vm, uint64 a, uint64 b, QsScriptType type) {
  const typeindex idx = vm->getTypes().findIndex(type);
  return vm->equals(a, b, idx);
}

QsScriptObject qsvm_makeStruct(QsVirtualMachine vm, QsScriptType structType) {

}

void qsvm_freeStruct(const QsVirtualMachine vm, const QsScriptObject obj) {
  vm->getHeap().freeMemory(obj);
}

QsScriptArray qsvm_makeArray(QsVirtualMachine vm, QsScriptType arrayType, uint32 capacity) {

}

void qsvm_freeArray(const QsVirtualMachine vm, const QsScriptArray array) {
  vm->getHeap().freeMemory(array);
}

boolean qsc_compileSourceFile(
  conststring sourceName,
  conststring source,
  QsEnv env,
  QsBytecodeFile* fileOut
) {
  return env->compileSourceFile(source, sourceName, fileOut);
}
