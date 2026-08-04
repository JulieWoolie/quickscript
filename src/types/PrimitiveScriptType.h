#ifndef QUICKSCRIPT_PRIMITIVESCRIPTTYPE_H
#define QUICKSCRIPT_PRIMITIVESCRIPTTYPE_H

#include "ScriptType.h"

#define PK_NIL      0
#define PK_BOOL     1
#define PK_INT8     2
#define PK_UINT8    3
#define PK_INT16    4
#define PK_UINT16   5
#define PK_INT32    6
#define PK_UINT32   7
#define PK_INT64    8
#define PK_UINT64   9
#define PK_FLOAT32  10
#define PK_FLOAT64  11
typedef uint8 primitivekind;

class PrimitiveScriptType: public ScriptType {
  const primitivekind m_primType;
  conststring const m_name;

  public:
    PrimitiveScriptType(uint64 stackSize, primitivekind primType, conststring name);

    primitivekind getPrimitiveType() const;

    conststring getTypeName() const override;
};


#endif //QUICKSCRIPT_PRIMITIVESCRIPTTYPE_H
