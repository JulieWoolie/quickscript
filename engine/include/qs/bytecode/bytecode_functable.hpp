#ifndef QS_BYTECODE_FUNCTABLE_H
#define QS_BYTECODE_FUNCTABLE_H

#include "qs/common.hpp"
#include "qs/types/TypeTable.hpp"

#define FUNCFLAG_NATIVE 0x1
typedef uint16 functionflags;

struct FunctionTableEntry {
  uint64 nameOffset = 0;
  typeindex signatureIndex = 0;
  uint64 startingInstruction = 0;
  uint64 stackSize = 0;
  functionflags flags = 0;
};

#endif //QS_BYTECODE_FUNCTABLE_H
