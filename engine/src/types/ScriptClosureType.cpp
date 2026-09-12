#include "qs/types/ScriptClosureType.hpp"

ScriptClosureType::ScriptClosureType()
  : ScriptType(TK_CLOSURE, POINTER_SIZE)
{

}

conststring ScriptClosureType::getTypeName() const {
  return "#closure";
}
