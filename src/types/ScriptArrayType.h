#ifndef QUICKSCRIPT_SCRIPTARRAYTYPE_H
#define QUICKSCRIPT_SCRIPTARRAYTYPE_H
#include "ScriptType.h"


class ScriptArrayType: public ScriptType {
  ScriptType* const m_componentType;

  public:
    explicit ScriptArrayType(ScriptType* componentType);

    ScriptType* getComponentType() const;

    uint32 typeFlags() const override;

    ScriptType* getIndexReturnType() const override;

    ScriptType* getPropertyType(std::string_view propertyName) const override;
};


#endif //QUICKSCRIPT_SCRIPTARRAYTYPE_H
