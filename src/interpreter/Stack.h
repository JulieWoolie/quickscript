#ifndef QUICKSCRIPT_STACK_H
#define QUICKSCRIPT_STACK_H

#include <utility>

#include "../common.h"

#define MAX_CALL_DEPTH 1024

template<typename T, typename... Args>
class Stack {
  T* m_frames[MAX_CALL_DEPTH] = {};
  uint32 m_len = 0;

  T* getFrame(uint32 idx);

  void popFrame();

  T* pushFrame(Args&& ... args);
};

template<typename T, typename... Args>
T* Stack<T, Args...>::getFrame(uint32 idx) {
  return m_frames[idx];
}

template<typename T, typename... Args>
void Stack<T, Args...>::popFrame() {
  if (m_len == 0) {
    return;
  }
  m_len--;
}

template<typename T, typename... Args>
T* Stack<T, Args...>::pushFrame(Args&&... args) {
  if (m_len >= MAX_CALL_DEPTH) {
    return nullptr;
  }

  T* ptr = m_frames + m_len;
  new (ptr) T(std::forward<Args>(args)...);

  m_len++;

  return ptr;
}

#endif //QUICKSCRIPT_STACK_H
