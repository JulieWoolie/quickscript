#include "bytecode_functable.h"

FunctionTableEntry* createFunctionTableArray(const uint32 entries) {
  if (entries == 0) {
    return nullptr;
  }
  return static_cast<FunctionTableEntry*>(malloc(sizeof(FunctionTableEntry) * entries));
}

void freeFunctionTableArray(FunctionTableEntry* arr) {
  free(arr);
}