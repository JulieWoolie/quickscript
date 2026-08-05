#ifndef QUICKSCRIPT_TYPES_H
#define QUICKSCRIPT_TYPES_H

#include "ScriptType.h"
#include "FunctionSignature.h"
#include "PrimitiveScriptType.h"
#include "ScriptArrayType.h"
#include "ScriptStructType.h"
#include "ScriptStringType.h"
#include "VoidScriptType.h"
#include "TypeTable.h"

bool isNumberType(ScriptType* type);

bool pkIsNumberType(primitivekind kind);

bool isIntegerType(ScriptType* type);

bool pkIsIntegerType(primitivekind kind);

bool isBooleanType(ScriptType* type);

bool pkIsSignedType(primitivekind kind);

PrimitiveScriptType* widestNumberType(PrimitiveScriptType* l, PrimitiveScriptType* r);

ScriptType* getCommonType(ScriptType* t1, ScriptType* t2);

bool isAssignableTo(ScriptType* holder, ScriptType* value);

#endif //QUICKSCRIPT_TYPES_H
