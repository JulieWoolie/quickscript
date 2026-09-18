#include "stdlib.hpp"

// export native void println(string message)
static void qs_stdlib_println_string(const QsVirtualMachine vm, const QsNativeCall call) {
  const QsScriptArray message = qsc_getArrayArgument(call, 0);
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

// export native bool isInf(float32 x)
static void qs_stdlib_isInf_float32(const QsVirtualMachine vm, const QsNativeCall call) {
  const float32 x = qsc_getF32Argument(call, 0);
  // Empty generated function stub
}

// export native bool isInf(float64 x)
static void qs_stdlib_isInf_float64(const QsVirtualMachine vm, const QsNativeCall call) {
  const float64 x = qsc_getF64Argument(call, 0);
  // Empty generated function stub
}

// export native bool isFinite(float32 x)
static void qs_stdlib_isFinite_float32(const QsVirtualMachine vm, const QsNativeCall call) {
  const float32 x = qsc_getF32Argument(call, 0);
  // Empty generated function stub
}

// export native bool isFinite(float64 x)
static void qs_stdlib_isFinite_float64(const QsVirtualMachine vm, const QsNativeCall call) {
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
  qse_registerNatives(env, ns, 63,
    "println",           "(string)",                   qs_stdlib_println_string,
    "currentTimeMillis", "()=>uint64",                 qs_stdlib_currentTimeMillis,
    "getenv",            "(string)=>string",           qs_stdlib_getenv_string,
    "setenv",            "(string,string)=>bool",      qs_stdlib_setenv_string_string,
    "setenv",            "(string,string,bool)=>bool", qs_stdlib_setenv_string_string_bool,
    "clz",               "(int8)=>int8",               qs_stdlib_clz_int8,
    "clz",               "(uint8)=>uint8",             qs_stdlib_clz_uint8,
    "clz",               "(int16)=>int16",             qs_stdlib_clz_int16,
    "clz",               "(uint16)=>uint16",           qs_stdlib_clz_uint16,
    "clz",               "(int32)=>int32",             qs_stdlib_clz_int32,
    "clz",               "(uint32)=>uint32",           qs_stdlib_clz_uint32,
    "clz",               "(int64)=>int64",             qs_stdlib_clz_int64,
    "clz",               "(uint64)=>uint64",           qs_stdlib_clz_uint64,
    "isInf",             "(float32)=>bool",            qs_stdlib_isInf_float32,
    "isInf",             "(float64)=>bool",            qs_stdlib_isInf_float64,
    "isFinite",          "(float32)=>bool",            qs_stdlib_isFinite_float32,
    "isFinite",          "(float64)=>bool",            qs_stdlib_isFinite_float64,
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