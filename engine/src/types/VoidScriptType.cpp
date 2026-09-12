#include "qs/types/VoidScriptType.hpp"

VoidScriptType::VoidScriptType() : ScriptType(TK_VOID, 0) {
}
conststring VoidScriptType::getTypeName() const {
  return "void";
}
