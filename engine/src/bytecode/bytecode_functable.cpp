#include "qs/bytecode/bytecode_functable.hpp"

FunctionTableEntry* createFunctionTableArray(const uint32 entries) {
  if (entries == 0) {
    return nullptr;
  }
  return static_cast<FunctionTableEntry*>(malloc(sizeof(FunctionTableEntry) * entries));
}

void freeFunctionTableArray(FunctionTableEntry* arr) {
  free(arr);
}