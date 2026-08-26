#ifndef QUICKSCRIPT_HEAP_MEM_H
#define QUICKSCRIPT_HEAP_MEM_H

#include "../objects.h"
#include "../common.h"

class HeapMemory {

  public:
    void* allocBytes(uint64 bytes);
    void freeBytes(void* ptr);

    QsArray allocArray(uint32 count, uint64 elemSize);
    QsArray allocString(uint32 length);

    QsArray allocConstArray(uint32 count, uint64 elemSize);
    QsArray allocConstString(uint32 length);

    QsObject allocObject(uint64 dataSize);
};

#endif //QUICKSCRIPT_HEAP_MEM_H
