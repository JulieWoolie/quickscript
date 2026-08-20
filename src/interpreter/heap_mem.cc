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
  const uint64 memSize = LENGTH_PREFIX_SIZE + REFCOUNT_PREFIX_SIZE + (count * elemSize);
  void* ptr = allocBytes(memSize);

  uint32* prefix = static_cast<uint32*>(ptr);
  prefix[0] = REFCOUNT_MASK;
  prefix[1] = count;

  uint8* dataPtr = static_cast<uint8*>(ptr) + LENGTH_PREFIX_SIZE + REFCOUNT_PREFIX_SIZE;

  return QsArray(count, prefix, dataPtr);
}

QsArray HeapMemory::allocString(const uint32 length) {
  return allocArray(length, 1);
}

QsArray HeapMemory::allocConstArray(const uint32 count, const uint64 elemSize) {
  const uint64 memSize = LENGTH_PREFIX_SIZE + (count * elemSize);
  void* ptr = allocBytes(memSize);

  uint32* prefix = static_cast<uint32*>(ptr);
  *prefix = count;

  uint8* dataPtr = static_cast<uint8*>(ptr) + LENGTH_PREFIX_SIZE;

  return QsArray(count, nullptr, dataPtr);
}

QsArray HeapMemory::allocConstString(uint32 length) {
  return allocConstArray(length, 1);
}
