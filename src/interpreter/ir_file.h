#ifndef QUICKSCRIPT_IR_FILE_H
#define QUICKSCRIPT_IR_FILE_H

#include "../common.h"
#include "../types/types.h"

#define FET_NIL 0
#define FET_LOCAL 1
#define FET_FOREIGN 2

typedef uint8 FunctionEntryType;

struct FunctionTableEntry {
  uint64 nameOffset = 0;
  uint32 signatureIndex = 0;
  FunctionEntryType type;

  uint64 startingInstruction = 0;
  void* funcAddr = nullptr;
};

struct BytecodeFile {
  uint8* constStringPool = nullptr;
  uint64 stringPoolSize = 0;

  ScriptType* typeTable = nullptr;
  uint64 typeTableSize = 0;

  FunctionTableEntry* funcTable = nullptr;
  uint32 funcTableEntries = 0;

  uint8* instructionBuf = nullptr;
  uint64 instructionsSize = 0;
};



#endif //QUICKSCRIPT_IR_FILE_H
