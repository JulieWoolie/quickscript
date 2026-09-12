#ifndef QS_CONSTTYPES_H
#define QS_CONSTTYPES_H

#include "qs/types/PrimitiveScriptType.hpp"
#include "qs/types/ScriptClosureType.hpp"
#include "qs/types/ScriptErroneousType.hpp"
#include "qs/types/ScriptStringType.hpp"
#include "qs/types/VoidScriptType.hpp"

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

    static ScriptClosureType* CLOSURE();

    static PrimitiveScriptType* getPrimitiveType(primitivekind kind);
};


#endif //QS_CONSTTYPES_H
