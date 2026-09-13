#define _QS_IMPL_
#include "qsni.h"

#include "qs/compiler_opts.hpp"
#include "qs/nativeinterface.hpp"
#include "qs/stringtable.hpp"
#include "qs/analysis/analyzer.hpp"
#include "qs/analysis/transformer.hpp"
#include "qs/bytecode/bytecode_file.hpp"
#include "qs/codegen/compiler.hpp"
#include "qs/interpreter/interpreter.hpp"
#include "qs/parse/lexer.hpp"
#include "qs/parse/parser.hpp"
#include "qs/types/types.hpp"

struct QsEnvironment {
  BindingsObject* bindings;
  CompilationOptions options;
};

qstypekind qst_getTypeKind(const QsScriptType type) {
  return type->kind();
}

uint8 qst_getStackSize(const QsScriptType type) {
  return type->stackSizeBytes();
}

conststring qst_getTypeName(const QsScriptType type) {
  return type->getTypeName();
}

QsScriptType qsarray_getComponentType(const QsScriptType type) {
  return static_cast<const ScriptArrayType*>(type)->getComponentType();
}

QsScriptType qstruct_getPropertyType(const QsScriptType type, const uint32 propertyIndex) {
  return static_cast<const ScriptStructType*>(type)->getProperty(propertyIndex)->type;
}

conststring qstruct_getPropertyName(const QsScriptType type, const uint32 propertyIndex) {
  return static_cast<const ScriptStructType*>(type)->getProperty(propertyIndex)->name.c_str();
}

uint64 qstruct_getPropertyOffset(const QsScriptType type, const uint32 propertyIndex) {
  return static_cast<const ScriptStructType*>(type)->getProperty(propertyIndex)->offset;
}

int32 qstruct_getPropertyIndex(const QsScriptType type, const conststring propName) {
  const ScriptStructType* sType = static_cast<const ScriptStructType*>(type);
  const uint32 propCount = sType->getPropertyCount();

  for (uint32 i = 0; i < propCount; i++) {
    if (sType->getProperty(i)->name == propName) {
      return i;
    }
  }

  return -1;
}

uint32 qstruct_getPropertyCount(const QsScriptType type) {
  return static_cast<const ScriptStructType*>(type)->getPropertyCount();
}

uint64 qstruct_getSize(const QsScriptType type) {
  return static_cast<const ScriptStructType*>(type)->getHeapSize();
}

uint8 qstruct_getAlignment(const QsScriptType type) {
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

boolean qsc_getBoolArgument(const QsNativeCall call, const uint32 idx) {
  return call->getBoolArgument(idx);
}

QsScriptType qsc_getArgumentType(const QsNativeCall call, const uint32 idx) {
  return call->getArgumentType(idx);
}

#define LIB_ARG_GETTER(type, shorthand) \
  type qsc_get##shorthand##Argument(const QsNativeCall call, const uint32 idx) {\
    return call->get##shorthand##Argument(idx);\
  }

LIB_ARG_GETTER(int8, I8)
LIB_ARG_GETTER(uint8, U8)
LIB_ARG_GETTER(int16, I16)
LIB_ARG_GETTER(uint16, U16)
LIB_ARG_GETTER(int32, I32)
LIB_ARG_GETTER(uint32, U32)
LIB_ARG_GETTER(int64, I64)
LIB_ARG_GETTER(uint64, U64)
LIB_ARG_GETTER(float32, F32)
LIB_ARG_GETTER(float64, F64)

uint32 qsbf_loadBytecodeFile(const void* buf, const uint64 bufferSize, QsBytecodeFile* fileOut) {
  BytecodeFile& bf = BytecodeFile::create();
  BytecodeReadResult res = deserializeBytecodeFile(static_cast<const uint8*>(buf), bufferSize, bf);

  if (res != IR_RESULT_OK) {
    BytecodeFile::destroy(bf);
    return res;
  }

  *fileOut = &bf;
  return IR_RESULT_OK;
}

conststring qsbf_getResultString(const uint32 result) {
  return getReadResultMessage(result);
}

void qsbf_freeBytecodeFile(const QsBytecodeFile bFile) {
  BytecodeFile::destroy(*static_cast<BytecodeFile*>(bFile));
}

QsEnv qse_createEnv() {
  return new QsEnvironment();
}

void qse_freeEnv(const QsEnv env) {
  delete static_cast<QsEnvironment*>(env);
}

void qse_addStandardLibraries(const QsEnv env) {
  // idk atm
}

void qse_addLibraryDirectory(const QsEnv env, const conststring dirPath) {
  // idk atm
}

void qse_registerNative(
  QsEnv env,
  conststring funcName,
  conststring signature,
  QsNativeFunction func
) {
  QsEnvironment* e = static_cast<QsEnvironment*>(env);

}

void qse_setStatementInlining(const QsEnv env, const boolean state) {
  static_cast<QsEnvironment*>(env)->options.statOptimizing = state;
}

void qse_setExpressionInlining(const QsEnv env, const boolean state) {
  static_cast<QsEnvironment*>(env)->options.exprOptimizing = state;
}

void qse_setAssertsCompiled(const QsEnv env, const boolean state) {
  static_cast<QsEnvironment*>(env)->options.includeAsserts = state;
}

QsVirtualMachine qsvm_createVirtualMachine(const QsEnv env) {
  VirtualMachine* vm = new VirtualMachine();
  const QsEnvironment* e = static_cast<QsEnvironment*>(env);

  if (e->bindings) {
    vm->addBindings(e->bindings);
  }

  return vm;
}

void qsvm_freeVirtualMachine(const QsVirtualMachine vm) {
  delete static_cast<VirtualMachine*>(vm);
}

void qsvm_loadBytecodeFile(const QsVirtualMachine vm, const QsBytecodeFile bFile, const conststring name) {
  static_cast<VirtualMachine*>(vm)->addBytecodeFile(*static_cast<const BytecodeFile*>(bFile), name);
}

int32 qsvm_beginExecution(QsVirtualMachine vm) {

}

uint64 qsvm_callFunction(QsVirtualMachine vm, QsFunction sf) {

}

conststring qsvm_toString(const QsVirtualMachine vm, const uint64 qsValue, const QsScriptType type) {
  VirtualMachine* qsvm = static_cast<VirtualMachine*>(vm);
  const typeindex typeIdx = qsvm->getTypes().findIndex(static_cast<const ScriptType*>(type));
  std::string result = "";
  qsvm->toString(result, typeIdx, qsValue);

  const uint32 strSize = result.length();

  int8* buf = static_cast<int8*>(malloc(strSize + 1));
  memcpy(buf, result.data(), strSize);
  buf[strSize] = 0;

  return buf;
}

boolean qsvm_equals(const QsVirtualMachine vm, const uint64 a, const uint64 b, const QsScriptType type) {
  const typeindex typeIdx = vm->getTypes().findIndex(type);
  return vm->equals(a, b, typeIdx);
}

boolean qsc_compileSourceFile(conststring sourceName, conststring source, QsEnv env, QsBytecodeFile* fileOut) {
  QsEnvironment* e = static_cast<QsEnvironment*>(env);

  std::string fileContent = source;

  CompilerErrors errors = CompilerErrors(&fileContent, sourceName);
  StringTable table = StringTable();
  TokenList tokens = TokenList();

  Lexer l = Lexer(fileContent, &tokens, &table, &errors);

  try {
    l.next();
    l.lex();
  } catch (std::runtime_error& error) {
    return false;
  }

  NoFreeAllocator alloc = NoFreeAllocator();

  Parser p = Parser(&tokens, &alloc, &errors, &table);
  ScriptFileStatement* sfs = nullptr;

  try {
    sfs = p.parse();
  } catch (std::runtime_error& error) {
    return false;
  }

  TypeTable types = TypeTable();

  BindingsObject* bindings = nullptr;
  bool tempBindings = false;

  if (e->bindings) {
    bindings = e->bindings;
  } else {
    bindings = BindingsObject::create();
    tempBindings = true;
  }

  SemanticContext ctx = SemanticContext(types, table, errors, alloc, e->options, bindings);
  runSemanticAnalysis(sfs, ctx);

  if (errors.getErrorCount() != 0) {
    if (tempBindings) {
      BindingsObject::destroy(bindings);
    }
    return false;
  }

  runSemanticTransformer(ctx, sfs);

  BytecodeFile& bFile = compile(ctx);
  *fileOut = &bFile;

  if (tempBindings) {
    BindingsObject::destroy(bindings);
  }

  return true;
}

void qs_onLoadNativeModule(QsVirtualMachine vm) {

}