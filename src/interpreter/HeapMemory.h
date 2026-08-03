#ifndef QUICKSCRIPT_HEAPMEMORY_H
#define QUICKSCRIPT_HEAPMEMORY_H

#include "../common.h"

struct HeapPage {
  uint8* memoryBlock = nullptr;
  uint64 pageSize = 0;
};

class HeapMemory {
  HeapPage* m_pages = nullptr;
  uint32 m_pageCap = 0;
  uint32 m_pageCount = 0;


};


#endif //QUICKSCRIPT_HEAPMEMORY_H
