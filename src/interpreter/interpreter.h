#ifndef QUICKSCRIPT_INTERPRETER_H
#define QUICKSCRIPT_INTERPRETER_H

#include <string>
#include <vector>

#include "heap_mem.h"
#include "ir_file.h"
#include "opcodes.h"
#include "StackMemory.h"
#include "StringPool.h"
#include "../common.h"

#define REGISTERS_COUNT 64
#define MAX_ARGS 128
#define MAX_CALL_DEPTH 1024

struct Instruction {
  opcode code;
  uint8 args[LENGTH_ARGS];
};

static_assert(sizeof(Instruction) == LENGTH_INSTRUCTION);
static_assert(offsetof(Instruction, args) == LENGTH_OPCODE);

struct CallFrame {
  uint32 line = 0;

  std::string name;
  std::string filename;

  uint8* stackBase = nullptr;
  uint64 allocatedSize = 0;

  uint32 returnAddr = 0;
};

class InstructionBuf {
  uint8* m_buf = nullptr;
  uint64 m_cap = 0;
  uint64 m_len = 0;
  uint64 m_instrCount = 0;

  public:
    InstructionBuf();

    void insertInstructions(const uint8* instrBuf, uint64 len);

    void getInstruction(Instruction* out, uint32 instrIndex) const;

    uint64 length() const;

    uint64 capacity() const;

    uint8* getBuffer() const;

    uint64 getInstructionCount() const;
};

class GlobalMemorySpace {
  uint64 m_size = 0;
  uint8* m_data = nullptr;

  public:
    GlobalMemorySpace();

    void grow(uint64 bytes);

    uint8* getData() const;

    uint64 size() const;
};

class ScriptFunction {
  uint32 firstInstrIndex = 0;
  uint64 nameOffset = 0;
  FunctionSignature* signature = nullptr;
};

class VirtualMachine {
  HeapMemory m_heap;
  StringPool m_stringPool;
  TypeTable m_types;

  InstructionBuf m_instrBuf;
  GlobalMemorySpace m_globalMem;

  std::vector<ScriptFunction> m_functions;

  public:
    VirtualMachine();
    ~VirtualMachine();

    void addBytecodeFile(const BytecodeFile& file);

    TypeTable& getTypes();
    StringPool& getStringPool();
    HeapMemory& getHeap();
    InstructionBuf& getInstructions();
    GlobalMemorySpace& getGlobalMemory();
    std::vector<ScriptFunction>& getFunctions();
};

class InterpreterState {
  VirtualMachine& m_vm;

  StackMemory m_stack;

  uint64 m_registers[REGISTERS_COUNT] = {};
  uint64 m_argTypeIndexes[MAX_ARGS] = {};

  CallFrame m_callFrames[MAX_CALL_DEPTH];
  uint32 m_frameCount = 0;

  public:
    explicit InterpreterState(VirtualMachine& vm);
    ~InterpreterState();

    CallFrame* getCallFrame(uint32 off = 0);
    CallFrame* pushNewFrame();
    void popCallFrame();

    VirtualMachine& getVirtualMachine() const;
};

#endif //QUICKSCRIPT_INTERPRETER_H
