#ifndef QS_BYTECODE_FUNCTABLE_H
#define QS_BYTECODE_FUNCTABLE_H

#include "qs/common.hpp"
#include "qs/types/TypeTable.hpp"

struct FunctionTableEntry {
  uint64 nameOffset = 0;
  typeindex signatureIndex = 0;
  uint64 startingInstruction = 0;
  uint64 stackSize = 0;
};

FunctionTableEntry* createFunctionTableArray(uint32 entries);

void freeFunctionTableArray(FunctionTableEntry* arr);

#endif //QS_BYTECODE_FUNCTABLE_H
