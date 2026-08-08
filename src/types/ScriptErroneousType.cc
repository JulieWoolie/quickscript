#include "ScriptErroneousType.h"

ScriptErroneousType::ScriptErroneousType() : ScriptType(TK_UNKNOWN, 0) {

}

conststring ScriptErroneousType::getTypeName() const {
  return "NIL";
}
