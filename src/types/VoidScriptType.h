#ifndef QUICKSCRIPT_VOIDTYPE_H
#define QUICKSCRIPT_VOIDTYPE_H
#include "ScriptType.h"


class VoidScriptType: public ScriptType {
  public:
    VoidScriptType();

    conststring getTypeName() const override;
};


#endif //QUICKSCRIPT_VOIDTYPE_H
