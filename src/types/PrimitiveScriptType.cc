#include "PrimitiveScriptType.h"

PrimitiveScriptType::PrimitiveScriptType(uint64 stackSize, primitivekind primType, conststring name)
  : ScriptType(TK_PRIMITIVE, stackSize), m_primType(primType), m_name(name)
{

}

primitivekind PrimitiveScriptType::getPrimitiveType() const {
  return m_primType;
}

conststring PrimitiveScriptType::getTypeName() const {
  return m_name;
}
