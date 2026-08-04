#ifndef QUICKSCRIPT_INTERPRETER_H
#define QUICKSCRIPT_INTERPRETER_H

#include "HeapMemory.h"
#include "Stack.h"
#include "StackMemory.h"
#include "StringPool.h"
#include "../common.h"

#define REGISTERS_COUNT 64
#define MAX_ARGS 128

struct StackFrame {
  uint8* stackBase = nullptr;
  uint64 allocatedSize = 0;
};

struct CallFrame {
  uint32 line = 0;
  std::string name;
  std::string filename;
};

class VirtualMachine {
  HeapMemory m_heap;
  StringPool m_stringPool;

  public:
    StringPool& getStringPool() const;
    HeapMemory& getHeap() const;
};

class InterpreterState {
  Stack<StackFrame> m_stackFrames;
  Stack<CallFrame> m_callStack;

  StackMemory m_stack;

  uint64 m_registers[REGISTERS_COUNT] = {};

  uint64 m_callArgs[MAX_ARGS] = {};
  uint64 m_argTypeIndexes[MAX_ARGS] = {};
  uint8 m_callArgCount = 0;

  std::vector<uint32> returnAddr;

  VirtualMachine& m_vm;

  public:
    explicit InterpreterState(VirtualMachine& vm);
    ~InterpreterState();
};

struct Instruction {
  uint16 opcode;
  uint8 args[LENGTH_ARGS];
};

struct CompiledScript {
  Instruction* instructions = nullptr;
  uint32 instructionsLen = 0;

  int32 entrypointIndex = -1;
};

#endif //QUICKSCRIPT_INTERPRETER_H
