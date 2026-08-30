#include "bytecode_typetable.h"

TypeTableArray* TypeTableArray::create() {
  TypeTableArray* array = new TypeTableArray();
  array->type = TYPE_TABLE_ARRAY;
  return array;
}

void TypeTableArray::destroy(const TypeTableArray* tt) {
  delete tt;
}

TypeTableFuncSign* TypeTableFuncSign::create(const uint32 argCount) {
  constexpr uint64 signSize = sizeof(TypeTableFuncSign);
  const uint64 argsSize = sizeof(typeindex) * argCount;
  const uint64 memSize = signSize + argsSize;

  TypeTableFuncSign* sign = static_cast<TypeTableFuncSign*>(malloc(memSize));
  new (sign) TypeTableFuncSign();

  sign->type = TYPE_TABLE_SIGNATURE;

  if (argCount != 0) {
    sign->argumentCount = argCount;
    sign->argTypes = reinterpret_cast<typeindex*>(sign + 1);
  }

  return sign;
}

void TypeTableFuncSign::destroy(TypeTableFuncSign* sign) {
  free(sign);
}

TypeTableStruct* TypeTableStruct::create(const uint32 propertyCount) {
  constexpr uint64 ttSize = sizeof(TypeTableStruct);
  const uint64 propsMemSize = propertyCount * sizeof(TypeTableStructProperty);
  const uint64 memSize = ttSize + propsMemSize;

  TypeTableStruct* data = static_cast<TypeTableStruct*>(malloc(memSize));
  new (data) TypeTableStruct();

  data->type = TYPE_TABLE_STRUCT;

  if (propertyCount != 0) {
    data->propertyCount = propertyCount;
    data->properties = reinterpret_cast<TypeTableStructProperty*>(data + 1);
  }

  return data;
}

void TypeTableStruct::destroy(TypeTableStruct* tt) {
  free(tt);
}

TypeTableEntry** createTypeTable(const uint32 entries) {
  if (entries == 0) {
    return nullptr;
  }
  return static_cast<TypeTableEntry**>(malloc(sizeof(TypeTableEntry*) * entries));
}

void freeTypeTable(TypeTableEntry** table) {
  free(table);
}
