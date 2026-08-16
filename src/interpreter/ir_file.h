#ifndef QUICKSCRIPT_IR_FILE_H
#define QUICKSCRIPT_IR_FILE_H

#include "../common.h"
#include "../types/types.h"

#define FILE_PREFIX "quickscript"
#define PREFIX_LEN 11
#define START_SIZE_PAIR_SIZE 16
#define HEADER_VERSION_SIZE 2
#define HEADER_SECTIONS 4
#define HEADER_LEN (PREFIX_LEN + HEADER_VERSION_SIZE + (START_SIZE_PAIR_SIZE * HEADER_SECTIONS))

#define HSECT_STRPOOL_OFF 0
#define HSECT_STRPOOL_SIZE 1
#define HSECT_TYPES_OFF 2
#define HSECT_TYPES_SIZE 3
#define HSECT_FTABLE_OFF 4
#define HSECT_FTABLE_SIZE 6
#define HSECT_INSTR_OFF 4
#define HSECT_INSTR_SIZE 6

#define TYPE_TABLE_ARRAY 0x0
#define TYPE_TABLE_STRUCT 0x1
#define TYPE_TABLE_SIGNATURE 0x3
typedef uint8 TypeTableType;

#define CURRENT_FILE_VERSION 0

struct FunctionTableEntry {
  uint64 nameOffset = 0;
  typeindex signatureIndex = 0;
  uint64 startingInstruction = 0;
};

struct TypeTableEntry {
  TypeTableType type = 0;
  typeindex index = 0;
};

struct TypeTableArray: TypeTableEntry {
  typeindex componentType = 0;
};

struct TypeTableFuncSign: TypeTableEntry {
  typeindex returnType = 0;
  bool varargs = false;
  typeindex* argTypes = nullptr;
  uint32 argumentCount = 0;
};

struct TypeTableStructProperty {
  uint64 nameOffset = 0;
  uint64 valueOffset = 0;
  typeindex type = 0;
};

struct TypeTableStruct: TypeTableEntry {
  uint64 nameOffset = 0;
  uint32 constructorFuncIndex = 0;
  uint32 propertyCount = 0;
  TypeTableStructProperty* properties = nullptr;
};

struct BytecodeFile {
  uint8* constStringPool = nullptr;
  uint64 stringPoolSize = 0;

  TypeTableEntry** typeTable = nullptr;
  uint64 typeTableSize = 0;

  FunctionTableEntry* funcTable = nullptr;
  uint32 funcTableEntries = 0;

  uint8* instructionBuf = nullptr;
  uint64 instructionsSize = 0;
};

uint8* serializeBytecodeFile(const BytecodeFile& file, uint64* sizeOut);

void printBytecodeFile(const BytecodeFile& file, FILE* printFile);

#endif //QUICKSCRIPT_IR_FILE_H
