#include "stringreader.h"

#include "strings.h"
StringReader::StringReader(const std::string_view& view)
    : m_buf(reinterpret_cast<const utf8char*>(view.data())),
      m_len(view.length())
{

}

StringReader::StringReader(const conststring str)
  : m_buf(reinterpret_cast<const utf8char*>(str)),
    m_len(strlen(str))
{

}

StringReader::StringReader(const conststring str, const uint32 len)
  : m_buf(reinterpret_cast<const utf8char*>(str)),
    m_len(len)
{

}

uint32 StringReader::length() const {
  return m_len;
}

conststring StringReader::content() const {
  return reinterpret_cast<conststring>(m_buf);
}

uint32& StringReader::cursor() {
  return m_cursor;
}

uint32 StringReader::remaining() const {
  return m_len - m_cursor;
}

std::string_view StringReader::remainingView() const {
  return std::string_view(reinterpret_cast<conststring>(m_buf + m_cursor), m_len - m_cursor);
}

std::string_view StringReader::substring(const uint32 start, const uint32 end) const {
  return std::string_view(content() + start, end - start);
}

bool StringReader::hasNext() const {
  return m_cursor < m_len;
}

void StringReader::skipWhitespace() {
  utf32char p;
  while (hasNext() && isWhitespace(p = peek())) {
    m_cursor += getUtf8ByteLength(p);
  }
}

utf32char StringReader::peek() const {
  if (!hasNext()) {
    return 0;
  }

  const utf8char* buf = m_buf + m_cursor;
  utf32char ch = 0;
  decodeUtf8(buf, &ch, m_len - m_cursor);
  return ch;
}

utf32char StringReader::next() {
  if (!hasNext()) {
    return 0;
  }

  const utf8char* buf = m_buf + m_cursor;
  utf32char ch = 0;

  const uint8 read = decodeUtf8(buf, &ch, m_len - m_cursor);

  m_cursor += read;
  return ch;
}

conststring StringReader::substring(const uint32 idx) const {
  return reinterpret_cast<conststring>(m_buf + idx);
}

bool StringReader::isNext(const conststring str) const {
  return isNext(str, strlen(str));
}

bool StringReader::isNext(const conststring str, const uint32 len) const {
  if (remaining() < len) {
    return false;
  }
  for (uint32 i = 0; i < len; i++) {
    if (m_buf[i + m_cursor] == str[i]) {
      continue;
    }
    return false;
  }
  return true;
}

bool StringReader::consumeIfMatches(const conststring str) {
  const uint32 len = strlen(str);

  if (!isNext(str, len)) {
    return false;
  }

  m_cursor += len;
  return true;
}

bool StringReader::parseBool(bool fallback) {
  if (isNext("true")) {
    m_cursor += 4;
    return true;
  }
  if (isNext("false")) {
    m_cursor += 5;
    return false;
  }
  return fallback;
}
