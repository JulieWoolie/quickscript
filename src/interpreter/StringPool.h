#ifndef QUICKSCRIPT_STRINGPOOL_H
#define QUICKSCRIPT_STRINGPOOL_H

#include "../common.h"

#define STRING_POOL_GROWTH 512

struct PoolOffsetRewrite {
  uint64* replacedOffset;
  uint64* replaceWith;
  uint32 len;

  PoolOffsetRewrite(uint64* replacedOffsets, uint64* replacedWith, uint32 len);

  ~PoolOffsetRewrite();
};

class StringPool {
  uint8* m_data = nullptr;
  uint64 m_len = 0;
  uint64 m_cap = 0;

  public:
    StringPool();
    ~StringPool();

    uint32 getLength(uint64 poolOffset) const;
    uint8* getCharacterData(uint64 poolOffset) const;

    uint64 emplaceString(uint32 len, const uint8* data);

    PoolOffsetRewrite emplacePoolData(uint8* data, uint64 len);
};


#endif //QUICKSCRIPT_STRINGPOOL_H
