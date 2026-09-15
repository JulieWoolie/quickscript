#ifndef QS_BYTECODE_TYPETABLE_H
#define QS_BYTECODE_TYPETABLE_H

#include <vector>

#include "qs/common.hpp"
#include "qs/types/TypeTable.hpp"

#define TYPE_TABLE_ARRAY 0x0
#define TYPE_TABLE_STRUCT 0x1
#define TYPE_TABLE_SIGNATURE 0x3
typedef uint8 TypeTableType;

struct TypeTableEntry {
  typeindex index = 0;
  const TypeTableType type;

  explicit TypeTableEntry(TypeTableType t);
};

struct TypeTableArray: TypeTableEntry {
  typeindex componentType = 0;

  TypeTableArray();

  static TypeTableArray* create();

  static void destroy(const TypeTableArray* tt);
};

struct TypeTableFuncSign: TypeTableEntry {
  typeindex returnType = 0;
  bool varargs = false;
  typeindex* argTypes = nullptr;
  uint32 argumentCount = 0;

  TypeTableFuncSign();

  static TypeTableFuncSign* create(uint32 argCount);

  static void destroy(TypeTableFuncSign* sign);
};

struct TypeTableStructProperty {
  uint64 nameOffset = 0;
  uint64 valueOffset = 0;
  typeindex type = 0;
};

struct TypeTableStruct: TypeTableEntry {
  uint64 nameOffset = 0;
  uint64 heapSize = 0;
  uint8 alignment = 1;
  uint32 constructorFuncIndex = 0;
  uint32 propertyCount = 0;
  TypeTableStructProperty* properties = nullptr;

  TypeTableStruct();

  static TypeTableStruct* create(uint32 propertyCount);

  static void destroy(TypeTableStruct* tt);
};

void freeTypeTableEntry(TypeTableEntry* entry);

TypeTableEntry** createTypeTable(uint32 entries);

void freeTypeTable(TypeTableEntry** table);

#endif //QS_BYTECODE_TYPETABLE_H
