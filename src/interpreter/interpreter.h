#ifndef QUICKSCRIPT_INTERPRETER_H
#define QUICKSCRIPT_INTERPRETER_H

#include "heap_mem.h"
#include "StackMemory.h"
#include "StringPool.h"
#include "../common.h"

#define REGISTERS_COUNT 64
#define MAX_ARGS 128
#define MAX_CALL_DEPTH 1024

struct Instruction {
  uint16 opcode;
  uint8 args[LENGTH_ARGS];
};

struct CallFrame {
  uint32 line = 0;

  std::string name;
  std::string filename;

  uint8* stackBase = nullptr;
  uint64 allocatedSize = 0;

  uint32 returnAddr = 0;
};

class InstructionBuf {
  Instruction* m_buf = nullptr;
  uint64 m_cap = 0;
  uint64 m_len = 0;

  public:
    InstructionBuf();


};

class GlobalMemorySpace {
  uint64 m_size = 0;
  uint8* m_data = nullptr;

  public:
    GlobalMemorySpace();


};

class VirtualMachine {
  HeapMemory m_heap;
  StringPool m_stringPool;

  InstructionBuf m_instrBuf;
  GlobalMemorySpace m_globalMem;

  public:
    VirtualMachine();

    StringPool& getStringPool() const;
    HeapMemory& getHeap() const;
    InstructionBuf& getInstructions() const;
    GlobalMemorySpace getGlobalMemory() const;
};

class InterpreterState {
  StackMemory m_stack;

  uint64 m_registers[REGISTERS_COUNT] = {};
  uint64 m_argTypeIndexes[MAX_ARGS] = {};

  VirtualMachine& m_vm;

  CallFrame m_callFrames[MAX_CALL_DEPTH];
  uint64 m_frameCount = 0;

  public:
    explicit InterpreterState(VirtualMachine& vm);
    ~InterpreterState();

    CallFrame* getCallFrame(uint32 off = 0);
    CallFrame* pushNewFrame();
    void popCallFrame();

    VirtualMachine& getVirtualMachine() const;
};

#endif //QUICKSCRIPT_INTERPRETER_H
