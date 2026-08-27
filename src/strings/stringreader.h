#ifndef QUICKSCRIPT_STRINGREADER_H
#define QUICKSCRIPT_STRINGREADER_H

#include <string_view>

#include "utf8.h"
#include "../common.h"

class StringReader {
  const utf8char* const m_buf;
  const uint32 m_len;

  uint32 m_cursor = 0;

  public:
    explicit StringReader(const std::string_view& view);

    explicit StringReader(conststring str);

    explicit StringReader(conststring str, uint32 len);

    uint32& cursor();

    bool hasNext() const;

    void skipWhitespace();

    utf32char peek() const;

    utf32char next();

    conststring substring(uint32 idx) const;
};

#endif //QUICKSCRIPT_STRINGREADER_H
