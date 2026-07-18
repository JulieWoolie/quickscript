#include "types.h"

TypeLookup::TypeLookup(NoFreeAllocator *alloc) {
  m_alloc = alloc;

  typeVoid.stackSize = 0;
  typeVoid.name = "void";
  typeVoid.primtype = PK_VOID;

  typeBool.stackSize = 1;
  typeBool.name = "bool";
  typeBool.primtype = PK_BOOL;

  typeInt8.stackSize = 8;
  typeInt8.primtype = PK_INT8;
  typeInt8.name = "int8";

  typeUint8.stackSize = 8;
  typeUint8.primtype = PK_UINT8;
  typeUint8.name = "uint8";

  typeInt16.stackSize = 16;
  typeInt16.primtype = PK_INT16;
  typeInt16.name = "int16";

  typeUint16.stackSize = 16;
  typeUint16.primtype = PK_UINT16;
  typeUint16.name = "uint16";

  typeInt32.stackSize = 32;
  typeInt32.primtype = PK_INT32;
  typeInt32.name = "int32";

  typeUint32.stackSize = 32;
  typeUint32.primtype = PK_UINT32;
  typeUint32.name = "uint32";

  typeInt64.stackSize = 64;
  typeInt64.primtype = PK_INT64;
  typeInt64.name = "int64";

  typeUint64.stackSize = 64;
  typeUint64.primtype = PK_UINT64;
  typeUint64.name = "uint64";

  typeFloat32.stackSize = 32;
  typeFloat32.primtype = PK_FLOAT32;
  typeFloat32.name = "float32";

  typeFloat64.stackSize = 64;
  typeFloat64.primtype = PK_FLOAT64;
  typeFloat64.name = "float64";
}

ScriptStructType* TypeLookup::createStructType(const std::string str) {
  if (m_typeLookup.contains(str)) {
    return nullptr;
  }

  ScriptStructType type;
  type.structName = str;

  ScriptStructType* emplaced = m_alloc->emplace(type);
  m_typeLookup[str] = emplaced;

  return emplaced;
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
    case PK_VOID: return &typeVoid;
    default: return nullptr;
  }
}

ScriptType* TypeLookup::findReferencedType(std::string str) {
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
