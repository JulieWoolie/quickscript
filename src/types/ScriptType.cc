#include "ScriptType.h"

ScriptType::ScriptType(typekind kind, uint64 stackSize)
  : m_kind(kind), m_stackSizeBytes(stackSize)
{

}

conststring ScriptType::getTypeName() const {
  return "";
}

typekind ScriptType::kind() const {
  return m_kind;
}

uint64 ScriptType::stackSizeBytes() const {
  return m_stackSizeBytes;
}

uint32 ScriptType::typeFlags() const {
  return 0;
}

ScriptType* ScriptType::getIndexReturnType() const {
  return nullptr;
}

ScriptType* ScriptType::getPropertyType(std::string_view propertyName) const {
  return nullptr;
}

