#ifndef QUICKSCRIPT_STRINGSCRIPTTYPE_H
#define QUICKSCRIPT_STRINGSCRIPTTYPE_H
#include "ScriptType.h"


class StringScriptType: public ScriptType {
  public:
    StringScriptType();

    uint32 typeFlags() const override;
    ScriptType* getIndexReturnType() const override;
    ScriptType* getPropertyType(std::string_view propertyName) const override;
};


#endif //QUICKSCRIPT_STRINGSCRIPTTYPE_H
