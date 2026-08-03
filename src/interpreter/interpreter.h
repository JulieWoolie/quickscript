#ifndef QUICKSCRIPT_INTERPRETER_H
#define QUICKSCRIPT_INTERPRETER_H

#include "HeapMemory.h"
#include "Stack.h"
#include "StackMemory.h"
#include "../common.h"

#define REGISTERS_COUNT 64

struct StackFrame {
  uint8* stackBase = nullptr;
  uint64 allocatedSize = 0;
};

struct CallFrame {
  uint32 line = 0;
  std::string name;
  std::string filename;
};

class InterpreterState {
  Stack<StackFrame> m_stackFrames;
  Stack<CallFrame> m_callStack;

  StackMemory m_stack;
  HeapMemory m_heap;

  uint64 m_registers[REGISTERS_COUNT] = {};
};

#endif //QUICKSCRIPT_INTERPRETER_H
