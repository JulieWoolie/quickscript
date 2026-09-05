#ifndef QUICKSCRIPT_HEAP_MEM_H
#define QUICKSCRIPT_HEAP_MEM_H

#include <vector>

#include "../objects.h"
#include "../common.h"

#define PAGE_SIZE 1024
#define PAGE_SHIFT 10

struct MemoryRange {
  uint64 start;
  uint64 end;
  uint32 pageIndex;

  bool isInside(const MemoryRange& other) const;

  bool isInside(uint64 oStart, uint64 oEnd) const;
};

struct HeapPage {
  uint64 pageSize = 0;
  uint8* data = nullptr;

  HeapPage();
  ~HeapPage();

  void getRange(MemoryRange& out) const;
};

class HeapMemory {
  std::vector<MemoryRange> m_usedRanges;
  std::vector<MemoryRange> m_gaps;
  std::vector<HeapPage> m_pages;

  int64 findGap(uint64 bytes, uint8 alignment, MemoryRange& out, uint32& gapIndex) const;

  bool popAllocation(uint64 ptr, MemoryRange& out);

  void findSurroundingGaps(const MemoryRange& area, int32& beforeIdx, int32& afterIdx) const;

  public:
    explicit HeapMemory(uint64 initialHeapSize = PAGE_SIZE);
    ~HeapMemory();

    void* allocate(uint64 memSize, uint8 alignment = 1);

    void freeMemory(void* ptr);

    QsArray allocArray(uint32 count, uint64 elemSize);
    QsArray allocString(uint32 length);

    QsArray allocConstArray(uint32 count, uint64 elemSize);
    QsArray allocConstString(uint32 length);

    QsObject allocObject(uint64 dataSize, uint8 alignment);

    uint64 getTotalMemory() const;
    uint64 getUsedMemory() const;
};

#endif //QUICKSCRIPT_HEAP_MEM_H
