#ifndef QS_HEAP_MEM_H
#define QS_HEAP_MEM_H

#include <vector>
#include <unordered_map>

#include "qs/objects.hpp"
#include "qs/common.hpp"

#define PAGE_SIZE 1024
#define PAGE_SHIFT 10

struct MemoryRange {
  uint64 start;
  uint64 end;

  bool isInside(const MemoryRange& other) const;

  bool isInside(uint64 oStart, uint64 oEnd) const;

  uint64 size() const;
};

struct HeapPage {
  uint64 pageSize = 0;
  uint8* data = nullptr;

  HeapPage();
  ~HeapPage();

  void getRange(MemoryRange& out) const;
};

class HeapMemory {
  std::unordered_map<uint64, uint64> m_usedRanges;
  std::vector<MemoryRange> m_gaps;
  std::vector<HeapPage> m_pages;

  uint64 m_totalMemory = 0;
  uint64 m_usedMemory = 0;

  int64 findGap(uint64 bytes, uint8 alignment, MemoryRange& out, uint32& gapIndex) const;

  bool popAllocation(uint64 ptr, MemoryRange& out);

  void pushAllocation(uint64 start, uint64 end);

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

#endif //QS_HEAP_MEM_H
