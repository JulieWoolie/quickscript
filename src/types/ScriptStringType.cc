#include "ScriptStringType.h"
#include "ConstTypes.h"

ScriptStringType::ScriptStringType() : ScriptType(TK_STRING, POINTER_SIZE) {

}

uint32 ScriptStringType::typeFlags() const {
  return TFLAG_INDEXABLE | TFLAG_PROPERTY_HOLDER;
}

ScriptType* ScriptStringType::getIndexReturnType() const {
  return ConstTypes::INT8();
}

ScriptType* ScriptStringType::getPropertyType(std::string_view propertyName) const {
  if (propertyName == "length") {
    return ConstTypes::UINT32();
  }
  return nullptr;
}

conststring ScriptStringType::getTypeName() const {
  return "string";
}
