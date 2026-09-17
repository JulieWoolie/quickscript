#ifndef QSNI
#define QSNI

#define QS_EXPORT __declspec(dllexport)
#define QS_IMPORT __declspec(dllimport)
#define QS_CALL

#ifdef _QS_IMPL_
#define QS_API QS_EXPORT
#else
#define QS_API QS_IMPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

// ======================================
// ========= Primitive QS Types =========
// ======================================

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

// ======================================
// ============ QS Constants ============
// ======================================

#define PROPERTY_NOT_FOUND (-1)
#define CONST_REF_COUNTER 0xFFFFFFFF

// ======================================
// ============= QS Typedefs ============
// ======================================

typedef const struct ScriptType* QsScriptType;
typedef struct NativeCall* QsNativeCall;
typedef struct BytecodeFile* QsBytecodeFile;
typedef struct VirtualMachine* QsVirtualMachine;
typedef struct QsEnvironment* QsEnv;
typedef void* QsFunction;
typedef void* QsScriptArray;
typedef void* QsScriptObject;

typedef void (*QsNativeFunction)(QsVirtualMachine vm, QsNativeCall call);

// ======================================
// ========== QS API Functions ==========
// ======================================

// ========= QS Script Type Functions =========

/**
 * Get the kind of a script type.
 *
 * Will return one of the following values:
 * - TK_UNKNOWN, if a null pointer was provided or if the type is the 'ERROR' type
 * - TK_PRIMITIVE
 * - TK_STRING
 * - TK_STRUCT
 * - TK_ARRAY
 * - TK_FUNC
 * - TK_VOID
 * - TK_CLOSURE
 *
 * @param type Type Pointer
 * @return Type Kind
 */
QS_API qstypekind QS_CALL qst_getTypeKind(QsScriptType type);

/**
 * Get the size of a type.
 *
 * The returned value will be a power of 2 no larger than 8.
 *
 * @param type Type Pointer
 * @return Number of bytes needed to store the type on the script's local memory
 */
QS_API uint8 QS_CALL qst_getStackSize(QsScriptType type);

/**
 * Get the name of a type.
 * @param type Type Pointer
 * @return Type name
 */
QS_API conststring QS_CALL qst_getTypeName(QsScriptType type);

/**
 * Assumes the input is a ScriptArrayType and returns the type's component type.
 *
 * Before calling this function, ensure the specified type is a TK_ARRAY with
 * qst_getTypeKind(type)
 *
 * @param type Type Pointer
 * @return Component Type Pointer
 */
QS_API QsScriptType QS_CALL qsat_getComponentType(QsScriptType type);

/**
 * Get the type of a struct property by its index.
 *
 * Before calling this function, ensure the specified type is a TK_STRUCT with
 * qst_getTypeKind(type) == TK_STRUCT
 *
 * @param type Struct Type Pointer
 * @param propertyIndex Property Index
 *
 * @return Property type
 */
QS_API QsScriptType QS_CALL qsst_getPropertyType(QsScriptType type, uint32 propertyIndex);

/**
 * Get the name of a struct property by its index
 *
 * Before calling this function, ensure the specified type is a TK_STRUCT with
 * qst_getTypeKind(type) == TK_STRUCT
 *
 * @param type Struct Type Pointer
 * @param propertyIndex Property Index
 *
 * @return Property name
 */
QS_API conststring QS_CALL qsst_getPropertyName(QsScriptType type, uint32 propertyIndex);

/**
 * Get the memory offset of a struct property by its index
 *
 * Before calling this function, ensure the specified type is a TK_STRUCT with
 * qst_getTypeKind(type) == TK_STRUCT
 *
 * Note that the returned value is not the offset of a value from the start of a
 * struct pointer, but rather the offset of the property's value from the start
 * of a struct's data.
 *
 * All structs are prefixed with a 32bit unsigned reference counter integer which
 * the returned offset does not take into account.
 *
 * @param type Struct Type Pointer
 * @param propertyIndex Property Index
 *
 * @return Property memory offset, in bytes.
 */
QS_API uint64 QS_CALL qsst_getPropertyOffset(QsScriptType type, uint32 propertyIndex);

/**
 * Get the index of a struct property by its name
 *
 * Before calling this function, ensure the specified type is a TK_STRUCT with
 * qst_getTypeKind(type) == TK_STRUCT
 *
 * @param type Struct Type Pointer
 * @param propName Property Name
 *
 * @return Property index, or PROPERTY_NOT_FOUND, if no property
 *         with the specified name was found in the struct.
 */
QS_API int32 QS_CALL qsst_getPropertyIndex(QsScriptType type, conststring propName);

/**
 * Get the number of properties in a struct type
 *
 * Before calling this function, ensure the specified type is a TK_STRUCT with
 * qst_getTypeKind(type) == TK_STRUCT
 *
 * @param type Struct type pointer
 * @return Property count
 */
QS_API uint32 QS_CALL qsst_getPropertyCount(QsScriptType type);

/**
 * Get the size of a struct type's data.
 *
 * Before calling this function, ensure the specified type is a TK_STRUCT with
 * qst_getTypeKind(type) == TK_STRUCT
 *
 * Note that this is different from the stack size of a struct. The stack size
 * function returns the bytes needed to store a pointer to a struct, while this
 * function returns the amount of bytes needed to store the struct's data.
 *
 * This function's return value does not include the 32bit unsigned reference counter
 * integer struct data is prefixed with.
 *
 * @param type Struct type pointer
 *
 * @return Struct data size
 */
QS_API uint64 QS_CALL qsst_getSize(QsScriptType type);

/**
 * Get the memory alignment of a struct type
 *
 * Before calling this function, ensure the specified type is a TK_STRUCT with
 * qst_getTypeKind(type) == TK_STRUCT
 *
 * The return value will be a power of 2 no larger than 8.
 *
 * @param type Struct type pointer
 * @return Struct alignment
 */
QS_API uint8 QS_CALL qsst_getAlignment(QsScriptType type);

/**
 * Get a function signature's argument counter
 *
 * Before calling this function, ensure the specified type is a TK_FUNC with
 * qst_getTypeKind(type) == TK_FUNC
 *
 * @param type Function signature pointer
 * @return Argument count
 */
QS_API uint32 QS_CALL qfs_getArgumentCount(QsScriptType type);

/**
 * Get the type of a function signature's argument
 *
 * Before calling this function, ensure the specified type is a TK_FUNC with
 * qst_getTypeKind(type) == TK_FUNC
 *
 * @param type Function signature
 * @param argIndex Argument index
 *
 * @return Argument's Type pointer
 */
QS_API QsScriptType QS_CALL qfs_getArgumentType(QsScriptType type, uint32 argIndex);

/**
 * Test if a function signature is variadic, meaning the last argument is an array type
 * which accepts a variadic number of arguments.
 *
 * Before calling this function, ensure the specified type is a TK_FUNC with
 * qst_getTypeKind(type) == TK_FUNC
 *
 * @param type Function signature
 * @return Signature's variadic state, either 0 or 1
 */
QS_API boolean QS_CALL qfs_isVariadic(QsScriptType type);

/**
 * Get the return type of a function signature
 *
 * Before calling this function, ensure the specified type is a TK_FUNC with
 * qst_getTypeKind(type) == TK_FUNC
 *
 * @param type Function signature
 * @return Function signature's return type
 */
QS_API QsScriptType QS_CALL qfs_getReturnType(QsScriptType type);


// ========= QS Script Array Functions =========

/**
 * Get the length of a script array
 *
 * If the provided array pointer is a null pointer,
 * then this function will return 0
 *
 * @param array Array pointer
 * @return Script array length
 */
QS_API uint64 QS_CALL qsa_getLength(QsScriptArray array);

/**
 * Get the pointer at which an array's data starts
 *
 * If the provided array pointer is a null pointer,
 * then this function will return a null pointer
 *
 * @param array Array pointer
 * @return Array data pointer
 */
QS_API uint8* QS_CALL qsa_getData(QsScriptArray array);

/**
 * Get the pointer at which an array's data starts.
 *
 * This function is virtually identical to qsa_getData, except that it returns
 * a signed 8bit integer type pointer, instead of an unsigned 8bit integer
 * pointer.
 *
 * If the provided array pointer is a null pointer,
 * then this function will return a null pointer
 *
 * @param array Array pointer
 * @return Array data pointer
 *
 * @see qsa_getData(QsScriptArray)
 */
QS_API int8* QS_CALL qsa_getCharData(QsScriptArray array);

/**
 * Get an array's reference counter.
 *
 * If the array is a 'const' array, or null, the return value will be CONST_REF_COUNTER
 *
 * @param array Array pointer
 * @return Reference counter value
 */
QS_API uint32 QS_CALL qsa_getRefCounter(QsScriptArray array);

/**
 * Set an array's reference counter value
 *
 * Note that setting this value to 0 will not mean it will be freed, but setting it to
 * CONST_REF_COUNTER will make the array const, meaning it will never be freed
 * during regular script execution.
 *
 * @param array Array pointer
 * @param refCounter Reference counter value
 */
QS_API void QS_CALL qsa_setRefCounter(QsScriptArray array, uint32 refCounter);

/**
 * Get a signed 8bit integer value in an array
 *
 * @param array Array pointer
 * @param idx Element index
 *
 * @return signed 8bit integer value
 */
QS_API int8 QS_CALL qsa_getI8(QsScriptArray array, uint32 idx);

/**
 * Get an unsigned 8bit integer value in an array
 *
 * @param array Array pointer
 * @param idx Element index
 *
 * @return unsigned 8bit integer value
 */
QS_API uint8 QS_CALL qsa_getU8(QsScriptArray array, uint32 idx);

/**
 * Get a signed 16bit integer value in an array
 *
 * @param array Array pointer
 * @param idx Element index
 *
 * @return signed 16bit integer value
 */
QS_API int16 QS_CALL qsa_getI16(QsScriptArray array, uint32 idx);

/**
 * Get an unsigned 16bit integer value in an array
 *
 * @param array Array pointer
 * @param idx Element index
 *
 * @return unsigned 16bit integer value
 */
QS_API uint16 QS_CALL qsa_getU16(QsScriptArray array, uint32 idx);

/**
 * Get a signed 32bit integer value in an array
 *
 * @param array Array pointer
 * @param idx Element index
 *
 * @return signed 32bit integer value
 */
QS_API int32 QS_CALL qsa_getI32(QsScriptArray array, uint32 idx);

/**
 * Get an unsigned 32bit integer value in an array
 *
 * @param array Array pointer
 * @param idx Element index
 *
 * @return unsigned 32bit integer value
 */
QS_API uint32 QS_CALL qsa_getU32(QsScriptArray array, uint32 idx);

/**
 * Get a signed 64bit integer value in an array
 *
 * @param array Array pointer
 * @param idx Element index
 *
 * @return signed 64bit integer value
 */
QS_API int64 QS_CALL qsa_getI64(QsScriptArray array, uint32 idx);

/**
 * Get an unsigned 64bit integer value in an array
 *
 * @param array Array pointer
 * @param idx Element index
 *
 * @return unsigned 64bit integer value
 */
QS_API uint64 QS_CALL qsa_getU64(QsScriptArray array, uint32 idx);

/**
 * Get a 32bit floating point value in an array
 *
 * @param array Array pointer
 * @param idx Element index
 *
 * @return 32bit floating point value
 */
QS_API float32 QS_CALL qsa_getF32(QsScriptArray array, uint32 idx);

/**
 * Get a 64bit floating point value in an array
 *
 * @param array Array pointer
 * @param idx Element index
 *
 * @return 64bit floating point value
 */
QS_API float64 QS_CALL qsa_getF64(QsScriptArray array, uint32 idx);

/**
 * Get a script object pointer in an array
 *
 * @param array Array pointer
 * @param idx Element index
 *
 * @return script object pointer
 */
QS_API QsScriptObject QS_CALL qsa_getObject(QsScriptArray array, uint32 idx);

/**
 * Get a script array pointer in an array
 *
 * @param array Array pointer
 * @param idx Element index
 *
 * @return script array pointer
 */
QS_API QsScriptArray QS_CALL qsa_getArray(QsScriptArray array, uint32 idx);

/**
 * Set a signed 8bit integer value in an array
 *
 * @param array Array pointer
 * @param idx Element index
 * @param value signed 8bit integer value
 */
QS_API void QS_CALL qsa_setI8(QsScriptArray array, uint32 idx, int8 value);

/**
 * Set an unsigned 8bit integer value in an array
 *
 * @param array Array pointer
 * @param idx Element index
 * @param value unsigned 8bit integer value
 */
QS_API void QS_CALL qsa_setU8(QsScriptArray array, uint32 idx, uint8 value);

/**
 * Set a signed 16bit integer value in an array
 *
 * @param array Array pointer
 * @param idx Element index
 * @param value signed 16bit integer value
 */
QS_API void QS_CALL qsa_setI16(QsScriptArray array, uint32 idx, int16 value);

/**
 * Set an unsigned 16bit integer value in an array
 *
 * @param array Array pointer
 * @param idx Element index
 * @param value unsigned 16bit integer value
 */
QS_API void QS_CALL qsa_setU16(QsScriptArray array, uint32 idx, uint16 value);

/**
 * Set a signed 32bit integer value in an array
 *
 * @param array Array pointer
 * @param idx Element index
 * @param value signed 32bit integer value
 */
QS_API void QS_CALL qsa_setI32(QsScriptArray array, uint32 idx, int32 value);

/**
 * Set an unsigned 32bit integer value in an array
 *
 * @param array Array pointer
 * @param idx Element index
 * @param value unsigned 32bit integer value
 */
QS_API void QS_CALL qsa_setU32(QsScriptArray array, uint32 idx, uint32 value);

/**
 * Set a signed 64bit integer value in an array
 *
 * @param array Array pointer
 * @param idx Element index
 * @param value signed 64bit integer value
 */
QS_API void QS_CALL qsa_setI64(QsScriptArray array, uint32 idx, int64 value);

/**
 * Set an unsigned 64bit integer value in an array
 *
 * @param array Array pointer
 * @param idx Element index
 * @param value unsigned 64bit integer value
 */
QS_API void QS_CALL qsa_setU64(QsScriptArray array, uint32 idx, uint64 value);

/**
 * Set a 32bit floating point value in an array
 *
 * @param array Array pointer
 * @param idx Element index
 * @param value 32bit floating point value
 */
QS_API void QS_CALL qsa_setF32(QsScriptArray array, uint32 idx, float32 value);

/**
 * Set a 64bit floating point value in an array
 *
 * @param array Array pointer
 * @param idx Element index
 * @param value 64bit floating point value
 */
QS_API void QS_CALL qsa_setF64(QsScriptArray array, uint32 idx, float64 value);

/**
 * Set a script object pointer in an array
 *
 * @param array Array pointer
 * @param idx Element index
 * @param value script object pointer
 */
QS_API void QS_CALL qsa_setObject(QsScriptArray array, uint32 idx, QsScriptObject value);

/**
 * Set a script array pointer in an array
 *
 * @param array Array pointer
 * @param idx Element index
 * @param value script array pointer
 */
QS_API void QS_CALL qsa_setArray(QsScriptArray array, uint32 idx, QsScriptArray value);


// ========= QS Script Object Functions =========

/**
 * Get a struct's reference counter.
 *
 * If the struct is null, the return value will be CONST_REF_COUNTER
 *
 * @param obj Struct pointer
 *
 * @return Reference counter value
 */
QS_API uint32 QS_CALL qsobj_getRefCounter(QsScriptObject obj);

/**
 * Set a struct's reference counter value
 *
 * Note that setting this value to 0 will not mean it will be freed, but setting it to
 * CONST_REF_COUNTER will make the struct const, meaning it will never be freed
 * during regular script execution.
 *
 * @param obj Struct pointer
 * @param refCounter Reference counter value
 */
QS_API void QS_CALL qsobj_setRefCounter(QsScriptObject obj, uint32 refCounter);

/**
 * Get a signed 8bit integer value in a struct
 *
 * @param obj Struct pointer
 * @param offset Property offset
 *
 * @return signed 8bit integer value
 */
QS_API int8 QS_CALL qsobj_getI8Property(QsScriptObject obj, uint64 offset);

/**
 * Get an unsigned 8bit integer value in a struct
 *
 * @param obj Struct pointer
 * @param offset Property offset
 *
 * @return unsigned 8bit integer value
 */
QS_API uint8 QS_CALL qsobj_getU8Property(QsScriptObject obj, uint64 offset);

/**
 * Get a signed 16bit integer value in a struct
 *
 * @param obj Struct pointer
 * @param offset Property offset
 *
 * @return signed 16bit integer value
 */
QS_API int16 QS_CALL qsobj_getI16Property(QsScriptObject obj, uint64 offset);

/**
 * Get an unsigned 16bit integer value in a struct
 *
 * @param obj Struct pointer
 * @param offset Property offset
 *
 * @return unsigned 16bit integer value
 */
QS_API uint16 QS_CALL qsobj_getU16Property(QsScriptObject obj, uint64 offset);

/**
 * Get a signed 32bit integer value in a struct
 *
 * @param obj Struct pointer
 * @param offset Property offset
 *
 * @return signed 32bit integer value
 */
QS_API int32 QS_CALL qsobj_getI32Property(QsScriptObject obj, uint64 offset);

/**
 * Get an unsigned 32bit integer value in a struct
 *
 * @param obj Struct pointer
 * @param offset Property offset
 *
 * @return unsigned 32bit integer value
 */
QS_API uint32 QS_CALL qsobj_getU32Property(QsScriptObject obj, uint64 offset);

/**
 * Get a signed 64bit integer value in a struct
 *
 * @param obj Struct pointer
 * @param offset Property offset
 *
 * @return signed 64bit integer value
 */
QS_API int64 QS_CALL qsobj_getI64Property(QsScriptObject obj, uint64 offset);

/**
 * Get an unsigned 64bit integer value in a struct
 *
 * @param obj Struct pointer
 * @param offset Property offset
 *
 * @return unsigned 64bit integer value
 */
QS_API uint64 QS_CALL qsobj_getU64Property(QsScriptObject obj, uint64 offset);

/**
 * Get a 32bit floating point value in a struct
 *
 * @param obj Struct pointer
 * @param offset Property offset
 *
 * @return 32bit floating point value
 */
QS_API float32 QS_CALL qsobj_getF32Property(QsScriptObject obj, uint64 offset);

/**
 * Get a 64bit floating point value in a struct
 *
 * @param obj Struct pointer
 * @param offset Property offset
 *
 * @return 64bit floating point value
 */
QS_API float64 QS_CALL qsobj_getF64Property(QsScriptObject obj, uint64 offset);

/**
 * Get a script object pointer in a struct
 *
 * @param obj Struct pointer
 * @param offset Property offset
 *
 * @return script object pointer
 */
QS_API QsScriptObject QS_CALL qsobj_getObjectProperty(QsScriptObject obj, uint64 offset);

/**
 * Get a script array pointer in a struct
 *
 * @param obj Struct pointer
 * @param offset Property offset
 *
 * @return script array pointer
 */
QS_API QsScriptArray QS_CALL qsobj_getArrayProperty(QsScriptObject obj, uint64 offset);

/**
 * Set a signed 8bit integer value in a struct
 *
 * @param obj Struct pointer
 * @param offset Property offset
 * @param value signed 8bit integer value
 */
QS_API void QS_CALL qsobj_setI8Property(QsScriptObject obj, uint64 offset, int8 value);

/**
 * Set an unsigned 8bit integer value in a struct
 *
 * @param obj Struct pointer
 * @param offset Property offset
 * @param value unsigned 8bit integer value
 */
QS_API void QS_CALL qsobj_setU8Property(QsScriptObject obj, uint64 offset, uint8 value);

/**
 * Set a signed 16bit integer value in a struct
 *
 * @param obj Struct pointer
 * @param offset Property offset
 * @param value signed 16bit integer value
 */
QS_API void QS_CALL qsobj_setI16Property(QsScriptObject obj, uint64 offset, int16 value);

/**
 * Set an unsigned 16bit integer value in a struct
 *
 * @param obj Struct pointer
 * @param offset Property offset
 * @param value unsigned 16bit integer value
 */
QS_API void QS_CALL qsobj_setU16Property(QsScriptObject obj, uint64 offset, uint16 value);

/**
 * Set a signed 32bit integer value in a struct
 *
 * @param obj Struct pointer
 * @param offset Property offset
 * @param value signed 32bit integer value
 */
QS_API void QS_CALL qsobj_setI32Property(QsScriptObject obj, uint64 offset, int32 value);

/**
 * Set an unsigned 32bit integer value in a struct
 *
 * @param obj Struct pointer
 * @param offset Property offset
 * @param value unsigned 32bit integer value
 */
QS_API void QS_CALL qsobj_setU32Property(QsScriptObject obj, uint64 offset, uint32 value);

/**
 * Set a signed 64bit integer value in a struct
 *
 * @param obj Struct pointer
 * @param offset Property offset
 * @param value signed 64bit integer value
 */
QS_API void QS_CALL qsobj_setI64Property(QsScriptObject obj, uint64 offset, int64 value);

/**
 * Set an unsigned 64bit integer value in a struct
 *
 * @param obj Struct pointer
 * @param offset Property offset
 * @param value unsigned 64bit integer value
 */
QS_API void QS_CALL qsobj_setU64Property(QsScriptObject obj, uint64 offset, uint64 value);

/**
 * Set a 32bit floating point value in a struct
 *
 * @param obj Struct pointer
 * @param offset Property offset
 * @param value 32bit floating point value
 */
QS_API void QS_CALL qsobj_setF32Property(QsScriptObject obj, uint64 offset, float32 value);

/**
 * Set a 64bit floating point value in a struct
 *
 * @param obj Struct pointer
 * @param offset Property offset
 * @param value 64bit floating point value
 */
QS_API void QS_CALL qsobj_setF64Property(QsScriptObject obj, uint64 offset, float64 value);

/**
 * Set a script object pointer in a struct
 *
 * @param obj Struct pointer
 * @param offset Property offset
 * @param value script object pointer
 */
QS_API void QS_CALL qsobj_setObjectProperty(QsScriptObject obj, uint64 offset, QsScriptObject value);

/**
 * Set a script array pointer in a struct
 *
 * @param obj Struct pointer
 * @param offset Property offset
 * @param value script array pointer
 */
QS_API void QS_CALL qsobj_setArrayProperty(QsScriptObject obj, uint64 offset, QsScriptArray value);


// ========= QS Native Call Functions =========

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
QS_API QsScriptArray QS_CALL qsc_getArrayArgument(QsNativeCall call, uint32 idx);
QS_API QsScriptObject QS_CALL qsc_getObjectArgument(QsNativeCall call, uint32 idx);


// ========= QS Bytecode File Functions =========

QS_API uint32 QS_CALL qsbf_loadBytecodeFile(const void* buf, uint64 bufferSize, QsBytecodeFile* fileOut);
QS_API conststring QS_CALL qsbf_getResultString(uint32 result);
QS_API void QS_CALL qsbf_freeBytecodeFile(QsBytecodeFile bFile);


// ========= QS Env Functions =========

QS_API QsEnv QS_CALL qse_createEnv();
QS_API void QS_CALL qse_freeEnv(QsEnv env);
QS_API void QS_CALL qse_addStandardLibraries(QsEnv env);
QS_API void QS_CALL qse_addLibraryDirectory(QsEnv env, conststring dirPath);

QS_API void QS_CALL qse_registerNative(
  QsEnv env,
  conststring ns,
  conststring funcName,
  conststring signature,
  QsNativeFunction func
);

QS_API void QS_CALL qse_registerNatives(QsEnv env, conststring ns, uint32 n, ...);

QS_API void QS_CALL qse_setStatementInlining(QsEnv env, boolean state);
QS_API void QS_CALL qse_setExpressionInlining(QsEnv env, boolean state);
QS_API void QS_CALL qse_setAssertsCompiled(QsEnv env, boolean state);


// ========= QS Virtual Machine Functions =========

QS_API QsVirtualMachine QS_CALL qsvm_createVirtualMachine(QsEnv env);
QS_API void QS_CALL qsvm_freeVirtualMachine(QsVirtualMachine vm);
QS_API void QS_CALL qsvm_loadBytecodeFile(QsVirtualMachine vm, QsBytecodeFile bFile, conststring name);
QS_API int32 QS_CALL qsvm_beginExecution(QsVirtualMachine vm, uint32 argc, cstring* argv);
QS_API uint64 QS_CALL qsvm_callFunction(QsVirtualMachine vm, QsFunction sf);
QS_API conststring QS_CALL qsvm_toString(QsVirtualMachine vm, uint64 qsValue, QsScriptType type);
QS_API boolean QS_CALL qsvm_equals(QsVirtualMachine vm, uint64 a, uint64 b, QsScriptType type);
QS_API QsScriptObject QS_CALL qsvm_makeStruct(QsVirtualMachine vm, QsScriptType structType);
QS_API void QS_CALL qsvm_freeStruct(QsVirtualMachine vm, QsScriptObject obj);
QS_API QsScriptArray QS_CALL qsvm_makeArray(QsVirtualMachine vm, QsScriptType arrayType, uint32 capacity);
QS_API void QS_CALL qsvm_freeArray(QsVirtualMachine vm, QsScriptArray array);

QS_API boolean QS_CALL qsc_compileSourceFile(conststring sourceName, conststring source, QsEnv env, QsBytecodeFile* fileOut);


// ========= User Defined Functions =========

#ifndef _QS_IMPL
QS_EXPORT void QS_CALL qs_onLoadNativeModule(QsEnv env, conststring ns);
#endif

#ifdef __cplusplus
}
#endif

#endif //QSNI
