#ifndef QSNI
#define QSNI

#define QS_EXPORT __declspec(dllexport)
#define QS_IMPORT __declspec(dllimport)
#define QS_CALL

#ifdef _QS_IMPL_
#define QS_API QS_EXPORT
#else
#define QS_FUNC QS_IMPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef long long int64;
typedef unsigned long long uint64;
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
typedef char int8;
typedef unsigned char uint8;
typedef unsigned char boolean;

typedef float float32;
typedef double float64;

typedef char* cstring;
typedef const char* conststring;


#define TK_UNKNOWN    0
#define TK_PRIMITIVE  1
#define TK_STRING     2
#define TK_STRUCT     3
#define TK_ARRAY      4
#define TK_FUNC       5
#define TK_VOID       6
#define TK_CLOSURE    7
typedef uint8 qstypekind;

typedef const struct ScriptType* QsScriptType;
typedef struct NativeCall* QsNativeCall;
typedef struct BytecodeFile* QsBytecodeFile;
typedef struct VirtualMachine* QsVirtualMachine;
typedef void* QsEnv;
typedef void* QsFunction;

typedef void (*QsNativeFunction)(QsVirtualMachine vm, QsNativeCall call);


QS_API qstypekind QS_CALL qst_getTypeKind(QsScriptType type);
QS_API uint8 QS_CALL qst_getStackSize(QsScriptType type);
QS_API conststring QS_CALL qst_getTypeName(QsScriptType type);

QS_API QsScriptType QS_CALL qsarray_getComponentType(QsScriptType type);

QS_API QsScriptType QS_CALL qstruct_getPropertyType(QsScriptType type, uint32 propertyIndex);
QS_API conststring QS_CALL qstruct_getPropertyName(QsScriptType type, uint32 propertyIndex);
QS_API uint64 QS_CALL qstruct_getPropertyOffset(QsScriptType type, uint32 propertyIndex);
QS_API int32 QS_CALL qstruct_getPropertyIndex(QsScriptType type, conststring propName);
QS_API uint32 QS_CALL qstruct_getPropertyCount(QsScriptType type);
QS_API uint64 QS_CALL qstruct_getSize(QsScriptType type);
QS_API uint8 QS_CALL qstruct_getAlignment(QsScriptType type);

QS_API uint32 QS_CALL qfs_getArgumentCount(QsScriptType type);
QS_API QsScriptType QS_CALL qfs_getArgumentType(QsScriptType type, uint32 argIndex);
QS_API boolean QS_CALL qfs_isVariadic(QsScriptType type);
QS_API QsScriptType QS_CALL qfs_getReturnType(QsScriptType type);


QS_API uint64 QS_CALL qsc_getReturnValue(QsNativeCall call);
QS_API boolean QS_CALL qsc_isFailedCall(QsNativeCall call);
QS_API conststring QS_CALL qsc_getErrorMessage(QsNativeCall call);
QS_API void QS_CALL qsc_setReturn(QsNativeCall call, uint64 value);
QS_API void QS_CALL qsc_setF64Return(QsNativeCall call, float64 value);
QS_API void QS_CALL qsc_setF32Return(QsNativeCall call, float32 value);
QS_API void QS_CALL qsc_throwError(QsNativeCall call, conststring errorMessage);
QS_API boolean QS_CALL qsc_getBoolArgument(QsNativeCall call, uint32 idx);
QS_API QsScriptType QS_CALL qsc_getArgumentType(QsNativeCall call, uint32 idx);
QS_API int8 QS_CALL qsc_getI8Argument(QsNativeCall call, uint32 idx);
QS_API uint8 QS_CALL qsc_getU8Argument(QsNativeCall call, uint32 idx);
QS_API int16 QS_CALL qsc_getI16Argument(QsNativeCall call, uint32 idx);
QS_API uint16 QS_CALL qsc_getU16Argument(QsNativeCall call, uint32 idx);
QS_API int32 QS_CALL qsc_getI32Argument(QsNativeCall call, uint32 idx);
QS_API uint32 QS_CALL qsc_getU32Argument(QsNativeCall call, uint32 idx);
QS_API int64 QS_CALL qsc_getI64Argument(QsNativeCall call, uint32 idx);
QS_API uint64 QS_CALL qsc_getU64Argument(QsNativeCall call, uint32 idx);
QS_API float32 QS_CALL qsc_getF32Argument(QsNativeCall call, uint32 idx);
QS_API float64 QS_CALL qsc_getF64Argument(QsNativeCall call, uint32 idx);


QS_API uint32 QS_CALL qsbf_loadBytecodeFile(const void* buf, uint64 bufferSize, QsBytecodeFile* fileOut);
QS_API conststring QS_CALL qsbf_getResultString(uint32 result);
QS_API void QS_CALL qsbf_freeBytecodeFile(QsBytecodeFile bFile);


QS_API QsEnv QS_CALL qse_createEnv();
QS_API void QS_CALL qse_freeEnv(QsEnv env);
QS_API void QS_CALL qse_addStandardLibraries(QsEnv env);
QS_API void QS_CALL qse_addLibraryDirectory(QsEnv env, conststring dirPath);
QS_API void QS_CALL qse_registerNative(QsEnv env, conststring funcName, conststring signature, QsNativeFunction func);
QS_API void QS_CALL qse_setStatementInlining(QsEnv env, boolean state);
QS_API void QS_CALL qse_setExpressionInlining(QsEnv env, boolean state);
QS_API void QS_CALL qse_setAssertsCompiled(QsEnv env, boolean state);


QS_API QsVirtualMachine QS_CALL qsvm_createVirtualMachine(QsEnv env);
QS_API void QS_CALL qsvm_freeVirtualMachine(QsVirtualMachine vm);
QS_API void QS_CALL qsvm_loadBytecodeFile(QsVirtualMachine vm, QsBytecodeFile bFile, conststring name);
QS_API int32 QS_CALL qsvm_beginExecution(QsVirtualMachine vm, uint32 argc, cstring* argv);
QS_API uint64 QS_CALL qsvm_callFunction(QsVirtualMachine vm, QsFunction sf);
QS_API conststring QS_CALL qsvm_toString(QsVirtualMachine vm, uint64 qsValue, QsScriptType type);
QS_API boolean QS_CALL qsvm_equals(QsVirtualMachine vm, uint64 a, uint64 b, QsScriptType type);

QS_API boolean QS_CALL qsc_compileSourceFile(conststring sourceName, conststring source, QsEnv env, QsBytecodeFile* fileOut);

QS_EXPORT void QS_CALL qs_onLoadNativeModule(QsVirtualMachine vm);

#ifdef __cplusplus
}
#endif

#endif //QSNI
