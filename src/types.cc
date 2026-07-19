#include "types.h"

ScriptType* ScriptArrayType::getPropertyType(std::string propName, TypeLookup* lookup) {
  if (propName != "length") {
    return nullptr;
  }
  return lookup->getPrimitiveType(PK_UINT32);
}

ScriptType* ScriptStructType::getPropertyType(std::string propName, TypeLookup* lookup) {
  for (uint32 i = 0; i < propertyCount; i++) {
    StructProperty* p = properties + i;

    if (p->propertyName != propName) {
      continue;
    }

    return p->type;
  }

  return nullptr;
}

ScriptType* ScriptStructType::getIndexedType(TypeLookup* lookup) {
  return lookup->getPrimitiveType(PK_UINT8);
}

ScriptType* ScriptStringType::getPropertyType(std::string propName, TypeLookup* lookup) {
  if (propName != "length") {
    return nullptr;
  }
  return lookup->getPrimitiveType(PK_UINT32);
}

bool isNumberType(ScriptType* type) {
  if (type->kind() != TK_PRIMITIVE) {
    return false;
  }

  PrimitiveScriptType* prim = (PrimitiveScriptType*) type;

  return pkIsNumberType(prim->primtype);
}

bool pkIsSignedType(primitivekind kind) {
  switch (kind) {
    case PK_INT8:
    case PK_INT16:
    case PK_INT32:
    case PK_INT64:
      return true;
    default:
      return false;
  }
}

bool pkIsFloat(primitivekind kind) {
  return kind == PK_FLOAT32 || kind == PK_FLOAT64;
}

PrimitiveScriptType* widestNumberType(PrimitiveScriptType* l, PrimitiveScriptType* r) {
  if (l->primtype == r->primtype) {
    return l;
  }

  primitivekind lkind = l->primtype;
  primitivekind rkind = r->primtype;

  uint32 lsize = l->stackSizeBytes();
  uint32 rsize = r->stackSizeBytes();

  if (lsize > rsize) {
    return l;
  } else if (rsize > lsize) {
    return r;
  }

  bool lsigned = pkIsSignedType(lkind);
  bool rsigned = pkIsSignedType(rkind);

  if (lsigned && !rsigned) {
    return l;
  }
  if (rsigned && !lsigned) {
    return r;
  }

  if (lkind == PK_BOOL) {
    return r;
  }

  return l;
}

ScriptType* getCommonType(ScriptType* t1, ScriptType* t2) {
  if (t1 == t2) {
    return t1;
  }
  if (t1->kind() == TK_PRIMITIVE && t2->kind() == TK_PRIMITIVE) {
    PrimitiveScriptType* p1 = (PrimitiveScriptType*) t1;
    PrimitiveScriptType* p2 = (PrimitiveScriptType*) t2;
    return widestNumberType(p1, p2);
  }
  return nullptr;
}

bool isAssignableTo(ScriptType* holder, ScriptType* value) {
  if (holder == value) {
    return true;
  }
  if (holder->kind() == TK_PRIMITIVE && value->kind() == TK_PRIMITIVE) {
    PrimitiveScriptType* ph = (PrimitiveScriptType*) holder;
    PrimitiveScriptType* pv = (PrimitiveScriptType*) value;

    primitivekind hk = ph->primtype;
    primitivekind vk = pv->primtype;

    if (pkIsFloat(vk) && !pkIsFloat(hk)) {
      return false;
    }

    return holder->stackSizeBytes() > value->stackSizeBytes();
  }
  return false;
}

bool pkIsNumberType(primitivekind kind) {
  switch (kind) {
    case PK_INT8:
    case PK_UINT8:
    case PK_INT16:
    case PK_UINT16:
    case PK_INT32:
    case PK_UINT32:
    case PK_INT64:
    case PK_UINT64:
    case PK_FLOAT32:
    case PK_FLOAT64:
      return true;
    default:
      return false;
  }
}

bool isIntegerType(ScriptType* type) {
  if (type->kind() != TK_PRIMITIVE) {
    return false;
  }

  PrimitiveScriptType* prim = (PrimitiveScriptType*) type;

  return pkIsIntegerType(prim->primtype);
}

bool pkIsIntegerType(const primitivekind kind) {
  switch (kind) {
    case PK_INT8:
    case PK_UINT8:
    case PK_INT16:
    case PK_UINT16:
    case PK_INT32:
    case PK_UINT32:
    case PK_INT64:
    case PK_UINT64:
      return true;
    default:
      return false;
  }
}

TypeLookup::TypeLookup(NoFreeAllocator *alloc) {
  m_alloc = alloc;

  typeBool.stackSize = 1;
  typeBool.name = "bool";
  typeBool.primtype = PK_BOOL;

  typeInt8.stackSize = 1;
  typeInt8.primtype = PK_INT8;
  typeInt8.name = "int8";

  typeUint8.stackSize = 1;
  typeUint8.primtype = PK_UINT8;
  typeUint8.name = "uint8";

  typeInt16.stackSize = 2;
  typeInt16.primtype = PK_INT16;
  typeInt16.name = "int16";

  typeUint16.stackSize = 2;
  typeUint16.primtype = PK_UINT16;
  typeUint16.name = "uint16";

  typeInt32.stackSize = 4;
  typeInt32.primtype = PK_INT32;
  typeInt32.name = "int32";

  typeUint32.stackSize = 4;
  typeUint32.primtype = PK_UINT32;
  typeUint32.name = "uint32";

  typeInt64.stackSize = 8;
  typeInt64.primtype = PK_INT64;
  typeInt64.name = "int64";

  typeUint64.stackSize = 8;
  typeUint64.primtype = PK_UINT64;
  typeUint64.name = "uint64";

  typeFloat32.stackSize = 4;
  typeFloat32.primtype = PK_FLOAT32;
  typeFloat32.name = "float32";

  typeFloat64.stackSize = 8;
  typeFloat64.primtype = PK_FLOAT64;
  typeFloat64.name = "float64";
}

ScriptStructType* TypeLookup::createStructType(const std::string& str, uint32 properties) {
  if (m_typeLookup.contains(str)) {
    return nullptr;
  }

  ScriptStructType type;
  type.structName = str;

  StructProperty* propbuf = m_alloc->arrayAlloc<StructProperty>(properties);
  type.properties = propbuf;
  type.propertyCount = properties;

  ScriptStructType* stored = m_alloc->emplace(type);
  m_typeLookup[str] = stored;

  return stored;
}

PrimitiveScriptType* TypeLookup::getPrimitiveType(const primitivekind pk) {
  switch (pk) {
    case PK_BOOL: return &typeBool;
    case PK_UINT8: return &typeUint8;
    case PK_INT8: return &typeInt8;
    case PK_UINT16: return &typeUint16;
    case PK_INT16: return &typeInt16;
    case PK_UINT32: return &typeUint32;
    case PK_INT32: return &typeInt32;
    case PK_UINT64: return &typeUint64;
    case PK_INT64: return &typeInt64;
    case PK_FLOAT32: return &typeFloat32;
    case PK_FLOAT64: return &typeFloat64;
    default: return nullptr;
  }
}

ScriptType* TypeLookup::findReferencedType(const std::string& str) {
  if (m_typeLookup.contains(str)) {
    return m_typeLookup[str];
  }
  return nullptr;
}

ScriptArrayType* TypeLookup::getArrayType(ScriptType *componentType) {
  std::string name = std::string(componentType->typeName());
  name.append("[]");

  if (!m_typeLookup.contains(name)) {
    ScriptArrayType type;
    type.name = name;
    type.componentType = componentType;
    ScriptArrayType* emplaced = m_alloc->emplace(type);
    m_typeLookup[name] = emplaced;
    return emplaced;
  }
  return (ScriptArrayType*) m_typeLookup[name];
}

ScriptStringType* TypeLookup::getStringType() {
  return &stringType;
}

ScriptVoidType* TypeLookup::getVoidType() {
  return &typeVoid;
}

FunctionSignature* TypeLookup::emplaceFunctionType(FunctionSignature* signature) {
  if (signature->name.empty()) {
    signature->composeName();
  }

  std::string name = signature->name;
  if (m_typeLookup.contains(name)) {
    return (FunctionSignature*) m_typeLookup[name];
  }

  uint32 pcount = signature->paramCount;

  FunctionSignature* emplaced = m_alloc->make<FunctionSignature>();
  FunctionSignatureParam* params = m_alloc->arrayAlloc<FunctionSignatureParam>(pcount);

  emplaced->params = params;
  emplaced->paramCount = pcount;
  emplaced->returnType = signature->returnType;

  uint64 memsize = pcount * sizeof(FunctionSignatureParam);
  memcpy(params, signature->params, memsize);

  emplaced->composeName();

  m_typeLookup[name] = emplaced;
}
