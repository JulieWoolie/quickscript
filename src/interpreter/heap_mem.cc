#include "heap_mem.h"

#include <stdlib.h>

#include "../qs_math.h"

bool MemoryRange::isInside(const MemoryRange& other) const {
  return other.start >= start && other.end <= end;
}

bool MemoryRange::isInside(const uint64 oStart, const uint64 oEnd) const {
  return oStart >= start && oEnd <= end;
}

HeapPage::HeapPage() {

}

HeapPage::~HeapPage() {

}
void HeapPage::getRange(MemoryRange& out) const {
  out.start = reinterpret_cast<uint64>(data);
  out.end = out.start + pageSize;
}

int64 HeapMemory::findGap(const uint64 bytes, const uint8 alignment, MemoryRange& out, uint32& gapIndex) const {
  const uint32 size = m_gaps.size();

  for (uint32 i = 0; i < size; i++) {
    const MemoryRange& gap = m_gaps[i];

    const uint64 start = NEXT_MULTIPLE_P2(gap.start, alignment);
    const uint64 end = start + bytes;

    if (!gap.isInside(start, end)) {
      continue;
    }

    out.start = start;
    out.end = gap.end;
    out.pageIndex = gap.pageIndex;

    gapIndex = i;

    return start;
  }

  return -1;
}

bool HeapMemory::popAllocation(const uint64 ptr, MemoryRange& out) {
  for (auto it = m_usedRanges.cbegin(); it != m_usedRanges.cend(); ++it) {
    const MemoryRange& area = *it;

    if (area.start != ptr) {
      continue;
    }

    out = area;
    m_usedRanges.erase(it);

    return true;
  }
  return false;
}

void HeapMemory::findSurroundingGaps(const MemoryRange& area, int32& beforeIdx, int32& afterIdx) const {
  beforeIdx = -1;
  afterIdx = -1;

  const uint32 size = m_gaps.size();

  for (uint32 i = 0; i < size; i++) {
    const MemoryRange& range = m_gaps[i];
    if (range.pageIndex != area.pageIndex) {
      continue;
    }

    if (range.start == area.end) {
      afterIdx = i;
    }
    if (range.end == area.start) {
      beforeIdx = i;
    }
  }
}

HeapMemory::HeapMemory(const uint64 initialHeapSize) {
  const uint64 pagedSize = NEXT_MULTIPLE_P2(initialHeapSize, PAGE_SIZE);
  uint8* block = static_cast<uint8*>(std::malloc(pagedSize));

  memset(block, 0, pagedSize);

  HeapPage page;
  page.data = block;
  page.pageSize = pagedSize;

  m_pages.push_back(page);

  MemoryRange gap;
  gap.start = reinterpret_cast<uint64>(block);
  gap.end = gap.start + pagedSize;
  gap.pageIndex = 0;

  m_gaps.push_back(gap);
}

HeapMemory::~HeapMemory() {
  const uint32 size = m_pages.size();
  for (uint32 i = 0; i < size; i++) {
    HeapPage& page = m_pages[i];

    if (!page.data) {
      continue;
    }

    std::free(page.data);
    page.data = nullptr;
  }
}

void* HeapMemory::allocate(const uint64 memSize, const uint8 alignment) {
  MemoryRange existingGap;
  uint32 idx = 0;
  const int64 start = findGap(memSize, alignment, existingGap, idx);

  if (start != -1) {
    const uint64 end = start + memSize;

    MemoryRange allocation;
    allocation.start = start;
    allocation.end = end;
    allocation.pageIndex = existingGap.pageIndex;
    m_usedRanges.push_back(allocation);

    if (start == existingGap.start) {
      existingGap.start += memSize;

      if (existingGap.end == existingGap.start) {
        m_gaps.erase(m_gaps.cbegin() + idx);
      } else {
        MemoryRange& g = m_gaps[idx];
        g.start = existingGap.start;
      }

      return reinterpret_cast<void*>(start);
    }

    if (end == existingGap.end) {
      m_gaps[idx].end = start;
      return reinterpret_cast<void*>(start);
    }

    MemoryRange afterGap;
    afterGap.start = end;
    afterGap.end = existingGap.end;
    afterGap.pageIndex = existingGap.pageIndex;

    m_gaps[idx].end = start;

    m_gaps.push_back(afterGap);

    return reinterpret_cast<void*>(start);
  }

  const uint64 pagedSize = NEXT_MULTIPLE_P2(memSize, PAGE_SIZE);
  uint8* block = static_cast<uint8*>(std::malloc(pagedSize));

  if (!block) {
    return nullptr;
  }

  const uint64 addr = reinterpret_cast<uint64>(block);

  HeapPage page;
  page.data = block;
  page.pageSize = pagedSize;

  MemoryRange gap;
  page.getRange(gap);
  gap.pageIndex = m_pages.size();
  gap.start += memSize;

  m_pages.push_back(page);
  m_gaps.push_back(gap);
  m_usedRanges.emplace_back(addr, gap.start, gap.pageIndex);

  return block;
}

void HeapMemory::freeMemory(void* ptr) {
  MemoryRange area;
  if (!popAllocation(reinterpret_cast<uint64>(ptr), area)) {
    return;
  }

  // Zero the memory
  uint8* dataPtr = reinterpret_cast<uint8*>(area.start);
  const uint64 dataSize = area.end - area.start;
  memset(dataPtr, 0, dataSize);

  int32 beforeIdx;
  int32 afterIdx;
  findSurroundingGaps(area, beforeIdx, afterIdx);

  if (beforeIdx != -1 && afterIdx != -1) {
    const uint64 gapEnd = m_gaps[afterIdx].end;
    m_gaps[beforeIdx].end = gapEnd;
    m_gaps.erase(m_gaps.cbegin() + afterIdx);
    return;
  }

  if (afterIdx != -1) {
    // After found, decrement beginning
    m_gaps[afterIdx].start = area.start;
    return;
  }

  if (beforeIdx != -1) {
    // Before found, increment ending
    m_gaps[beforeIdx].end = area.end;
    return;
  }

  // Neither found, create new gap
  m_gaps.push_back(area);
}

QsArray HeapMemory::allocArray(const uint32 count, const uint64 elemSize) {
  const uint64 memSize = LENGTH_PREFIX_SIZE + REFCOUNT_PREFIX_SIZE + (count * elemSize);
  void* ptr = allocate(memSize);

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
  void* ptr = allocate(memSize);

  uint32* prefix = static_cast<uint32*>(ptr);
  *prefix = count;

  uint8* dataPtr = static_cast<uint8*>(ptr) + LENGTH_PREFIX_SIZE;

  return QsArray(count, nullptr, dataPtr);
}

QsArray HeapMemory::allocConstString(uint32 length) {
  return allocConstArray(length, 1);
}

QsObject HeapMemory::allocObject(const uint64 dataSize, const uint8 alignment) {
  const uint64 memSize = REFCOUNT_PREFIX_SIZE + dataSize;
  void* ptr = allocate(memSize, alignment);

  *static_cast<uint32*>(ptr) = REFCOUNT_MASK;

  return castToQsObject(ptr);
}

uint64 HeapMemory::getTotalMemory() const {
  const uint32 pages = m_pages.size();
  uint64 result = 0;

  for (uint32 i = 0; i < pages; i++) {
    result += m_pages[i].pageSize;
  }

  return result;
}

uint64 HeapMemory::getUsedMemory() const {
  const uint32 objects = m_usedRanges.size();
  uint64 result = 0;

  for (uint32 i = 0; i < objects; i++) {
    const MemoryRange& range = m_usedRanges[i];
    result += (range.end - range.start);
  }

  return result;
}
