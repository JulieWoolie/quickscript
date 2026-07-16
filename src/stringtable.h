#ifndef QUICKSCRIPT_STRINGTABLE_H
#define QUICKSCRIPT_STRINGTABLE_H

#include <string>

#include "common.h"

#define EMPTY_STRING 0

typedef uint32 stringid;

struct qsString {
  char* data = nullptr;
  uint32 len = 0;
};

struct StringEntry {
  uint64 offset = 0;
  uint32 len = 0;
};

class StringTable {
  char* m_data = nullptr;
  uint32 m_dataCap = 0;
  uint32 m_dataLen = 0;

  StringEntry* m_lengths = nullptr;
  uint32 m_lenEntries = 0;
  uint32 m_lenCap = 0;

  public:
    StringTable();

    stringid allocate(conststring str);
    stringid allocate(conststring str, uint32 len);
    stringid allocate(const std::string& str);

    std::string_view getview(stringid id) const;
    int32 getlen(stringid id) const;
    int32 getchars(stringid id, char* out, uint32 maxout) const;
};

#endif //QUICKSCRIPT_STRINGTABLE_H
