#ifndef QUICKSCRIPT_STRINGPOOL_H
#define QUICKSCRIPT_STRINGPOOL_H

#include "../common.h"

#define STRING_POOL_GROWTH 512
#define STRPOOL_LEN(pool, offset) (*reinterpret_cast<uint32*>(pool + offset))
#define STRPOOL_DATA(pool, offset) reinterpret_cast<int8*>(pool + offset + sizeof(uint32))

struct PoolOffsetRewrite {
  uint64* replacedOffset;
  uint64* replaceWith;
  uint32 len;

  PoolOffsetRewrite(uint64* replacedOffsets, uint64* replacedWith, uint32 len);

  ~PoolOffsetRewrite();

  uint64 findReplacement(uint64 stringAddr) const;
};

class StringPool {
  uint8* m_data = nullptr;
  uint64 m_len = 0;
  uint64 m_cap = 0;

  public:
    StringPool();
    ~StringPool();

    uint32 getLength(uint64 poolOffset) const;
    int8* getCharacterData(uint64 poolOffset) const;

    uint64 emplaceString(uint32 len, const uint8* data);

    PoolOffsetRewrite emplacePoolData(uint8* data, uint64 len);
};

#endif //QUICKSCRIPT_STRINGPOOL_H
