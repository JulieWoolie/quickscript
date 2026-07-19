#ifndef QUICKSCRIPT_TYPES_H
#define QUICKSCRIPT_TYPES_H

#include <unordered_map>
#include <vector>

#include "allocator.h"
#include "common.h"

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

#define TYPEFLAG_INDEXABLE (1 << 0)
#define TYPEFLAG_PROPERTIES (2 << 0)

class TypeLookup;

struct ScriptType {

  virtual conststring typeName() = 0;
  virtual uint32 stackSizeBytes() = 0;
  virtual typekind kind() = 0;
  virtual ~ScriptType() = default;

  virtual uint32 typeFlags() {
    return 0;
  }

  virtual ScriptType* getPropertyType(std::string propName, TypeLookup* lookup) {
    return nullptr;
  }

  virtual ScriptType* getIndexedType(TypeLookup* lookup) {
    return nullptr;
  }
};

struct FunctionSignatureParam {
  ScriptType* type = nullptr;
  bool varargs = false;
};

struct FunctionSignature: ScriptType {
  ScriptType* returnType = nullptr;
  FunctionSignatureParam* params = nullptr;
  uint32 paramCount = 0;

  std::string name;

  void composeName() {
    name = "";
    name.append("(");

    if (paramCount != 0) {
      for (uint32 i = 0; i < paramCount; i++) {
        if (i != 0) {
          name.append(",");
        }
        FunctionSignatureParam* p = &params[i];
        name.append(p->type->typeName());
        if (p->varargs) {
          name.append("...");
        }
      }
    }

    name.append(")=>");
    if (returnType) {
      name.append(returnType->typeName());
    } else {
      name.append("void");
    }
  }

  conststring typeName() override {
    return name.c_str();
  }
  uint32 stackSizeBytes() override {
    return POINTERSIZE;
  }
  typekind kind() override {
    return TK_FUNC;
  }

  uint32 getMaxArgs() {
    if (paramCount == 0) {
      return 0;
    }

    FunctionSignatureParam* last = &params[paramCount - 1];
    if (last->varargs) {
      return UINT_MAX;
    }

    return paramCount;
  }
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

  uint32 typeFlags() override {
    return TYPEFLAG_INDEXABLE | TYPEFLAG_PROPERTIES;
  }

  ScriptType* getPropertyType(std::string propName, TypeLookup* lookup) override;

  ScriptType* getIndexedType(TypeLookup* lookup) override {
    return componentType;
  }
};

struct StructProperty {
  std::string propertyName = "";
  ScriptType* type = nullptr;
};

struct ScriptStructType: ScriptType {
  StructProperty* properties = nullptr;
  uint32 propertyCount = 0;
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

  uint32 typeFlags() override {
    return TYPEFLAG_PROPERTIES;
  }

  ScriptType* getPropertyType(std::string propName, TypeLookup* lookup) override;

  ScriptType* getIndexedType(TypeLookup* lookup) override;
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

  uint32 typeFlags() override {
    return TYPEFLAG_PROPERTIES | TYPEFLAG_INDEXABLE;
  }

  ScriptType* getPropertyType(std::string propName, TypeLookup* lookup) override;
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

bool isNumberType(ScriptType* type);

bool pkIsNumberType(primitivekind kind);

bool isIntegerType(ScriptType* type);

bool pkIsIntegerType(primitivekind kind);

bool pkIsSignedType(primitivekind kind);

PrimitiveScriptType* widestNumberType(PrimitiveScriptType* l, PrimitiveScriptType* r);

ScriptType* getCommonType(ScriptType* t1, ScriptType* t2);

bool isAssignableTo(ScriptType* holder, ScriptType* value);

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

  ScriptVoidType typeVoid;
  ScriptStringType stringType;

  public:
    explicit TypeLookup(NoFreeAllocator *alloc);

    ScriptStructType* createStructType(const std::string& str, uint32 properties);

    PrimitiveScriptType* getPrimitiveType(primitivekind pk);

    ScriptType* findReferencedType(const std::string& str);

    ScriptArrayType* getArrayType(ScriptType* componentType);

    ScriptStringType* getStringType();

    ScriptVoidType* getVoidType();

    FunctionSignature* emplaceFunctionType(FunctionSignature* signature);
};

#endif //QUICKSCRIPT_TYPES_H
