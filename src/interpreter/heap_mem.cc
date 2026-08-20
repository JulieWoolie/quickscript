#include <cstdlib>
#include "heap_mem.h"

#include <cstring>

void* HeapMemory::allocBytes(uint64 bytes) {
  void* result = malloc(bytes);
  memset(result, 0, bytes);
  return result;
}

void HeapMemory::freeBytes(void* ptr) {
  free(ptr);
}

QsArray HeapMemory::allocArray(const uint32 count, const uint64 elemSize) {
  const uint64 memSize = LENGTH_PREFIX_SIZE + (count * elemSize);
  void* ptr = allocBytes(memSize);

  *static_cast<uint32*>(ptr) = count;
  return QsArray(count, static_cast<uint8*>(ptr) + LENGTH_PREFIX_SIZE);
}

QsString HeapMemory::allocString(uint32 length) {
  const uint64 memSize = LENGTH_PREFIX_SIZE + length;
  void* ptr = allocBytes(memSize);

  *static_cast<uint32*>(ptr) = length;
  return QsString(length, static_cast<int8*>(ptr) + LENGTH_PREFIX_SIZE);
}