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

    uint32 length() const;

    conststring content() const;

    uint32& cursor();

    uint32 remaining() const;

    std::string_view remainingView() const;

    std::string_view substring(uint32 start, uint32 end) const;

    bool hasNext() const;

    void skipWhitespace();

    utf32char peek() const;

    utf32char next();

    conststring substring(uint32 idx) const;

    bool isNext(conststring str) const;

    bool isNext(conststring str, uint32 len) const;

    bool consumeIfMatches(conststring str);

    bool parseBool(bool fallback = false);
};

#endif //QUICKSCRIPT_STRINGREADER_H
