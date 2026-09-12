#ifndef QS_SCRIPTARRAYTYPE_H
#define QS_SCRIPTARRAYTYPE_H

#include <string>

#include "qs/types/ScriptType.hpp"


class ScriptArrayType: public ScriptType {
  ScriptType* const m_componentType;
  std::string m_name;

  public:
    explicit ScriptArrayType(ScriptType* componentType);

    ScriptType* getComponentType() const;

    uint32 typeFlags() const override;

    ScriptType* getIndexReturnType() const override;

    ScriptType* getPropertyType(std::string_view propertyName) const override;

    conststring getTypeName() const override;
};


#endif //QS_SCRIPTARRAYTYPE_H
