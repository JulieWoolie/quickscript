
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

  char temp[len + 1];
  memcpy(temp, str, len);
  temp[len] = '\0';

  char* ptr;
  uint64 off;

  if (m_data) {
    ptr = strstr(m_data, temp);
  } else {
    ptr = nullptr;
  }

  if (!ptr) {
    uint32 nlen = m_dataLen + len;

    if (nlen > m_dataCap || !m_data) {
      uint32 growth = len < 1024 ? 1024 : len + 100;
      uint32 ncap = m_dataCap + growth;

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

    m_data[m_dataLen] = '\0';
  } else {
    off = ptr - m_data;

    if (m_lenEntries > 1) {
      const StringEntry* entry = m_lengths + 1;

      for (uint32 i = 1; i < m_lenEntries; i++) {
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

int32 StringTable::getlen(const stringid id) const {
  if (id >= m_lenEntries) {
    return -1;
  }
  if (id == 0) {
    return 0;
  }
  return m_lengths[id].len;
}

int32 StringTable::getchars(const stringid id, char *out, const uint32 maxout) const {
  if (id >= m_lenEntries || maxout == 0) {
    return -1;
  }
  if (id == 0) {
    out[0] = '\0';
    return 0;
  }

  auto [offset, len] = m_lengths[id];
  uint32 copied = 0;

  if (len < maxout) {
    memcpy(out, m_data + offset, len);
    out[len] = '\0';
    copied = len;
  } else {
    memcpy(out, m_data + offset, maxout - 1);
    out[maxout - 1] = '\0';
    copied = maxout;
  }

  return copied;
}

std::string StringTable::getstring(stringid id) {
  if (id == EMPTY_STRING || id >= m_lenEntries) {
    return "";
  }

  auto [offset, len] = m_lengths[id];

  char content[len];
  memcpy(content, m_data + offset, len);

  return std::string(content, len);
}
