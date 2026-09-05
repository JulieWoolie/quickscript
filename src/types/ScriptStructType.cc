#include "ScriptStructType.h"

#include <utility>

#include "../qs_math.h"

ScriptStructType::ScriptStructType(std::string name, StructProperty* properties, const uint32 propCount)
  : ScriptType(TK_STRUCT,POINTER_SIZE),
    m_name(std::move(name)),
    m_propertyCount(propCount),
    m_properties(properties)
{

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
  StructProperty* propArr;

  if (propCount != 0) {
    propArr = reinterpret_cast<StructProperty*>(data + 1);

    if (properties) {
      for (uint32 i = 0; i < propCount; i++) {
        StructProperty* dest = &propArr[i];
        new (dest) StructProperty(properties[i]);
      }
    } else {
      for (uint32 i = 0; i < propCount; i++) {
        StructProperty* dest = &propArr[i];
        new (dest) StructProperty("", nullptr);
      }
    }
  } else {
    propArr = nullptr;
  }

  return new (data) ScriptStructType(std::move(name), propArr, propCount);
}

void ScriptStructType::free(ScriptStructType* type) {
  std::free(type);
}

void ScriptStructType::setupMemoryProperties(const bool packed) {
  if (m_propertyCount == 0) {
    m_alignment = 1;
    m_heapSize = 0;
  }

  if (packed) {
    uint64 offset = 0;

    for (uint32 i = 0; i < m_propertyCount; i++) {
      m_properties[i].offset = offset;
      offset += m_properties[i].type->stackSizeBytes();
    }

    m_heapSize = offset;
    m_alignment = 1;

    return;
  }

  uint8 largestSize = 1;

  for (uint32 i = 0; i < m_propertyCount; i++) {
    const StructProperty* prop = getProperty(i);
    if (!prop || !prop->type) {
      continue;
    }

    const uint8 size = prop->type->stackSizeBytes();
    if (size > largestSize) {
      largestSize = size;
    }
  }

  uint64 offset = 0;

  for (uint32 i = 0; i < m_propertyCount; i++) {
    StructProperty* prop = getProperty(i);
    if (!prop || !prop->type) {
      continue;
    }

    const int8 size = prop->type->stackSizeBytes();
    const uint64 propOffset = NEXT_MULTIPLE_P2(offset, size);

    prop->offset = propOffset;
    offset = propOffset + size;
  }

  m_heapSize = offset + (offset % largestSize);
  m_alignment = largestSize;
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

StructProperty* ScriptStructType::getPropertyFromName(const std::string_view& propertyName) const {
  for (uint32 i = 0; i < m_propertyCount; i++) {
    StructProperty* prop = &m_properties[i];
    if (prop->name == propertyName) {
      return prop;
    }
  }
  return nullptr;
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

void ScriptStructType::setHeapSize(const uint64 heapSize) {
  m_heapSize = heapSize;
}

uint64 ScriptStructType::getAlignment() const {
  return m_alignment;
}

void ScriptStructType::setAlignment(const uint8 alignment) {
  m_alignment = alignment;
}

const std::string& ScriptStructType::getNameString() const {
  return m_name;
}
