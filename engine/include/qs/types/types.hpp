#ifndef QS_TYPES_H
#define QS_TYPES_H

#include "qs/types/ScriptType.hpp"
#include "qs/types/FunctionSignature.hpp"
#include "qs/types/PrimitiveScriptType.hpp"
#include "qs/types/ScriptArrayType.hpp"
#include "qs/types/ScriptStructType.hpp"
#include "qs/types/ScriptStringType.hpp"
#include "qs/types/VoidScriptType.hpp"
#include "qs/types/TypeTable.hpp"

bool isNumberType(ScriptType* type);

bool pkIsNumberType(primitivekind kind);

bool isIntegerType(ScriptType* type);

bool pkIsIntegerType(primitivekind kind);

bool isBooleanType(ScriptType* type);

bool pkIsSignedType(primitivekind kind);

PrimitiveScriptType* widestNumberType(PrimitiveScriptType* l, PrimitiveScriptType* r);

ScriptType* getCommonType(ScriptType* t1, ScriptType* t2);

bool isAssignableTo(ScriptType* holder, ScriptType* value);

#endif //QS_TYPES_H
