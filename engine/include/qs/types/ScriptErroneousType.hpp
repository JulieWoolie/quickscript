#ifndef QS_SCRIPTERRONEOUSTYPE_H
#define QS_SCRIPTERRONEOUSTYPE_H
#include "qs/types/ScriptType.hpp"


class ScriptErroneousType: public ScriptType {
  public:
    ScriptErroneousType();

    conststring getTypeName() const override;
};


#endif //QS_SCRIPTERRONEOUSTYPE_H
