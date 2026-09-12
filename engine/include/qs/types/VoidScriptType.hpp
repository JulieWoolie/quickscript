#ifndef QS_VOIDTYPE_H
#define QS_VOIDTYPE_H
#include "qs/types/ScriptType.hpp"


class VoidScriptType: public ScriptType {
  public:
    VoidScriptType();

    conststring getTypeName() const override;
};


#endif //QS_VOIDTYPE_H
