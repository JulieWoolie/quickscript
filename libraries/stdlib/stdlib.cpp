#include "stdlib.hpp"

// export native void printf(string format, uint64... args)
static void qs_stdlib_printf_string_uint64va(const QsVirtualMachine vm, const QsNativeCall call) {
  const QsScriptArray format = qsc_getArrayArgument(call, 0);
  const QsScriptArray args = qsc_getArrayArgument(call, 1);
  // Empty generated function stub
}

// export native void println(string message)
static void qs_stdlib_println_string(const QsVirtualMachine vm, const QsNativeCall call) {
  const QsScriptArray message = qsc_getArrayArgument(call, 0);
  // Empty generated function stub
}

// export native string sformat(string format, uint64... args)
static void qs_stdlib_sformat_string_uint64va(const QsVirtualMachine vm, const QsNativeCall call) {
  const QsScriptArray format = qsc_getArrayArgument(call, 0);
  const QsScriptArray args = qsc_getArrayArgument(call, 1);
  // Empty generated function stub
}

// export native uint64 currentTimeMillis()
static void qs_stdlib_currentTimeMillis(const QsVirtualMachine vm, const QsNativeCall call) {
  // Empty generated function stub
}

// export native string getenv(string name)
static void qs_stdlib_getenv_string(const QsVirtualMachine vm, const QsNativeCall call) {
  const QsScriptArray name = qsc_getArrayArgument(call, 0);
  // Empty generated function stub
}

// export native bool setenv(string name, string value)
static void qs_stdlib_setenv_string_string(const QsVirtualMachine vm, const QsNativeCall call) {
  const QsScriptArray name = qsc_getArrayArgument(call, 0);
  const QsScriptArray value = qsc_getArrayArgument(call, 1);
  // Empty generated function stub
}

// export native bool setenv(string name, string value, bool overwrite)
static void qs_stdlib_setenv_string_string_bool(const QsVirtualMachine vm, const QsNativeCall call) {
  const QsScriptArray name = qsc_getArrayArgument(call, 0);
  const QsScriptArray value = qsc_getArrayArgument(call, 1);
  const boolean overwrite = qsc_getBoolArgument(call, 2);
  // Empty generated function stub
}

// export native int8 abs(int8 x)
static void qs_stdlib_abs_int8(const QsVirtualMachine vm, const QsNativeCall call) {
  const int8 x = qsc_getI8Argument(call, 0);
  // Empty generated function stub
}

// export native int16 abs(int16 x)
static void qs_stdlib_abs_int16(const QsVirtualMachine vm, const QsNativeCall call) {
  const int16 x = qsc_getI16Argument(call, 0);
  // Empty generated function stub
}

// export native int32 abs(int32 x)
static void qs_stdlib_abs_int32(const QsVirtualMachine vm, const QsNativeCall call) {
  const int32 x = qsc_getI32Argument(call, 0);
  // Empty generated function stub
}

// export native int64 abs(int64 x)
static void qs_stdlib_abs_int64(const QsVirtualMachine vm, const QsNativeCall call) {
  const int64 x = qsc_getI64Argument(call, 0);
  // Empty generated function stub
}

// export native float32 abs(float32 x)
static void qs_stdlib_abs_float32(const QsVirtualMachine vm, const QsNativeCall call) {
  const float32 x = qsc_getF32Argument(call, 0);
  // Empty generated function stub
}

// export native float64 abs(float64 x)
static void qs_stdlib_abs_float64(const QsVirtualMachine vm, const QsNativeCall call) {
  const float64 x = qsc_getF64Argument(call, 0);
  // Empty generated function stub
}

// export native int8 min(int8... values)
static void qs_stdlib_min_int8va(const QsVirtualMachine vm, const QsNativeCall call) {
  const QsScriptArray values = qsc_getArrayArgument(call, 0);
  // Empty generated function stub
}

// export native uint8 min(uint8... values)
static void qs_stdlib_min_uint8va(const QsVirtualMachine vm, const QsNativeCall call) {
  const QsScriptArray values = qsc_getArrayArgument(call, 0);
  // Empty generated function stub
}

// export native int16 min(int16... values)
static void qs_stdlib_min_int16va(const QsVirtualMachine vm, const QsNativeCall call) {
  const QsScriptArray values = qsc_getArrayArgument(call, 0);
  // Empty generated function stub
}

// export native uint16 min(uint16... values)
static void qs_stdlib_min_uint16va(const QsVirtualMachine vm, const QsNativeCall call) {
  const QsScriptArray values = qsc_getArrayArgument(call, 0);
  // Empty generated function stub
}

// export native int32 min(int32... values)
static void qs_stdlib_min_int32va(const QsVirtualMachine vm, const QsNativeCall call) {
  const QsScriptArray values = qsc_getArrayArgument(call, 0);
  // Empty generated function stub
}

// export native uint32 min(uint32... values)
static void qs_stdlib_min_uint32va(const QsVirtualMachine vm, const QsNativeCall call) {
  const QsScriptArray values = qsc_getArrayArgument(call, 0);
  // Empty generated function stub
}

// export native int64 min(int64... values)
static void qs_stdlib_min_int64va(const QsVirtualMachine vm, const QsNativeCall call) {
  const QsScriptArray values = qsc_getArrayArgument(call, 0);
  // Empty generated function stub
}

// export native uint64 min(uint64... values)
static void qs_stdlib_min_uint64va(const QsVirtualMachine vm, const QsNativeCall call) {
  const QsScriptArray values = qsc_getArrayArgument(call, 0);
  // Empty generated function stub
}

// export native float32 min(float32... values)
static void qs_stdlib_min_float32va(const QsVirtualMachine vm, const QsNativeCall call) {
  const QsScriptArray values = qsc_getArrayArgument(call, 0);
  // Empty generated function stub
}

// export native float64 min(float64... values)
static void qs_stdlib_min_float64va(const QsVirtualMachine vm, const QsNativeCall call) {
  const QsScriptArray values = qsc_getArrayArgument(call, 0);
  // Empty generated function stub
}

// export native int8 max(int8... values)
static void qs_stdlib_max_int8va(const QsVirtualMachine vm, const QsNativeCall call) {
  const QsScriptArray values = qsc_getArrayArgument(call, 0);
  // Empty generated function stub
}

// export native uint8 max(uint8... values)
static void qs_stdlib_max_uint8va(const QsVirtualMachine vm, const QsNativeCall call) {
  const QsScriptArray values = qsc_getArrayArgument(call, 0);
  // Empty generated function stub
}

// export native int16 max(int16... values)
static void qs_stdlib_max_int16va(const QsVirtualMachine vm, const QsNativeCall call) {
  const QsScriptArray values = qsc_getArrayArgument(call, 0);
  // Empty generated function stub
}

// export native uint16 max(uint16... values)
static void qs_stdlib_max_uint16va(const QsVirtualMachine vm, const QsNativeCall call) {
  const QsScriptArray values = qsc_getArrayArgument(call, 0);
  // Empty generated function stub
}

// export native int32 max(int32... values)
static void qs_stdlib_max_int32va(const QsVirtualMachine vm, const QsNativeCall call) {
  const QsScriptArray values = qsc_getArrayArgument(call, 0);
  // Empty generated function stub
}

// export native uint32 max(uint32... values)
static void qs_stdlib_max_uint32va(const QsVirtualMachine vm, const QsNativeCall call) {
  const QsScriptArray values = qsc_getArrayArgument(call, 0);
  // Empty generated function stub
}

// export native int64 max(int64... values)
static void qs_stdlib_max_int64va(const QsVirtualMachine vm, const QsNativeCall call) {
  const QsScriptArray values = qsc_getArrayArgument(call, 0);
  // Empty generated function stub
}

// export native uint64 max(uint64... values)
static void qs_stdlib_max_uint64va(const QsVirtualMachine vm, const QsNativeCall call) {
  const QsScriptArray values = qsc_getArrayArgument(call, 0);
  // Empty generated function stub
}

// export native float32 max(float32... values)
static void qs_stdlib_max_float32va(const QsVirtualMachine vm, const QsNativeCall call) {
  const QsScriptArray values = qsc_getArrayArgument(call, 0);
  // Empty generated function stub
}

// export native float64 max(float64... values)
static void qs_stdlib_max_float64va(const QsVirtualMachine vm, const QsNativeCall call) {
  const QsScriptArray values = qsc_getArrayArgument(call, 0);
  // Empty generated function stub
}

// export native int8 sign(int8 x)
static void qs_stdlib_sign_int8(const QsVirtualMachine vm, const QsNativeCall call) {
  const int8 x = qsc_getI8Argument(call, 0);
  // Empty generated function stub
}

// export native int8 sign(int16 x)
static void qs_stdlib_sign_int16(const QsVirtualMachine vm, const QsNativeCall call) {
  const int16 x = qsc_getI16Argument(call, 0);
  // Empty generated function stub
}

// export native int8 sign(int32 x)
static void qs_stdlib_sign_int32(const QsVirtualMachine vm, const QsNativeCall call) {
  const int32 x = qsc_getI32Argument(call, 0);
  // Empty generated function stub
}

// export native int8 sign(int64 x)
static void qs_stdlib_sign_int64(const QsVirtualMachine vm, const QsNativeCall call) {
  const int64 x = qsc_getI64Argument(call, 0);
  // Empty generated function stub
}

// export native int8 sign(float32 x)
static void qs_stdlib_sign_float32(const QsVirtualMachine vm, const QsNativeCall call) {
  const float32 x = qsc_getF32Argument(call, 0);
  // Empty generated function stub
}

// export native int8 sign(float64 x)
static void qs_stdlib_sign_float64(const QsVirtualMachine vm, const QsNativeCall call) {
  const float64 x = qsc_getF64Argument(call, 0);
  // Empty generated function stub
}

// export native int8 clz(int8 x)
static void qs_stdlib_clz_int8(const QsVirtualMachine vm, const QsNativeCall call) {
  const int8 x = qsc_getI8Argument(call, 0);
  // Empty generated function stub
}

// export native uint8 clz(uint8 x)
static void qs_stdlib_clz_uint8(const QsVirtualMachine vm, const QsNativeCall call) {
  const uint8 x = qsc_getU8Argument(call, 0);
  // Empty generated function stub
}

// export native int16 clz(int16 x)
static void qs_stdlib_clz_int16(const QsVirtualMachine vm, const QsNativeCall call) {
  const int16 x = qsc_getI16Argument(call, 0);
  // Empty generated function stub
}

// export native uint16 clz(uint16 x)
static void qs_stdlib_clz_uint16(const QsVirtualMachine vm, const QsNativeCall call) {
  const uint16 x = qsc_getU16Argument(call, 0);
  // Empty generated function stub
}

// export native int32 clz(int32 x)
static void qs_stdlib_clz_int32(const QsVirtualMachine vm, const QsNativeCall call) {
  const int32 x = qsc_getI32Argument(call, 0);
  // Empty generated function stub
}

// export native uint32 clz(uint32 x)
static void qs_stdlib_clz_uint32(const QsVirtualMachine vm, const QsNativeCall call) {
  const uint32 x = qsc_getU32Argument(call, 0);
  // Empty generated function stub
}

// export native int64 clz(int64 x)
static void qs_stdlib_clz_int64(const QsVirtualMachine vm, const QsNativeCall call) {
  const int64 x = qsc_getI64Argument(call, 0);
  // Empty generated function stub
}

// export native uint64 clz(uint64 x)
static void qs_stdlib_clz_uint64(const QsVirtualMachine vm, const QsNativeCall call) {
  const uint64 x = qsc_getU64Argument(call, 0);
  // Empty generated function stub
}

// export native bool isNaN(float32 x)
static void qs_stdlib_isNaN_float32(const QsVirtualMachine vm, const QsNativeCall call) {
  const float32 x = qsc_getF32Argument(call, 0);
  // Empty generated function stub
}

// export native bool isNaN(float64 x)
static void qs_stdlib_isNaN_float64(const QsVirtualMachine vm, const QsNativeCall call) {
  const float64 x = qsc_getF64Argument(call, 0);
  // Empty generated function stub
}

// export native float32 sqrt(float32 x)
static void qs_stdlib_sqrt_float32(const QsVirtualMachine vm, const QsNativeCall call) {
  const float32 x = qsc_getF32Argument(call, 0);
  // Empty generated function stub
}

// export native float64 sqrt(float64 x)
static void qs_stdlib_sqrt_float64(const QsVirtualMachine vm, const QsNativeCall call) {
  const float64 x = qsc_getF64Argument(call, 0);
  // Empty generated function stub
}

// export native float32 cbrt(float32 x)
static void qs_stdlib_cbrt_float32(const QsVirtualMachine vm, const QsNativeCall call) {
  const float32 x = qsc_getF32Argument(call, 0);
  // Empty generated function stub
}

// export native float64 cbrt(float64 x)
static void qs_stdlib_cbrt_float64(const QsVirtualMachine vm, const QsNativeCall call) {
  const float64 x = qsc_getF64Argument(call, 0);
  // Empty generated function stub
}

// export native float32 acos(float32 x)
static void qs_stdlib_acos_float32(const QsVirtualMachine vm, const QsNativeCall call) {
  const float32 x = qsc_getF32Argument(call, 0);
  // Empty generated function stub
}

// export native float64 acos(float64 x)
static void qs_stdlib_acos_float64(const QsVirtualMachine vm, const QsNativeCall call) {
  const float64 x = qsc_getF64Argument(call, 0);
  // Empty generated function stub
}

// export native float32 acosh(float32 x)
static void qs_stdlib_acosh_float32(const QsVirtualMachine vm, const QsNativeCall call) {
  const float32 x = qsc_getF32Argument(call, 0);
  // Empty generated function stub
}

// export native float64 acosh(float64 x)
static void qs_stdlib_acosh_float64(const QsVirtualMachine vm, const QsNativeCall call) {
  const float64 x = qsc_getF64Argument(call, 0);
  // Empty generated function stub
}

// export native float32 asin(float32 x)
static void qs_stdlib_asin_float32(const QsVirtualMachine vm, const QsNativeCall call) {
  const float32 x = qsc_getF32Argument(call, 0);
  // Empty generated function stub
}

// export native float64 asin(float64 x)
static void qs_stdlib_asin_float64(const QsVirtualMachine vm, const QsNativeCall call) {
  const float64 x = qsc_getF64Argument(call, 0);
  // Empty generated function stub
}

// export native float32 asinh(float32 x)
static void qs_stdlib_asinh_float32(const QsVirtualMachine vm, const QsNativeCall call) {
  const float32 x = qsc_getF32Argument(call, 0);
  // Empty generated function stub
}

// export native float64 asinh(float64 x)
static void qs_stdlib_asinh_float64(const QsVirtualMachine vm, const QsNativeCall call) {
  const float64 x = qsc_getF64Argument(call, 0);
  // Empty generated function stub
}

// export native float32 atan(float32 x)
static void qs_stdlib_atan_float32(const QsVirtualMachine vm, const QsNativeCall call) {
  const float32 x = qsc_getF32Argument(call, 0);
  // Empty generated function stub
}

// export native float64 atan(float64 x)
static void qs_stdlib_atan_float64(const QsVirtualMachine vm, const QsNativeCall call) {
  const float64 x = qsc_getF64Argument(call, 0);
  // Empty generated function stub
}

// export native float32 atan2(float32 x, float32 y)
static void qs_stdlib_atan2_float32_float32(const QsVirtualMachine vm, const QsNativeCall call) {
  const float32 x = qsc_getF32Argument(call, 0);
  const float32 y = qsc_getF32Argument(call, 1);
  // Empty generated function stub
}

// export native float64 atan2(float64 x, float64 y)
static void qs_stdlib_atan2_float64_float64(const QsVirtualMachine vm, const QsNativeCall call) {
  const float64 x = qsc_getF64Argument(call, 0);
  const float64 y = qsc_getF64Argument(call, 1);
  // Empty generated function stub
}

// export native float32 atanh(float32 x)
static void qs_stdlib_atanh_float32(const QsVirtualMachine vm, const QsNativeCall call) {
  const float32 x = qsc_getF32Argument(call, 0);
  // Empty generated function stub
}

// export native float64 atanh(float64 x)
static void qs_stdlib_atanh_float64(const QsVirtualMachine vm, const QsNativeCall call) {
  const float64 x = qsc_getF64Argument(call, 0);
  // Empty generated function stub
}

// export native float32 ceil(float32 x)
static void qs_stdlib_ceil_float32(const QsVirtualMachine vm, const QsNativeCall call) {
  const float32 x = qsc_getF32Argument(call, 0);
  // Empty generated function stub
}

// export native float64 ceil(float64 x)
static void qs_stdlib_ceil_float64(const QsVirtualMachine vm, const QsNativeCall call) {
  const float64 x = qsc_getF64Argument(call, 0);
  // Empty generated function stub
}

// export native float32 floor(float32 x)
static void qs_stdlib_floor_float32(const QsVirtualMachine vm, const QsNativeCall call) {
  const float32 x = qsc_getF32Argument(call, 0);
  // Empty generated function stub
}

// export native float64 floor(float64 x)
static void qs_stdlib_floor_float64(const QsVirtualMachine vm, const QsNativeCall call) {
  const float64 x = qsc_getF64Argument(call, 0);
  // Empty generated function stub
}

// export native float32 cos(float32 x)
static void qs_stdlib_cos_float32(const QsVirtualMachine vm, const QsNativeCall call) {
  const float32 x = qsc_getF32Argument(call, 0);
  // Empty generated function stub
}

// export native float64 cos(float64 x)
static void qs_stdlib_cos_float64(const QsVirtualMachine vm, const QsNativeCall call) {
  const float64 x = qsc_getF64Argument(call, 0);
  // Empty generated function stub
}

// export native float32 cosh(float32 x)
static void qs_stdlib_cosh_float32(const QsVirtualMachine vm, const QsNativeCall call) {
  const float32 x = qsc_getF32Argument(call, 0);
  // Empty generated function stub
}

// export native float64 cosh(float64 x)
static void qs_stdlib_cosh_float64(const QsVirtualMachine vm, const QsNativeCall call) {
  const float64 x = qsc_getF64Argument(call, 0);
  // Empty generated function stub
}

// export native float32 hypot(float32... values)
static void qs_stdlib_hypot_float32va(const QsVirtualMachine vm, const QsNativeCall call) {
  const QsScriptArray values = qsc_getArrayArgument(call, 0);
  // Empty generated function stub
}

// export native float64 hypot(float64... values)
static void qs_stdlib_hypot_float64va(const QsVirtualMachine vm, const QsNativeCall call) {
  const QsScriptArray values = qsc_getArrayArgument(call, 0);
  // Empty generated function stub
}

// export native float32 log(float32 x)
static void qs_stdlib_log_float32(const QsVirtualMachine vm, const QsNativeCall call) {
  const float32 x = qsc_getF32Argument(call, 0);
  // Empty generated function stub
}

// export native float64 log(float64 x)
static void qs_stdlib_log_float64(const QsVirtualMachine vm, const QsNativeCall call) {
  const float64 x = qsc_getF64Argument(call, 0);
  // Empty generated function stub
}

// export native float32 log10(float32 x)
static void qs_stdlib_log10_float32(const QsVirtualMachine vm, const QsNativeCall call) {
  const float32 x = qsc_getF32Argument(call, 0);
  // Empty generated function stub
}

// export native float32 log1p(float32 x)
static void qs_stdlib_log1p_float32(const QsVirtualMachine vm, const QsNativeCall call) {
  const float32 x = qsc_getF32Argument(call, 0);
  // Empty generated function stub
}

// export native float64 log10(float64 x)
static void qs_stdlib_log10_float64(const QsVirtualMachine vm, const QsNativeCall call) {
  const float64 x = qsc_getF64Argument(call, 0);
  // Empty generated function stub
}

// export native float64 log1p(float64 x)
static void qs_stdlib_log1p_float64(const QsVirtualMachine vm, const QsNativeCall call) {
  const float64 x = qsc_getF64Argument(call, 0);
  // Empty generated function stub
}

// export native float32 log2(float32 x)
static void qs_stdlib_log2_float32(const QsVirtualMachine vm, const QsNativeCall call) {
  const float32 x = qsc_getF32Argument(call, 0);
  // Empty generated function stub
}

// export native float64 log2(float64 x)
static void qs_stdlib_log2_float64(const QsVirtualMachine vm, const QsNativeCall call) {
  const float64 x = qsc_getF64Argument(call, 0);
  // Empty generated function stub
}

// export native float32 round(float32 x)
static void qs_stdlib_round_float32(const QsVirtualMachine vm, const QsNativeCall call) {
  const float32 x = qsc_getF32Argument(call, 0);
  // Empty generated function stub
}

// export native float64 round(float64 x)
static void qs_stdlib_round_float64(const QsVirtualMachine vm, const QsNativeCall call) {
  const float64 x = qsc_getF64Argument(call, 0);
  // Empty generated function stub
}

// export native float32 round(float32 x, uint32 precision)
static void qs_stdlib_round_float32_uint32(const QsVirtualMachine vm, const QsNativeCall call) {
  const float32 x = qsc_getF32Argument(call, 0);
  const uint32 precision = qsc_getU32Argument(call, 1);
  // Empty generated function stub
}

// export native float64 round(float64 x, uint32 precision)
static void qs_stdlib_round_float64_uint32(const QsVirtualMachine vm, const QsNativeCall call) {
  const float64 x = qsc_getF64Argument(call, 0);
  const uint32 precision = qsc_getU32Argument(call, 1);
  // Empty generated function stub
}

// export native float32 sin(float32 x)
static void qs_stdlib_sin_float32(const QsVirtualMachine vm, const QsNativeCall call) {
  const float32 x = qsc_getF32Argument(call, 0);
  // Empty generated function stub
}

// export native float64 sin(float64 x)
static void qs_stdlib_sin_float64(const QsVirtualMachine vm, const QsNativeCall call) {
  const float64 x = qsc_getF64Argument(call, 0);
  // Empty generated function stub
}

// export native float32 sinh(float32 x)
static void qs_stdlib_sinh_float32(const QsVirtualMachine vm, const QsNativeCall call) {
  const float32 x = qsc_getF32Argument(call, 0);
  // Empty generated function stub
}

// export native float64 sinh(float64 x)
static void qs_stdlib_sinh_float64(const QsVirtualMachine vm, const QsNativeCall call) {
  const float64 x = qsc_getF64Argument(call, 0);
  // Empty generated function stub
}

// export native float32 tan(float32 x)
static void qs_stdlib_tan_float32(const QsVirtualMachine vm, const QsNativeCall call) {
  const float32 x = qsc_getF32Argument(call, 0);
  // Empty generated function stub
}

// export native float64 tan(float64 x)
static void qs_stdlib_tan_float64(const QsVirtualMachine vm, const QsNativeCall call) {
  const float64 x = qsc_getF64Argument(call, 0);
  // Empty generated function stub
}

void qs_onLoadNativeModule(QsEnv env, conststring ns) {
  qse_registerNatives(env, ns, 95,
    "printf",            "(string,uint64...)",         qs_stdlib_printf_string_uint64va,
    "println",           "(string)",                   qs_stdlib_println_string,
    "sformat",           "(string,uint64...)=>string", qs_stdlib_sformat_string_uint64va,
    "currentTimeMillis", "()=>uint64",                 qs_stdlib_currentTimeMillis,
    "getenv",            "(string)=>string",           qs_stdlib_getenv_string,
    "setenv",            "(string,string)=>bool",      qs_stdlib_setenv_string_string,
    "setenv",            "(string,string,bool)=>bool", qs_stdlib_setenv_string_string_bool,
    "abs",               "(int8)=>int8",               qs_stdlib_abs_int8,
    "abs",               "(int16)=>int16",             qs_stdlib_abs_int16,
    "abs",               "(int32)=>int32",             qs_stdlib_abs_int32,
    "abs",               "(int64)=>int64",             qs_stdlib_abs_int64,
    "abs",               "(float32)=>float32",         qs_stdlib_abs_float32,
    "abs",               "(float64)=>float64",         qs_stdlib_abs_float64,
    "min",               "(int8...)=>int8",            qs_stdlib_min_int8va,
    "min",               "(uint8...)=>uint8",          qs_stdlib_min_uint8va,
    "min",               "(int16...)=>int16",          qs_stdlib_min_int16va,
    "min",               "(uint16...)=>uint16",        qs_stdlib_min_uint16va,
    "min",               "(int32...)=>int32",          qs_stdlib_min_int32va,
    "min",               "(uint32...)=>uint32",        qs_stdlib_min_uint32va,
    "min",               "(int64...)=>int64",          qs_stdlib_min_int64va,
    "min",               "(uint64...)=>uint64",        qs_stdlib_min_uint64va,
    "min",               "(float32...)=>float32",      qs_stdlib_min_float32va,
    "min",               "(float64...)=>float64",      qs_stdlib_min_float64va,
    "max",               "(int8...)=>int8",            qs_stdlib_max_int8va,
    "max",               "(uint8...)=>uint8",          qs_stdlib_max_uint8va,
    "max",               "(int16...)=>int16",          qs_stdlib_max_int16va,
    "max",               "(uint16...)=>uint16",        qs_stdlib_max_uint16va,
    "max",               "(int32...)=>int32",          qs_stdlib_max_int32va,
    "max",               "(uint32...)=>uint32",        qs_stdlib_max_uint32va,
    "max",               "(int64...)=>int64",          qs_stdlib_max_int64va,
    "max",               "(uint64...)=>uint64",        qs_stdlib_max_uint64va,
    "max",               "(float32...)=>float32",      qs_stdlib_max_float32va,
    "max",               "(float64...)=>float64",      qs_stdlib_max_float64va,
    "sign",              "(int8)=>int8",               qs_stdlib_sign_int8,
    "sign",              "(int16)=>int8",              qs_stdlib_sign_int16,
    "sign",              "(int32)=>int8",              qs_stdlib_sign_int32,
    "sign",              "(int64)=>int8",              qs_stdlib_sign_int64,
    "sign",              "(float32)=>int8",            qs_stdlib_sign_float32,
    "sign",              "(float64)=>int8",            qs_stdlib_sign_float64,
    "clz",               "(int8)=>int8",               qs_stdlib_clz_int8,
    "clz",               "(uint8)=>uint8",             qs_stdlib_clz_uint8,
    "clz",               "(int16)=>int16",             qs_stdlib_clz_int16,
    "clz",               "(uint16)=>uint16",           qs_stdlib_clz_uint16,
    "clz",               "(int32)=>int32",             qs_stdlib_clz_int32,
    "clz",               "(uint32)=>uint32",           qs_stdlib_clz_uint32,
    "clz",               "(int64)=>int64",             qs_stdlib_clz_int64,
    "clz",               "(uint64)=>uint64",           qs_stdlib_clz_uint64,
    "isNaN",             "(float32)=>bool",            qs_stdlib_isNaN_float32,
    "isNaN",             "(float64)=>bool",            qs_stdlib_isNaN_float64,
    "sqrt",              "(float32)=>float32",         qs_stdlib_sqrt_float32,
    "sqrt",              "(float64)=>float64",         qs_stdlib_sqrt_float64,
    "cbrt",              "(float32)=>float32",         qs_stdlib_cbrt_float32,
    "cbrt",              "(float64)=>float64",         qs_stdlib_cbrt_float64,
    "acos",              "(float32)=>float32",         qs_stdlib_acos_float32,
    "acos",              "(float64)=>float64",         qs_stdlib_acos_float64,
    "acosh",             "(float32)=>float32",         qs_stdlib_acosh_float32,
    "acosh",             "(float64)=>float64",         qs_stdlib_acosh_float64,
    "asin",              "(float32)=>float32",         qs_stdlib_asin_float32,
    "asin",              "(float64)=>float64",         qs_stdlib_asin_float64,
    "asinh",             "(float32)=>float32",         qs_stdlib_asinh_float32,
    "asinh",             "(float64)=>float64",         qs_stdlib_asinh_float64,
    "atan",              "(float32)=>float32",         qs_stdlib_atan_float32,
    "atan",              "(float64)=>float64",         qs_stdlib_atan_float64,
    "atan2",             "(float32,float32)=>float32", qs_stdlib_atan2_float32_float32,
    "atan2",             "(float64,float64)=>float64", qs_stdlib_atan2_float64_float64,
    "atanh",             "(float32)=>float32",         qs_stdlib_atanh_float32,
    "atanh",             "(float64)=>float64",         qs_stdlib_atanh_float64,
    "ceil",              "(float32)=>float32",         qs_stdlib_ceil_float32,
    "ceil",              "(float64)=>float64",         qs_stdlib_ceil_float64,
    "floor",             "(float32)=>float32",         qs_stdlib_floor_float32,
    "floor",             "(float64)=>float64",         qs_stdlib_floor_float64,
    "cos",               "(float32)=>float32",         qs_stdlib_cos_float32,
    "cos",               "(float64)=>float64",         qs_stdlib_cos_float64,
    "cosh",              "(float32)=>float32",         qs_stdlib_cosh_float32,
    "cosh",              "(float64)=>float64",         qs_stdlib_cosh_float64,
    "hypot",             "(float32...)=>float32",      qs_stdlib_hypot_float32va,
    "hypot",             "(float64...)=>float64",      qs_stdlib_hypot_float64va,
    "log",               "(float32)=>float32",         qs_stdlib_log_float32,
    "log",               "(float64)=>float64",         qs_stdlib_log_float64,
    "log10",             "(float32)=>float32",         qs_stdlib_log10_float32,
    "log1p",             "(float32)=>float32",         qs_stdlib_log1p_float32,
    "log10",             "(float64)=>float64",         qs_stdlib_log10_float64,
    "log1p",             "(float64)=>float64",         qs_stdlib_log1p_float64,
    "log2",              "(float32)=>float32",         qs_stdlib_log2_float32,
    "log2",              "(float64)=>float64",         qs_stdlib_log2_float64,
    "round",             "(float32)=>float32",         qs_stdlib_round_float32,
    "round",             "(float64)=>float64",         qs_stdlib_round_float64,
    "round",             "(float32,uint32)=>float32",  qs_stdlib_round_float32_uint32,
    "round",             "(float64,uint32)=>float64",  qs_stdlib_round_float64_uint32,
    "sin",               "(float32)=>float32",         qs_stdlib_sin_float32,
    "sin",               "(float64)=>float64",         qs_stdlib_sin_float64,
    "sinh",              "(float32)=>float32",         qs_stdlib_sinh_float32,
    "sinh",              "(float64)=>float64",         qs_stdlib_sinh_float64,
    "tan",               "(float32)=>float32",         qs_stdlib_tan_float32,
    "tan",               "(float64)=>float64",         qs_stdlib_tan_float64
  );
}