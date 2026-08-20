#include "StringPool.h"

#include <cstdlib>
#include <vector>

PoolOffsetRewrite::PoolOffsetRewrite(uint64* replacedOffsets, uint64* replacedWith, uint32 len)
  : replacedOffset(replacedOffsets),
    replaceWith(replacedWith),
    len(len)
{

}

PoolOffsetRewrite::~PoolOffsetRewrite() {
  if (!replacedOffset) {
    return;
  }

  free(replacedOffset);

  replacedOffset = nullptr;
  replaceWith = nullptr;
  len = 0;
}

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

uint32 StringPool::getLength(const uint64 poolOffset) const {
  return STRPOOL_LEN(m_data, poolOffset);
}

int8* StringPool::getCharacterData(const uint64 poolOffset) const {
  return STRPOOL_DATA(m_data, poolOffset);
}

uint64 StringPool::emplaceString(const uint32 len, const uint8* data) {
  uint64 off = 0;
  while (off < m_len) {
    uint32 eLen = STRPOOL_LEN(m_data, off);
    if (eLen != len) {
      off += eLen + sizeof(uint32);
      continue;
    }

    const int8* eContent = STRPOOL_DATA(m_data, off);
    if (memcmp(data, eContent, eLen) == 0) {
      return off;
    }

    off += eLen + sizeof(uint32);
  }

  const uint64 requiredIncrease = len + sizeof(uint32);
  const uint64 newLen = m_len + requiredIncrease;

  if (newLen > m_cap) {
    uint64 newCap;

    if (requiredIncrease < STRING_POOL_GROWTH) {
      newCap = m_cap + STRING_POOL_GROWTH;
    } else {
      newCap = m_cap + requiredIncrease;
    }

    uint8* newBuf = static_cast<uint8*>(realloc(m_data, newCap));
    if (!newBuf) {
      throw std::runtime_error("Failed to increase string buffer size");
    }

    m_cap = newCap;
    m_data = newBuf;
  }

  *reinterpret_cast<uint32*>(m_data + m_len) = len;
  memcpy(m_data + m_len + sizeof(uint32), data, len);

  const uint64 offset = m_len;
  m_len = newLen;

  return offset;
}

PoolOffsetRewrite StringPool::emplacePoolData(uint8* data, uint64 len) {
  std::vector<uint64> replaced;
  std::vector<uint64> replacedWith;

  uint32 rewritten = 0;

  uint64 off = 0;
  while (off < len) {
    const uint32 strLength = *reinterpret_cast<uint32*>(data + off);
    const uint8* content = data + off + sizeof(uint32);

    uint64 emplacedOffset = emplaceString(strLength, content);

    if (emplacedOffset == off) {
      off += strLength + sizeof(uint32);
      continue;
    }

    replaced.push_back(off);
    replacedWith.push_back(emplacedOffset);
    rewritten++;

    off += strLength + sizeof(uint32);
  }

  if (rewritten == 0) {
    return PoolOffsetRewrite(nullptr, nullptr, 0);
  }

  const uint64 memSize = rewritten * 2 * sizeof(uint64);
  uint64* replacedOffsets = static_cast<uint64*>(malloc(memSize));
  uint64* replacedOffsetsWith = replacedOffsets + rewritten;

  for (uint32 i = 0; i < rewritten; i++) {
    replacedOffsets[i] = replaced[i];
    replacedOffsetsWith[i] = replacedWith[i];
  }

  return PoolOffsetRewrite(replacedOffsets, replacedOffsetsWith, rewritten);
}
