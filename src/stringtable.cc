
#include "stringtable.h"

#include <cstring>
#include <stdexcept>

StringRef::StringRef(uint32 _idx, int32 _len, int8* _data)
  : index(_idx), len(_len), data(_data)
{

}

StringTable::StringTable() {
  // Create the 'empty string' entry
  m_lengths = static_cast<StringEntry*>(malloc(sizeof(StringEntry) * 100));
  m_lenEntries = 1;
  m_lenCap = 100;
  m_lengths->len = 0;
  m_lengths->offset = 0;
  m_lengths->ref = new StringRef(0, 0, nullptr);
}

StringTable::~StringTable() {
  if (m_data) {
    free(m_data);
  }
  if (m_lengths) {
    free(m_lengths);
  }
}

stringid StringTable::allocate(const conststring str) {
  uint32 len = strlen(str);
  return allocate(str, len);
}

static void updateRefs(const uint32 len, const StringEntry* entries, const char* buf) {
  if (len < 2) {
    return;
  }
  for (uint32 i = 1; i < len; i++) {
    const StringEntry* entry = &entries[i];
    StringRef* ref = entry->ref;

    if (!ref) {
      continue;
    }

    ref->data = buf + entry->offset;
  }
}

stringid StringTable::allocate(const conststring str, const uint32 len) {
  if (len <= 0) {
    return m_lengths[0].ref;
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

      updateRefs(m_lenEntries, m_lengths, m_data);
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
        return entry->ref;
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
  const uint32 id = m_lenEntries;
  StringRef* ref = new StringRef(id, len, m_data + off);

  entry->len = len;
  entry->offset = off;
  entry->ref = ref;

  m_lenEntries++;

  return ref;
}

stringid StringTable::allocate(const std::string& str) {
  return allocate(str.c_str(), str.length());
}

stringid StringTable::findId(const std::string& str) const {
  if (m_lenEntries < 2) {
    return EMPTY_STRING;
  }

  uint32 len = str.length();

  for (uint32 i = 0; i < m_lenEntries; i++) {
    StringEntry* entry = &m_lengths[i];
    if (entry->len != len) {
      continue;
    }

    char* data = m_data + entry->offset;
    bool matches = true;

    for (uint32 chi = 0; chi < len; chi++) {
      char ch = data[chi];
      if (ch == str[chi]) {
        continue;
      }
      matches = false;
      break;
    }

    if (!matches) {
      continue;
    }

    return entry->ref;
  }

  return EMPTY_STRING;
}

std::string_view StringTable::getview(const stringid id) const {
  if (!id) {
    return {};
  }
  return std::string_view(id->data, id->len);
}

int32 StringTable::getlen(const stringid id) const {
  if (!id) {
    return 0;
  }
  return id->len;
}

int32 StringTable::getchars(const stringid id, char *out, const uint32 maxout) const {
  if (!id || id->index == 0) {
    out[0] = '\0';
    return 0;
  }
  if (id->index >= m_lenEntries || maxout == 0) {
    return -1;
  }

  auto [offset, len, ref] = m_lengths[id->index];
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

int32 StringTable::copychars(stringid id, char* out, uint32 maxout) const {
  if (!id || id->index) {
    return 0;
  }
  if (!id || maxout == 0) {
    return -1;
  }

  auto [offset, len, ref] = m_lengths[id->index];
  uint32 cpylen = std::min(maxout, len);

  memcpy(out, m_data + offset, cpylen);

  return cpylen;
}

std::string StringTable::getstring(stringid id) {
  if (!id || id->index == EMPTY_STRING || id->index >= m_lenEntries) {
    return "";
  }

  auto [offset, len, ref] = m_lengths[id->index];

  char content[len];
  memcpy(content, m_data + offset, len);

  return std::string(content, len);
}
