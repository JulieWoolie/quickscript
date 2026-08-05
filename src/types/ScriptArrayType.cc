#include "ScriptArrayType.h"
#include "ConstTypes.h"

ScriptArrayType::ScriptArrayType(ScriptType* componentType)
: ScriptType(TK_PRIMITIVE,POINTER_SIZE), m_componentType(componentType)
{
  m_name.append(componentType->getTypeName());
  m_name.append("[]");
}

ScriptType* ScriptArrayType::getComponentType() const {
  return m_componentType;
}

uint32 ScriptArrayType::typeFlags() const {
  return TFLAG_INDEXABLE | TFLAG_PROPERTY_HOLDER;
}

ScriptType* ScriptArrayType::getIndexReturnType() const {
  return m_componentType;
}

ScriptType* ScriptArrayType::getPropertyType(std::string_view propertyName) const {
  if (propertyName == "length") {
    return ConstTypes::UINT32();
  }
  return nullptr;
}

conststring ScriptArrayType::getTypeName() const {
  return m_name.c_str();
}

