#ifndef QUICKSCRIPT_SCRIPTSTRUCTTYPE_H
#define QUICKSCRIPT_SCRIPTSTRUCTTYPE_H

#include <string>
#include "ScriptType.h"

struct StructProperty {
  std::string name;
  ScriptType* type;
};

class ScriptStructType: public ScriptType {
  const std::string m_name;
  const uint32 m_propertyCount;
  StructProperty* const m_properties;

  public:
    ScriptStructType(std::string name, StructProperty* properties, uint32 propCount);

    static ScriptStructType* create(std::string name, const StructProperty* properties, uint32 propCount);

    static void free(ScriptStructType* type);

    uint32 getPropertyCount() const;
    StructProperty* getProperty(uint32 idx) const;

    uint32 typeFlags() const override;
    ScriptType* getPropertyType(std::string_view propertyName) const override;
    conststring getTypeName() const override;

    uint64 getHeapSize() const;
};


#endif //QUICKSCRIPT_SCRIPTSTRUCTTYPE_H
