#ifndef QUICKSCRIPT_HEAP_MEM_H
#define QUICKSCRIPT_HEAP_MEM_H

#include <vector>

#include "../objects.h"
#include "../common.h"

#define PAGE_SIZE 1024
#define PAGE_SHIFT 10

// -------------------------------------------------------------------------------------------------
// Notes:
// -------------------------------------------------------------------------------------------------
//
// data divided into 'pages' and open space is tracked with FreeMemoryGap structs
//
// MemoryGap {
//   uint32 pageIndex
//   pointer start
//   pointer end
// }
//
// Allocation {
//   pointer address
//   uint64 size
//   uint32 pageIndex
// }
// -------------------------------------------------------------------------------------------------
// To find a memory gap:
//   Take 'required bytes' as a parameter of uint64
//   Take 'alignment' as a parameter of uint8, default to a value of 1 if not specified
//
//   Iterate over each existing gap:
//     If alignment is 1 or if the gap's starting address already matches the specified alignment:
//       If the gap's size is greater than or equal to the required bytes:
//         Return the gap
//       Otherwise, move on to the next gap
//     Temporarily increase the gap's starting address until its of the proper alignment
//     If the gap's size is now greater  than or equal to the required bytes:
//       Return the gap and the incremented starting address.
//     Otherwise, move on to the next gap
// -------------------------------------------------------------------------------------------------
// To allocate:
//   Take 'required bytes' as a parameter of uint64
//   Take 'alignment' as a parameter of uint8, default to 1, if not specified.
//   Find a gap that has enough space
//
//   If gap found:
//     If the allocation would not start from the start of the gap:
//       If the allocation reaches until the end of the gap:
//         Decrease the gap's ending pointer until the start of the allocated memory
//       Otherwise:
//         Decrease the gap's ending pointer until the start of the allocated memory
//         Create a second gap for the ending bytes
//     Decrease gap size by amount of bytes
//     If gap is now 0: delete the gap
//     Return the former start address of the gap
//     Push an Allocation to the allocated memory list
//
//   If gap NOT found:
//     Allocate new page
//     If base page size is less than required size, use that for the new page's size
//     Otherwise round the required bytes size to the nearest multiple of the base
//       page size and use that as the required byte amount.
//     Create a MemoryGap with a size of (allocated page size - required bytes)
//     Push the gap to the free memory block list
//     Push an Allocation to the allocated memory list
//     Return the pointer to the beginning of the created block.
// -------------------------------------------------------------------------------------------------
// To free memory:
//   Take 'pointer' as a parameter of void*, this is the pointer to the
//     memory that must be freed.
//   Find the associated Allocation struct for the pointer
//   Find a memory gap that borders the allocated block.
//
//   Delete the associated allocation, or return and do nothing if it wasn't found.
//
//   If a memory gap was found before AND after the block:
//     Delete the 2nd one and set the 1st one's end address to be the 2nd's end address.
//
//   If a memory gap was found before the block:
//     Decrement the gap's starting address to the allocated block's start address
//
//   If a memory gap was found after the block:
//     Increase the gap's ending address to the allocated block's end address
//
//   If no gap was found:
//     Create and push a gap that begins at the allocated block's pointer and
//       ends at the block's end.
//
//   Return.
// -------------------------------------------------------------------------------------------------

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
