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

#define CURRENT_FILE_VERSION 0

#define FET_NIL 0
#define FET_LOCAL 1
#define FET_FOREIGN 2
typedef uint8 FunctionEntryType;

struct FunctionTableEntry {
  uint64 nameOffset = 0;
  typeindex signatureIndex = 0;
  uint64 startingInstruction = 0;
};

struct TypeTableEntry {
  ScriptType* type;
  typeindex index;
};

struct BytecodeFile {
  uint8* constStringPool = nullptr;
  uint64 stringPoolSize = 0;

  TypeTableEntry* typeTable = nullptr;
  uint64 typeTableSize = 0;

  FunctionTableEntry* funcTable = nullptr;
  uint32 funcTableEntries = 0;

  uint8* instructionBuf = nullptr;
  uint64 instructionsSize = 0;
};

uint8* serializeBytecodeFile(const BytecodeFile& file, uint64* sizeOut);

#endif //QUICKSCRIPT_IR_FILE_H
