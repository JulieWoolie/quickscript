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

uint32& StringReader::cursor() {
  return m_cursor;
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
  const utf8char* buf = m_buf + m_cursor;
  utf32char ch = 0;
  decodeUtf8(buf, &ch, m_len - m_cursor);
  return ch;
}

utf32char StringReader::next() {
  const utf8char* buf = m_buf + m_cursor;
  utf32char ch = 0;

  const uint8 read = decodeUtf8(buf, &ch, m_len - m_cursor);

  m_cursor += read;
  return ch;
}

conststring StringReader::substring(const uint32 idx) const {
  return reinterpret_cast<conststring>(m_buf + idx);
}
