
#include "stringtable.h"

#include <cstring>
#include <stdexcept>

StringTable::StringTable() {

  // Create the 'empty string' entry
  m_lengths = static_cast<StringEntry*>(malloc(sizeof(StringEntry) * 100));
  m_lenEntries = 1;
  m_lenCap = 100;
  m_lengths->len = 0;
  m_lengths->offset = 0;
}

stringid StringTable::allocate(const conststring str) {
  uint32 len = strlen(str);
  return allocate(str, len);
}

stringid StringTable::allocate(const conststring str, const uint32 len) {
  if (len <= 0) {
    return 0;
  }

  char* ptr = strstr(str, m_data);
  uint64 off;

  if (!ptr) {
    uint32 nlen = m_dataLen + len;
    if (nlen > m_dataCap) {
      uint32 ncap = m_dataCap + 1024;
      char* ndata = static_cast<char*>(realloc(m_data, sizeof(char) * ncap));

      if (!ndata) {
        throw std::runtime_error("Failed to allocate more room on string table");
      }

      m_data = ndata;
      m_dataCap = ncap;
    }

    char* dst = m_data + m_dataLen;
    memcpy(dst, str, len);

    off = m_dataLen;
    m_dataLen += len;
  } else {
    off = ptr - m_data;

    if (m_lenEntries > 0) {
      const StringEntry* entry = m_lengths;

      for (uint32 i = 0; i < m_lenEntries; i++) {
        if (entry->offset != off || entry->len != len) {
          entry++;
          continue;
        }
        return i;
      }
    }
  }

  if (m_lenEntries >= m_lenCap) {
    uint64 nlencap = m_lenCap + 100;
    StringEntry* ntable = static_cast<StringEntry*>(realloc(m_lengths, nlencap * sizeof(StringEntry)));

    if (!ntable) {
      throw std::runtime_error("Failed to increase size of string metadata table");
    }

    m_lengths = ntable;
    m_lenCap = nlencap;
  }

  StringEntry* entry = m_lengths + m_lenEntries;
  const stringid id = m_lenEntries;

  entry->len = len;
  entry->offset = off;

  m_lenEntries++;

  return id;
}

stringid StringTable::allocate(const std::string& str) {
  return allocate(str.c_str(), str.length());
}

std::string_view StringTable::getview(const stringid id) const {
  if (id >= m_lenEntries) {
    return {};
  }
  if (id == 0) {
    return "";
  }

  StringEntry entry = m_lengths[id];
  return {m_data + entry.offset, entry.len};
}
