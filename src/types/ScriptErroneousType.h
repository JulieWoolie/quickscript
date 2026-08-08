#ifndef QUICKSCRIPT_SCRIPTERRONEOUSTYPE_H
#define QUICKSCRIPT_SCRIPTERRONEOUSTYPE_H
#include "ScriptType.h"


class ScriptErroneousType: public ScriptType {
  public:
    ScriptErroneousType();

    conststring getTypeName() const override;
};


#endif //QUICKSCRIPT_SCRIPTERRONEOUSTYPE_H
