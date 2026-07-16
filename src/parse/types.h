#ifndef QUICKSCRIPT_TYPES_H
#define QUICKSCRIPT_TYPES_H

#include <vector>

#include "../common.h"

#define TK_NIL 0
#define TK_INT 1
#define TK_FLOAT 2
#define TK_STRUCT 3
#define TK_ARRAY 4

typedef uint8 typekind;

struct ScriptType {

  virtual conststring typeName() = 0;
  virtual uint32 bytes() = 0;
  virtual typekind kind() = 0;
  virtual ~ScriptType() = default;
};

#define CREATE_TYPE_EXPR(name, tname, bytec, tk) struct name: ScriptType {\
  conststring typeName() override { return tname; }\
  uint32 bytes() override { return bytec; }\
  typekind kind() override { return tk; }\
  };

CREATE_TYPE_EXPR(Int8Type, "int8", 1, TK_INT)
CREATE_TYPE_EXPR(Uint8Type, "uint8", 1, TK_INT)

CREATE_TYPE_EXPR(Int16Type, "int16", 2, TK_INT)
CREATE_TYPE_EXPR(Uint16Type, "uint16", 2, TK_INT)

CREATE_TYPE_EXPR(Int32Type, "int32", 4, TK_INT)
CREATE_TYPE_EXPR(Uint32Type, "uint32", 4, TK_INT)

CREATE_TYPE_EXPR(Int64Type, "int64", 8, TK_INT)
CREATE_TYPE_EXPR(Uint64Type, "uint64", 8, TK_INT)

CREATE_TYPE_EXPR(Float32Type, "float32", 4, TK_FLOAT)
CREATE_TYPE_EXPR(Float64Type, "float64", 8, TK_FLOAT)

struct BoolType: ScriptType {
  conststring typeName() override { return "bool"; }
  uint32 bytes() override { return 1; }
};

struct StructProperty {
  cstring propertyName = nullptr;
  ScriptType* type = nullptr;
};

struct ScriptStructType: ScriptType {
  std::vector<StructProperty*> properties;
  cstring structName;

  conststring typeName() override {
    return structName;
  }

  uint32 bytes() override {
    uint32 res = 0;
    for (auto & prop : properties) {
      res += prop->type->bytes();
    }
    return res;
  }
};

#endif //QUICKSCRIPT_TYPES_H
