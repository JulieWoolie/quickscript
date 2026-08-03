#ifndef QUICKSCRIPT_STACKMEMORY_H
#define QUICKSCRIPT_STACKMEMORY_H

#include "../common.h"

class StackMemory {
  uint8* m_buf = nullptr;
  uint64 m_cap = 0;
  uint64 m_usedLen = 0;

  public:
    explicit StackMemory();
    ~StackMemory();

    uint8* allocateFrame(uint64 bytes);
    void popFrame(uint64 bytes);

    uint8* getBuffer() const;
    uint64 getCapacity() const;
    uint64 getUsedBytes() const;
};


#endif //QUICKSCRIPT_STACKMEMORY_H
