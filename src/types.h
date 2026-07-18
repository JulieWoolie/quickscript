#ifndef QUICKSCRIPT_TYPES_H
#define QUICKSCRIPT_TYPES_H

#include <unordered_map>
#include <vector>

#include "allocator.h"
#include "common.h"
#include "parse/syntaxtree.h"

#define POINTERSIZE 8

#define TK_NIL        0
#define TK_PRIMITIVE  1
#define TK_STRING     2
#define TK_STRUCT     3
#define TK_ARRAY      4
#define TK_FUNC       5
#define TK_VOID       6

typedef uint8 typekind;

#define PK_NIL      0
#define PK_BOOL     (PK_NIL+1)
#define PK_INT8     (PK_BOOL+1)
#define PK_UINT8    (PK_INT8+1)
#define PK_INT16    (PK_UINT8+1)
#define PK_UINT16   (PK_INT16+1)
#define PK_INT32    (PK_UINT16+1)
#define PK_UINT32   (PK_INT32+1)
#define PK_INT64    (PK_UINT32+1)
#define PK_UINT64   (PK_INT64+1)
#define PK_FLOAT32  (PK_UINT64+1)
#define PK_FLOAT64  (PK_FLOAT32+1)
typedef uint8 primitivekind;

struct ScriptType {

  virtual conststring typeName() = 0;
  virtual uint32 stackSizeBytes() = 0;
  virtual typekind kind() = 0;
  virtual ~ScriptType() = default;
};

struct PrimitiveScriptType: ScriptType {
  primitivekind primtype = PK_NIL;
  uint32 stackSize = 1;
  conststring name = nullptr;

  conststring typeName() override {
    return name;
  }

  uint32 stackSizeBytes() override {
    return stackSize;
  }

  typekind kind() override {
    return TK_PRIMITIVE;
  }
};

struct ScriptArrayType: ScriptType {
  ScriptType* componentType = nullptr;
  std::string name;

  conststring typeName() override {
    return name.c_str();
  }

  uint32 stackSizeBytes() override {
    return POINTERSIZE;
  }

  typekind kind() override {
    return TK_ARRAY;
  }
};

struct StructProperty {
  std::string propertyName = nullptr;
  ScriptType* type = nullptr;
};

struct ScriptStructType: ScriptType {
  std::vector<StructProperty> properties;
  std::string structName;

  conststring typeName() override {
    return structName.c_str();
  }

  uint32 stackSizeBytes() override {
    return POINTERSIZE;
  }

  typekind kind() override {
    return TK_STRUCT;
  }
};

struct ScriptStringType: ScriptType {
  conststring typeName() override {
    return "string";
  }

  uint32 stackSizeBytes() override {
    return POINTERSIZE;
  }

  typekind kind() override {
    return TK_STRING;
  }
};

struct ScriptVoidType: ScriptType {
  conststring typeName() override {
    return "void";
  }
  uint32 stackSizeBytes() override {
    return 0;
  }
  typekind kind() override {
    return TK_VOID;
  }
};

class TypeLookup {
  NoFreeAllocator* m_alloc = nullptr;
  std::unordered_map<std::string, ScriptType*> m_typeLookup;

  PrimitiveScriptType typeBool;
  PrimitiveScriptType typeInt8;
  PrimitiveScriptType typeUint8;
  PrimitiveScriptType typeInt16;
  PrimitiveScriptType typeUint16;
  PrimitiveScriptType typeInt32;
  PrimitiveScriptType typeUint32;
  PrimitiveScriptType typeInt64;
  PrimitiveScriptType typeUint64;
  PrimitiveScriptType typeFloat32;
  PrimitiveScriptType typeFloat64;
  ScriptStringType stringType;

  public:
    explicit TypeLookup(NoFreeAllocator *alloc);

    ScriptStructType* createStructType(std::string str);

    PrimitiveScriptType* getPrimitiveType(primitivekind pk);

    ScriptType* findReferencedType(std::string str);

    ScriptArrayType* getArrayType(ScriptType* componentType);

    ScriptStringType* getStringType();
};

#endif //QUICKSCRIPT_TYPES_H
