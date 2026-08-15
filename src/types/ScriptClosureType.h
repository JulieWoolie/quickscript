#ifndef QUICKSCRIPT_SCRIPTCLOSURETYPE_H
#define QUICKSCRIPT_SCRIPTCLOSURETYPE_H
#include "ScriptType.h"


class ScriptClosureType: public ScriptType {
  public:
    ScriptClosureType();

    conststring getTypeName() const override;
};


#endif //QUICKSCRIPT_SCRIPTCLOSURETYPE_H
