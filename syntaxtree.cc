
#include "syntaxtree.h"

#include <cstring>
#include <stdexcept>

template<typename T>
NodeRef<T> NodePool::emplace(T node) {
  uint64 sz = sizeof(T);
  uint64 idx = allocspace(sz);

  uint8* dstart = m_data + idx;
  *static_cast<T*>(dstart) = node;

  return idx;
}

template<typename T>
NodeAndRef<T> NodePool::make() {
  uint64 sz = sizeof(T);
  uint64 idx = allocspace(sz);

  uint8* ndata = m_data + idx;

  memset(ndata, 0, sz);

  new (ndata) T();

  NodeAndRef<T> nr = {};

  nr.node = ndata;
  nr.ref = idx;

  return nr;
}

template<typename T>
T* NodePool::get(NodeRef<T> ref) {
  if (ref >= cursor) {
    return nullptr;
  }

  uint8* dstart = m_data + ref;
  return static_cast<T *>(dstart);
}

uint64 NodePool::allocspace(const uint64 sz) {
  uint64 nsize = cursor + sz;

  if (nsize > capacity) {
    uint64 ncap = capacity + 1024;
    uint8* ndata = static_cast<uint8 *>(realloc(m_data, ncap));

    if (!ndata) {
      throw std::runtime_error("Error expanding node pool");
    }

    m_data = ndata;
    capacity = ncap;
  }

  uint64 ref = cursor;
  cursor += sz;

  return ref;
}
