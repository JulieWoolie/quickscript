#ifndef QS_BYTECODE_TYPETABLE_H
#define QS_BYTECODE_TYPETABLE_H

#include "qs/common.hpp"
#include "qs/types/TypeTable.hpp"

#define TYPE_TABLE_ARRAY 0x0
#define TYPE_TABLE_STRUCT 0x1
#define TYPE_TABLE_SIGNATURE 0x3
typedef uint8 TypeTableType;

struct TypeTableEntry {
  typeindex index = 0;

  virtual ~TypeTableEntry() = default;
  virtual TypeTableType type() const = 0;
};

struct TypeTableArray: TypeTableEntry {
  typeindex componentType = 0;

  static TypeTableArray* create();

  static void destroy(const TypeTableArray* tt);

  TypeTableType type() const override;
};

struct TypeTableFuncSign: TypeTableEntry {
  typeindex returnType = 0;
  bool varargs = false;
  typeindex* argTypes = nullptr;
  uint32 argumentCount = 0;

  static TypeTableFuncSign* create(uint32 argCount);

  static void destroy(TypeTableFuncSign* sign);

  TypeTableType type() const override;
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

  static TypeTableStruct* create(uint32 propertyCount);

  static void destroy(TypeTableStruct* tt);

  TypeTableType type() const override;
};

void freeTypeTableEntry(TypeTableEntry* entry);

TypeTableEntry** createTypeTable(uint32 entries);

void freeTypeTable(TypeTableEntry** table);

#endif //QS_BYTECODE_TYPETABLE_H
