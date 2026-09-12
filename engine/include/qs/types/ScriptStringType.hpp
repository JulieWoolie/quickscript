#ifndef QS_STRINGSCRIPTTYPE_H
#define QS_STRINGSCRIPTTYPE_H
#include "qs/types/ScriptType.hpp"


class ScriptStringType: public ScriptType {
  public:
    ScriptStringType();

    uint32 typeFlags() const override;
    ScriptType* getIndexReturnType() const override;
    ScriptType* getPropertyType(std::string_view propertyName) const override;
    conststring getTypeName() const override;
};


#endif //QS_STRINGSCRIPTTYPE_H
