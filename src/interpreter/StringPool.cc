#include "StringPool.h"

#include <cstdlib>
StringPool::StringPool() {

}

StringPool::~StringPool() {
  if (m_data) {
    free(m_data);
  }

  m_data = nullptr;
  m_cap = 0;
  m_len = 0;
}

uint32 StringPool::getLength(uint64 poolOffset) const {
  return *reinterpret_cast<uint32*>(m_data + poolOffset);
}

uint8* StringPool::getCharacterData(uint64 poolOffset) const {
  return m_data + poolOffset + sizeof(uint32);
}

uint64 StringPool::emplaceString(uint32 len, uint8* data) {

}

void StringPool::emplacePoolData(uint8* data, uint64 len) {

}
