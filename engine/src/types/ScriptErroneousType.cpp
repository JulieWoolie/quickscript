#include "qs/types/ScriptErroneousType.hpp"

ScriptErroneousType::ScriptErroneousType() : ScriptType(TK_UNKNOWN, 0) {

}

conststring ScriptErroneousType::getTypeName() const {
  return "NIL";
}
