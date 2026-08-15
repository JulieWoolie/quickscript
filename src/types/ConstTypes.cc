#include "ConstTypes.h"

#define CONST_TYPE_GETTER(type, constName, mname) type* ConstTypes::mname() { return &constName; }

static PrimitiveScriptType TC_BOOL = PrimitiveScriptType(1, PK_BOOL, "bool");

static PrimitiveScriptType TC_INT8 = PrimitiveScriptType(1, PK_INT8, "int8");
static PrimitiveScriptType TC_UINT8 = PrimitiveScriptType(1, PK_UINT8, "uint8");
static PrimitiveScriptType TC_INT16 = PrimitiveScriptType(2, PK_INT16, "int16");
static PrimitiveScriptType TC_UINT16 = PrimitiveScriptType(2, PK_UINT16, "uint16");
static PrimitiveScriptType TC_INT32 = PrimitiveScriptType(4, PK_INT32, "int32");
static PrimitiveScriptType TC_UINT32 = PrimitiveScriptType(4, PK_UINT32, "uint32");
static PrimitiveScriptType TC_INT64 = PrimitiveScriptType(8, PK_INT64, "int64");
static PrimitiveScriptType TC_UINT64 = PrimitiveScriptType(8, PK_UINT64, "uint64");

static PrimitiveScriptType TC_FLOAT32 = PrimitiveScriptType(4, PK_FLOAT32, "float32");
static PrimitiveScriptType TC_FLOAT64 = PrimitiveScriptType(8, PK_FLOAT64, "float64");

static VoidScriptType TC_VOID = VoidScriptType();

static ScriptStringType TC_STRING = ScriptStringType();

static ScriptErroneousType TC_ERROR = ScriptErroneousType();

static ScriptClosureType TC_CLOSURE = ScriptClosureType();

CONST_TYPE_GETTER(PrimitiveScriptType, TC_BOOL, BOOL)

CONST_TYPE_GETTER(PrimitiveScriptType, TC_INT8, INT8)
CONST_TYPE_GETTER(PrimitiveScriptType, TC_UINT8, UINT8)
CONST_TYPE_GETTER(PrimitiveScriptType, TC_INT16, INT16)
CONST_TYPE_GETTER(PrimitiveScriptType, TC_UINT16, UINT16)
CONST_TYPE_GETTER(PrimitiveScriptType, TC_INT32, INT32)
CONST_TYPE_GETTER(PrimitiveScriptType, TC_UINT32, UINT32)
CONST_TYPE_GETTER(PrimitiveScriptType, TC_INT64, INT64)
CONST_TYPE_GETTER(PrimitiveScriptType, TC_UINT64, UINT64)

CONST_TYPE_GETTER(PrimitiveScriptType, TC_FLOAT32, FLOAT32)
CONST_TYPE_GETTER(PrimitiveScriptType, TC_FLOAT64, FLOAT64)

CONST_TYPE_GETTER(VoidScriptType, TC_VOID, VOID)

CONST_TYPE_GETTER(ScriptStringType, TC_STRING, STRING)

CONST_TYPE_GETTER(ScriptErroneousType, TC_ERROR, UNKNOWN)

CONST_TYPE_GETTER(ScriptClosureType, TC_CLOSURE, CLOSURE)

PrimitiveScriptType* ConstTypes::getPrimitiveType(primitivekind kind) {
  switch (kind) {
    case PK_BOOL:     return &TC_BOOL;
    case PK_INT8:     return &TC_INT8;
    case PK_UINT8:    return &TC_UINT8;
    case PK_INT16:    return &TC_INT16;
    case PK_UINT16:   return &TC_UINT16;
    case PK_INT32:    return &TC_INT32;
    case PK_UINT32:   return &TC_UINT32;
    case PK_INT64:    return &TC_INT64;
    case PK_UINT64:   return &TC_UINT64;
    case PK_FLOAT32:  return &TC_FLOAT32;
    case PK_FLOAT64:  return &TC_FLOAT64;
    default: return nullptr;
  }
}
