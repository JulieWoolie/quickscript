#ifndef QS_STACKMEMORY_H
#define QS_STACKMEMORY_H

#include "qs/common.hpp"

#define STACK_START_SIZE 1024 * 64

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


#endif //QS_STACKMEMORY_H
