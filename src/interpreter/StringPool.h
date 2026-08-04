#ifndef QUICKSCRIPT_STRINGPOOL_H
#define QUICKSCRIPT_STRINGPOOL_H

#include "../common.h"

struct PoolOffsetRewrite {
  uint64 replacedOffset;
  uint64 replaceWith;
  uint32 len;
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

    uint64 emplaceString(uint32 len, uint8* data);

    void emplacePoolData(uint8* data, uint64 len);
};


#endif //QUICKSCRIPT_STRINGPOOL_H
