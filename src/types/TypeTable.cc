#include "TypeTable.h"

#include <stdexcept>

#include "ConstTypes.h"
#include "FunctionSignature.h"
#include "ScriptArrayType.h"
#include "ScriptStructType.h"

void TypeTable::ensureIndexLookupHasSpace(const uint32 desiredSize) {
  const uint64 desiredCap = static_cast<uint64>(desiredSize) * sizeof(ScriptType**);

  if (m_indexLookupCap > desiredCap) {
    return;
  }

  uint64 newCap = m_indexLookupCap;
  if (newCap == 0) {
    newCap = 1024;
  } else {
    while (newCap < desiredCap) {
      newCap *= 2;
    }
  }

  ScriptType** newArray = static_cast<ScriptType**>(realloc(m_indexLookup, newCap));
  if (!newArray) {
    throw std::runtime_error("Failed to allocate larger type array");
  }

  m_indexLookup = newArray;
  m_indexLookupCap = newCap;
}

TypeTable::TypeTable() {
  ensureIndexLookupHasSpace(20);
  m_indexLookup[TI_VOID] = ConstTypes::VOID();
  m_indexLookup[TI_BOOL] = ConstTypes::BOOL();
  m_indexLookup[TI_INT8] = ConstTypes::INT8();
  m_indexLookup[TI_UINT8] = ConstTypes::UINT8();
  m_indexLookup[TI_INT16] = ConstTypes::INT16();
  m_indexLookup[TI_UINT16] = ConstTypes::UINT16();
  m_indexLookup[TI_INT32] = ConstTypes::INT32();
  m_indexLookup[TI_UINT32] = ConstTypes::UINT32();
  m_indexLookup[TI_INT64] = ConstTypes::INT64();
  m_indexLookup[TI_UINT64] = ConstTypes::UINT64();
  m_indexLookup[TI_FLOAT32] = ConstTypes::FLOAT32();
  m_indexLookup[TI_FLOAT64] = ConstTypes::FLOAT64();
  m_indexLookup[TI_STRING] = ConstTypes::STRING();
  m_indexLookup[TI_CLOSURE] = ConstTypes::CLOSURE();
  m_indexLookupLen = TI_CLOSURE + 1;

  m_nameLookup["void"] = ConstTypes::VOID();
  m_nameLookup["bool"] = ConstTypes::BOOL();
  m_nameLookup["int8"] = ConstTypes::INT8();
  m_nameLookup["uint8"] = ConstTypes::UINT8();
  m_nameLookup["int16"] = ConstTypes::INT16();
  m_nameLookup["uint16"] = ConstTypes::UINT16();
  m_nameLookup["int32"] = ConstTypes::INT32();
  m_nameLookup["uint32"] = ConstTypes::UINT32();
  m_nameLookup["int64"] = ConstTypes::INT64();
  m_nameLookup["uint64"] = ConstTypes::UINT64();
  m_nameLookup["float32"] = ConstTypes::FLOAT32();
  m_nameLookup["float64"] = ConstTypes::FLOAT64();
  m_nameLookup["string"] = ConstTypes::STRING();
  m_nameLookup["#closure"] = ConstTypes::CLOSURE();
}

TypeTable::~TypeTable() {
  for (uint64 i = TI_STRING + 1; i < m_indexLookupLen; i++) {
    ScriptType* type = m_indexLookup[i];

    if (type->kind() == TK_ARRAY) {
      delete static_cast<ScriptArrayType*>(type);
      continue;
    }
    if (type->kind() == TK_FUNC) {
      FunctionSignature::free(static_cast<FunctionSignature*>(type));
      continue;
    }
    if (type->kind() == TK_STRUCT) {
      ScriptStructType::free(static_cast<ScriptStructType*>(type));
      continue;
    }
  }

  free(m_indexLookup);

  m_indexLookup = nullptr;
  m_indexLookupCap = 0;
  m_indexLookupLen = 0;
}

ScriptType* TypeTable::lookupByIndex(typeindex index) const {
  if (index >= m_indexLookupLen) {
    return nullptr;
  }
  return m_indexLookup[index];
}

ScriptType* TypeTable::lookupByName(const std::string& name) const {
  if (!m_nameLookup.contains(name)) {
    return nullptr;
  }
  return m_nameLookup.at(name);
}

FunctionSignature* TypeTable::getSignature(
  ScriptType* returnType,
  bool variadic,
  uint32 pCount,
  ScriptType** paramTypes
) {
  std::string string = "";
  FunctionSignature::composeName(string, returnType, pCount, paramTypes);

  FunctionSignature* result = static_cast<FunctionSignature*>(lookupByName(string));
  if (result) {
    return result;
  }

  result = FunctionSignature::create(returnType, variadic, pCount, paramTypes);
  emplaceType(result);
  
  return result;
}

ScriptType* TypeTable::getArrayType(ScriptType* componentType) {
  std::string compName = componentType->getTypeName();
  compName.append("[]");

  ScriptType* found = lookupByName(compName);
  if (found) {
    return found;
  }

  found = new ScriptArrayType(componentType);
  emplaceType(found);

  return found;
}

typeindex TypeTable::emplaceType(ScriptType* type) {
  conststring name = type->getTypeName();
  std::string nameStr = name;

  ensureIndexLookupHasSpace(m_indexLookupLen + 1);

  m_nameLookup[nameStr] = type;

  typeindex idx = m_indexLookupLen++;
  m_indexLookup[idx] = type;

  return idx;
}

int64 TypeTable::findIndex(const ScriptType* type) const {
  for (uint32 i = 0; i < m_indexLookupLen; i++) {
    if (m_indexLookup[i] == type) {
      return i;
    }
  }
  return -1;
}

uint64 TypeTable::size() const {
  return m_indexLookupLen;
}

