#ifndef QUICKSCRIPT_HEAP_MEM_H
#define QUICKSCRIPT_HEAP_MEM_H

#include <cstdlib>

#include "../common.h"

class HeapMemory {

  public:
    void* allocBytes(const uint64 bytes) {
      return malloc(bytes);
    }

    void freeBytes(void* ptr) {
      return free(ptr);
    }
};

#endif //QUICKSCRIPT_HEAP_MEM_H
