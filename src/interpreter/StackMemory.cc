#include "StackMemory.h"

#include <cstdlib>

#define KB 1024

StackMemory::StackMemory() {
  m_buf = static_cast<uint8*>(malloc(STACK_START_SIZE));
  m_cap = STACK_START_SIZE;
}

StackMemory::~StackMemory() {
  if (m_buf) {
    free(m_buf);
    m_buf = nullptr;
  }

  m_cap = 0;
  m_usedLen = 0;
}

static uint64 newStackSize(const uint64 len) {
  return (len / KB + (len % KB == 0 ? 0 : 1)) * KB;
}

uint8* StackMemory::allocateFrame(const uint64 bytes) {
  const uint64 newLen = m_usedLen + bytes;

  if (newLen > m_cap) {
    const uint64 newCap = newStackSize(newLen);
    uint8* newBuf = static_cast<uint8*>(realloc(m_buf, newCap));

    if (!newBuf) {
      return nullptr;
    }

    m_cap = newCap;
    m_buf = newBuf;
  }

  uint8* ptr = m_buf + m_usedLen;
  m_usedLen = newLen;

  return ptr;
}

void StackMemory::popFrame(const uint64 bytes) {
  const uint64 newLen = m_usedLen - bytes;

  if (newLen > m_usedLen) {
    // 64bit overflow
    // TODO: Handle better?
    return;
  }

  m_usedLen = newLen;
}

uint8* StackMemory::getBuffer() const {
  return m_buf;
}

uint64 StackMemory::getCapacity() const {
  return m_cap;
}

uint64 StackMemory::getUsedBytes() const {
  return m_usedLen;
}
