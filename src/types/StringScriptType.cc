#include "StringScriptType.h"
#include "ConstTypes.h"

StringScriptType::StringScriptType() : ScriptType(TK_STRING, POINTER_SIZE) {

}

uint32 StringScriptType::typeFlags() const {
  return TFLAG_INDEXABLE | TFLAG_PROPERTY_HOLDER;
}

ScriptType* StringScriptType::getIndexReturnType() const {
  return ConstTypes::INT8();
}

ScriptType* StringScriptType::getPropertyType(std::string_view propertyName) const {
  if (propertyName == "length") {
    return ConstTypes::UINT32();
  }
  return nullptr;
}
