#include "ScriptStructType.h"

#include <utility>

ScriptStructType::ScriptStructType(std::string name, StructProperty* properties, const uint32 propCount)
  : ScriptType(TK_STRUCT,POINTER_SIZE),
    m_name(std::move(name)),
    m_propertyCount(propCount),
    m_properties(properties)
{
  m_heapSize = 0;
  for (uint32 i = 0; i < m_propertyCount; i++) {
    m_heapSize += m_properties[i].type->stackSizeBytes();
  }
}

ScriptStructType* ScriptStructType::create(
  std::string name,
  const StructProperty* properties,
  const uint32 propCount
) {
  const uint64 propArrMemSize = propCount * sizeof(StructProperty);
  constexpr uint64 typeMemSize = sizeof(ScriptStructType);
  const uint64 totalSize = propArrMemSize + typeMemSize;

  ScriptStructType* data = static_cast<ScriptStructType*>(malloc(totalSize));
  StructProperty* propArr = reinterpret_cast<StructProperty*>(data + 1);

  for (uint32 i = 0; i < propCount; i++) {
    propArr[i] = properties[i];
  }

  return new (data) ScriptStructType(std::move(name), propArr, propCount);
}

void ScriptStructType::free(ScriptStructType* type) {
  std::free(type);
}

uint32 ScriptStructType::getPropertyCount() const {
  return m_propertyCount;
}

StructProperty* ScriptStructType::getProperty(const uint32 idx) const {
  if (idx > m_propertyCount) {
    return nullptr;
  }
  return &m_properties[idx];
}

uint32 ScriptStructType::typeFlags() const {
  return TFLAG_PROPERTY_HOLDER;
}

ScriptType* ScriptStructType::getPropertyType(const std::string_view propertyName) const {
  for (uint32 idx = 0; idx < m_propertyCount; idx++) {
    StructProperty* p = &m_properties[idx];
    if (p->name != propertyName) {
      continue;
    }
    return p->type;
  }
  return nullptr;
}

conststring ScriptStructType::getTypeName() const {
  return m_name.c_str();
}

uint64 ScriptStructType::getHeapSize() const {
  return m_heapSize;
}
