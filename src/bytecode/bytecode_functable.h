#ifndef QUICKSCRIPT_BYTECODE_FUNCTABLE_H
#define QUICKSCRIPT_BYTECODE_FUNCTABLE_H

#include "../common.h"
#include "../types/TypeTable.h"

struct FunctionTableEntry {
  uint64 nameOffset = 0;
  typeindex signatureIndex = 0;
  uint64 startingInstruction = 0;
  uint64 stackSize = 0;
};

FunctionTableEntry* createFunctionTableArray(uint32 entries);

void freeFunctionTableArray(FunctionTableEntry* arr);

#endif //QUICKSCRIPT_BYTECODE_FUNCTABLE_H
