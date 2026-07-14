#ifndef QUICKSCRIPT_ALLOCATOR_H
#define QUICKSCRIPT_ALLOCATOR_H

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

#include "common.h"

// ====
// Allocator which assumes all objects will never be freed,
// or at least, will be freed all at once
// ====

#define CHUNK_SIZE ((uint64) (32 * 1024))
#define START_CHUNKS 5

struct Chunk {
  uint8* data = nullptr;
  uint64 cap = 0;
  uint64 cursor = 0;
};

class NoFreeAllocator {
  Chunk* m_chunkData = nullptr;
  uint32 m_capacity = 0;

  public:
    NoFreeAllocator() = default;

    template<typename T>
    T* emplace(T node);

    template<typename T>
    T* make();

    void reset();

  private:
    uint8* allocate(uint64 sz);
    Chunk* findFreeChunk(uint64 sizebytes) const;
};

template<typename T>
T* NoFreeAllocator::emplace(T node) {
  uint64 msize = sizeof(T);
  uint8* ptr = allocate(msize);
  return new (ptr) T(std::move(node));
}

template<typename T>
T* NoFreeAllocator::make() {
  uint64 msize = sizeof(T);
  uint8* ptr = allocate(msize);
  return new (ptr) T();
}

inline uint8 * NoFreeAllocator::allocate(uint64 sz) {
  Chunk* fc = findFreeChunk(sz);

  if (!fc) {
    uint32 ncap = m_capacity + 5;
    uint64 csize = sizeof(Chunk);

    Chunk* nchunks = (Chunk*) realloc(m_chunkData, ncap * csize);

    if (!nchunks) {
      throw std::runtime_error("Failed to allocate more memory chunks");
    }

    memset(nchunks + m_capacity, 0, (ncap - m_capacity) * csize);

    fc = nchunks + m_capacity;

    m_chunkData = nchunks;
    m_capacity = ncap;
  }

  if (!fc->data) {
    uint64 memsize = std::max(sz, CHUNK_SIZE);
    uint8* data = (uint8*) malloc(memsize);

    if (!data) {
      throw std::runtime_error("Failed to allocate chunk for allocator");
    }

    fc->data = data;
    fc->cap = memsize;
  }

  uint8* res = fc->data + fc->cursor;
  fc->cursor += sz;

  return res;
}

inline Chunk* NoFreeAllocator::findFreeChunk(uint64 sizebytes) const {
  if (!m_chunkData) {
    return nullptr;
  }

  for (uint32 i = 0; i < m_capacity; i++) {
    Chunk* c = m_chunkData + i;

    if (!c->data) {
      return c;
    }

    uint64 free = c->cap - c->cursor;
    if (free >= sizebytes) {
      return c;
    }
  }

  return nullptr;
}

#endif //QUICKSCRIPT_ALLOCATOR_H
