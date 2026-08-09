#ifndef QUICKSCRIPT_STRINGTABLE_H
#define QUICKSCRIPT_STRINGTABLE_H

#include <string>

#include "common.h"

#define EMPTY_STRING 0

struct StringRef {
  const uint32 index;
  const uint32 len;

  conststring data;

  StringRef(uint32 idx, int32 len, int8* data);

  std::string_view view() const;
};

typedef StringRef* stringid;

struct StringEntry {
  uint64 offset = 0;
  uint32 len = 0;
  StringRef* ref = nullptr;
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
    ~StringTable();

    stringid allocate(conststring str);
    stringid allocate(conststring str, uint32 len);
    stringid allocate(const std::string& str);

    stringid findId(const std::string& str) const;

    std::string_view getview(stringid id) const;
    int32 getlen(stringid id) const;
    int32 getchars(stringid id, char* out, uint32 maxout) const;
    int32 copychars(stringid id, char* out, uint32 maxout) const;

    std::string getstring(stringid id);
};

#endif //QUICKSCRIPT_STRINGTABLE_H
