#ifndef QS_SCRIPTCLOSURETYPE_H
#define QS_SCRIPTCLOSURETYPE_H
#include "qs/types/ScriptType.hpp"


class ScriptClosureType: public ScriptType {
  public:
    ScriptClosureType();

    conststring getTypeName() const override;
};


#endif //QS_SCRIPTCLOSURETYPE_H
