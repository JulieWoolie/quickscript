#ifndef QUICKSCRIPT_CONSTTYPES_H
#define QUICKSCRIPT_CONSTTYPES_H

#include "PrimitiveScriptType.h"
#include "ScriptErroneousType.h"
#include "ScriptStringType.h"
#include "VoidScriptType.h"

class ConstTypes {
  public:
    static ScriptErroneousType* UNKNOWN();

    static PrimitiveScriptType* BOOL();

    static PrimitiveScriptType* INT8();
    static PrimitiveScriptType* UINT8();
    static PrimitiveScriptType* INT16();
    static PrimitiveScriptType* UINT16();
    static PrimitiveScriptType* INT32();
    static PrimitiveScriptType* UINT32();
    static PrimitiveScriptType* INT64();
    static PrimitiveScriptType* UINT64();

    static PrimitiveScriptType* FLOAT32();
    static PrimitiveScriptType* FLOAT64();

    static VoidScriptType* VOID();

    static ScriptStringType* STRING();

    static PrimitiveScriptType* getPrimitiveType(primitivekind kind);
};


#endif //QUICKSCRIPT_CONSTTYPES_H
