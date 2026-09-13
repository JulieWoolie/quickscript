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

void qs_onLoadNativeModule(QsEnv env) {
  qse_registerNative(env, "printf", "(string,uint64...)", qs_stdlib_printf_string_uint64va);
  qse_registerNative(env, "println", "(string)", qs_stdlib_println_string);
  qse_registerNative(env, "sformat", "(string,uint64...)=>string", qs_stdlib_sformat_string_uint64va);
  qse_registerNative(env, "currentTimeMillis", "()=>uint64", qs_stdlib_currentTimeMillis);
  qse_registerNative(env, "getenv", "(string)=>string", qs_stdlib_getenv_string);
  qse_registerNative(env, "setenv", "(string,string)=>bool", qs_stdlib_setenv_string_string);
  qse_registerNative(env, "setenv", "(string,string,bool)=>bool", qs_stdlib_setenv_string_string_bool);
  qse_registerNative(env, "abs", "(int8)=>int8", qs_stdlib_abs_int8);
  qse_registerNative(env, "abs", "(int16)=>int16", qs_stdlib_abs_int16);
  qse_registerNative(env, "abs", "(int32)=>int32", qs_stdlib_abs_int32);
  qse_registerNative(env, "abs", "(int64)=>int64", qs_stdlib_abs_int64);
  qse_registerNative(env, "abs", "(float32)=>float32", qs_stdlib_abs_float32);
  qse_registerNative(env, "abs", "(float64)=>float64", qs_stdlib_abs_float64);
  qse_registerNative(env, "min", "(int8...)=>int8", qs_stdlib_min_int8va);
  qse_registerNative(env, "min", "(uint8...)=>uint8", qs_stdlib_min_uint8va);
  qse_registerNative(env, "min", "(int16...)=>int16", qs_stdlib_min_int16va);
  qse_registerNative(env, "min", "(uint16...)=>uint16", qs_stdlib_min_uint16va);
  qse_registerNative(env, "min", "(int32...)=>int32", qs_stdlib_min_int32va);
  qse_registerNative(env, "min", "(uint32...)=>uint32", qs_stdlib_min_uint32va);
  qse_registerNative(env, "min", "(int64...)=>int64", qs_stdlib_min_int64va);
  qse_registerNative(env, "min", "(uint64...)=>uint64", qs_stdlib_min_uint64va);
  qse_registerNative(env, "min", "(float32...)=>float32", qs_stdlib_min_float32va);
  qse_registerNative(env, "min", "(float64...)=>float64", qs_stdlib_min_float64va);
  qse_registerNative(env, "max", "(int8...)=>int8", qs_stdlib_max_int8va);
  qse_registerNative(env, "max", "(uint8...)=>uint8", qs_stdlib_max_uint8va);
  qse_registerNative(env, "max", "(int16...)=>int16", qs_stdlib_max_int16va);
  qse_registerNative(env, "max", "(uint16...)=>uint16", qs_stdlib_max_uint16va);
  qse_registerNative(env, "max", "(int32...)=>int32", qs_stdlib_max_int32va);
  qse_registerNative(env, "max", "(uint32...)=>uint32", qs_stdlib_max_uint32va);
  qse_registerNative(env, "max", "(int64...)=>int64", qs_stdlib_max_int64va);
  qse_registerNative(env, "max", "(uint64...)=>uint64", qs_stdlib_max_uint64va);
  qse_registerNative(env, "max", "(float32...)=>float32", qs_stdlib_max_float32va);
  qse_registerNative(env, "max", "(float64...)=>float64", qs_stdlib_max_float64va);
  qse_registerNative(env, "sign", "(int8)=>int8", qs_stdlib_sign_int8);
  qse_registerNative(env, "sign", "(int16)=>int8", qs_stdlib_sign_int16);
  qse_registerNative(env, "sign", "(int32)=>int8", qs_stdlib_sign_int32);
  qse_registerNative(env, "sign", "(int64)=>int8", qs_stdlib_sign_int64);
  qse_registerNative(env, "sign", "(float32)=>int8", qs_stdlib_sign_float32);
  qse_registerNative(env, "sign", "(float64)=>int8", qs_stdlib_sign_float64);
  qse_registerNative(env, "clz", "(int8)=>int8", qs_stdlib_clz_int8);
  qse_registerNative(env, "clz", "(uint8)=>uint8", qs_stdlib_clz_uint8);
  qse_registerNative(env, "clz", "(int16)=>int16", qs_stdlib_clz_int16);
  qse_registerNative(env, "clz", "(uint16)=>uint16", qs_stdlib_clz_uint16);
  qse_registerNative(env, "clz", "(int32)=>int32", qs_stdlib_clz_int32);
  qse_registerNative(env, "clz", "(uint32)=>uint32", qs_stdlib_clz_uint32);
  qse_registerNative(env, "clz", "(int64)=>int64", qs_stdlib_clz_int64);
  qse_registerNative(env, "clz", "(uint64)=>uint64", qs_stdlib_clz_uint64);
  qse_registerNative(env, "isNaN", "(float32)=>bool", qs_stdlib_isNaN_float32);
  qse_registerNative(env, "isNaN", "(float64)=>bool", qs_stdlib_isNaN_float64);
  qse_registerNative(env, "sqrt", "(float32)=>float32", qs_stdlib_sqrt_float32);
  qse_registerNative(env, "sqrt", "(float64)=>float64", qs_stdlib_sqrt_float64);
  qse_registerNative(env, "cbrt", "(float32)=>float32", qs_stdlib_cbrt_float32);
  qse_registerNative(env, "cbrt", "(float64)=>float64", qs_stdlib_cbrt_float64);
  qse_registerNative(env, "acos", "(float32)=>float32", qs_stdlib_acos_float32);
  qse_registerNative(env, "acos", "(float64)=>float64", qs_stdlib_acos_float64);
  qse_registerNative(env, "acosh", "(float32)=>float32", qs_stdlib_acosh_float32);
  qse_registerNative(env, "acosh", "(float64)=>float64", qs_stdlib_acosh_float64);
  qse_registerNative(env, "asin", "(float32)=>float32", qs_stdlib_asin_float32);
  qse_registerNative(env, "asin", "(float64)=>float64", qs_stdlib_asin_float64);
  qse_registerNative(env, "asinh", "(float32)=>float32", qs_stdlib_asinh_float32);
  qse_registerNative(env, "asinh", "(float64)=>float64", qs_stdlib_asinh_float64);
  qse_registerNative(env, "atan", "(float32)=>float32", qs_stdlib_atan_float32);
  qse_registerNative(env, "atan", "(float64)=>float64", qs_stdlib_atan_float64);
  qse_registerNative(env, "atan2", "(float32,float32)=>float32", qs_stdlib_atan2_float32_float32);
  qse_registerNative(env, "atan2", "(float64,float64)=>float64", qs_stdlib_atan2_float64_float64);
  qse_registerNative(env, "atanh", "(float32)=>float32", qs_stdlib_atanh_float32);
  qse_registerNative(env, "atanh", "(float64)=>float64", qs_stdlib_atanh_float64);
  qse_registerNative(env, "ceil", "(float32)=>float32", qs_stdlib_ceil_float32);
  qse_registerNative(env, "ceil", "(float64)=>float64", qs_stdlib_ceil_float64);
  qse_registerNative(env, "floor", "(float32)=>float32", qs_stdlib_floor_float32);
  qse_registerNative(env, "floor", "(float64)=>float64", qs_stdlib_floor_float64);
  qse_registerNative(env, "cos", "(float32)=>float32", qs_stdlib_cos_float32);
  qse_registerNative(env, "cos", "(float64)=>float64", qs_stdlib_cos_float64);
  qse_registerNative(env, "cosh", "(float32)=>float32", qs_stdlib_cosh_float32);
  qse_registerNative(env, "cosh", "(float64)=>float64", qs_stdlib_cosh_float64);
  qse_registerNative(env, "hypot", "(float32...)=>float32", qs_stdlib_hypot_float32va);
  qse_registerNative(env, "hypot", "(float64...)=>float64", qs_stdlib_hypot_float64va);
  qse_registerNative(env, "log", "(float32)=>float32", qs_stdlib_log_float32);
  qse_registerNative(env, "log", "(float64)=>float64", qs_stdlib_log_float64);
  qse_registerNative(env, "log10", "(float32)=>float32", qs_stdlib_log10_float32);
  qse_registerNative(env, "log1p", "(float32)=>float32", qs_stdlib_log1p_float32);
  qse_registerNative(env, "log10", "(float64)=>float64", qs_stdlib_log10_float64);
  qse_registerNative(env, "log1p", "(float64)=>float64", qs_stdlib_log1p_float64);
  qse_registerNative(env, "log2", "(float32)=>float32", qs_stdlib_log2_float32);
  qse_registerNative(env, "log2", "(float64)=>float64", qs_stdlib_log2_float64);
  qse_registerNative(env, "round", "(float32)=>float32", qs_stdlib_round_float32);
  qse_registerNative(env, "round", "(float64)=>float64", qs_stdlib_round_float64);
  qse_registerNative(env, "round", "(float32,uint32)=>float32", qs_stdlib_round_float32_uint32);
  qse_registerNative(env, "round", "(float64,uint32)=>float64", qs_stdlib_round_float64_uint32);
  qse_registerNative(env, "sin", "(float32)=>float32", qs_stdlib_sin_float32);
  qse_registerNative(env, "sin", "(float64)=>float64", qs_stdlib_sin_float64);
  qse_registerNative(env, "sinh", "(float32)=>float32", qs_stdlib_sinh_float32);
  qse_registerNative(env, "sinh", "(float64)=>float64", qs_stdlib_sinh_float64);
  qse_registerNative(env, "tan", "(float32)=>float32", qs_stdlib_tan_float32);
  qse_registerNative(env, "tan", "(float64)=>float64", qs_stdlib_tan_float64);
}