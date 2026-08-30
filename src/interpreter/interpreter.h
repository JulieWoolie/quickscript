#ifndef QUICKSCRIPT_INTERPRETER_H
#define QUICKSCRIPT_INTERPRETER_H

#include <string>
#include <vector>

#include "heap_mem.h"
#include "../bytecode/bytecode_file.h"
#include "opcodes.h"
#include "StackMemory.h"
#include "StringPool.h"
#include "../args.h"
#include "../common.h"
#include "registers.h"
#include "../nativeinterface.h"

#define MAX_ARGS 128
#define MAX_CALL_DEPTH 1024
#define NO_RETURN_ADDR 0xFFFFFFFF

#define LEFT_GT_RIGHT 1
#define LEFT_EQ_RIGHT 0
#define LEFT_LT_RIGHT (-1)

struct CallFrame {
  uint32 line = 0;

  std::string name;
  std::string filename;

  uint8* stackBase = nullptr;
  uint64 stackFrameSize = 0;
  uint64 allocatedSize = 0;

  uint32 returnAddr = NO_RETURN_ADDR;
};

class InstructionBuf {
  uint8* m_buf = nullptr;
  uint64 m_cap = 0;
  uint64 m_len = 0;
  uint64 m_instrCount = 0;

  public:
    InstructionBuf();
    ~InstructionBuf();

    void insertInstructions(const uint8* instrBuf, uint64 len);

    void getInstruction(opcode* code, uint8 args[], uint32 instrIndex) const;

    uint64 length() const;

    uint64 capacity() const;

    uint8* getBuffer() const;

    uint64 getInstructionCount() const;

    void freeBuffer();
};

class GlobalMemorySpace {
  uint64 m_size = 0;
  uint8* m_data = nullptr;

  public:
    GlobalMemorySpace();
    ~GlobalMemorySpace();

    void grow(uint64 bytes);

    uint8* getData() const;

    uint64 size() const;

    void free();
};

#define FUNCTYPE_LOCAL 0
#define FUNCTYPE_NATIVE 1
typedef uint8 functype;

struct ScriptFunction {
  FunctionSignature* signature = nullptr;

  virtual ~ScriptFunction() = default;
  virtual functype ftype() const = 0;
};

struct LocalScriptFunction: ScriptFunction {
  std::string filename = "";
  uint32 firstInstrIndex = 0;
  uint64 nameOffset = 0;
  uint64 stackSize = 0;

  functype ftype() const override;
};

struct NativeScriptFunction: ScriptFunction {
  std::string name = "";
  NativeFunction callback = nullptr;

  functype ftype() const override;
};

class VirtualMachine {
  HeapMemory m_heap;
  StringPool m_stringPool;
  TypeTable m_types;

  InstructionBuf m_instrBuf;
  GlobalMemorySpace m_globalMem;

  std::vector<NativeScriptFunction> m_nativeFunctions;
  std::vector<LocalScriptFunction> m_functions;

  public:
    VirtualMachine();
    ~VirtualMachine();

    void addBindings(const BindingsObject* object);

    uint32 addBytecodeFile(const BytecodeFile& file, const std::string& filename);

    int32 beginExecution(uint32 funcEntryIdx, const ProgramArgs& args);

    void toString(std::string& out, typeindex type, uint64 value);

    bool objectEquals(uint64 leftPtr, uint64 rightPtr, const ScriptStructType* structType);

    bool arrayEquals(uint64 leftPtr, uint64 rightPtr, const ScriptArrayType* arrayType);

    bool stringEquals(uint64 leftAddr, uint64 rightAddr);

    bool equals(uint64 left, uint64 right, typeindex idx);

    int8 compareString(uint64 leftPtr, uint64 rightPtr);

    int8 compareArray(uint64 leftPtr, uint64 rightPtr, const ScriptArrayType* arrType);

    NativeScriptFunction* lookupNativeFunction(const FunctionSignature* sign, const std::string& name);

    TypeTable& getTypes();
    StringPool& getStringPool();
    HeapMemory& getHeap();
    InstructionBuf& getInstructions();
    GlobalMemorySpace& getGlobalMemory();
    std::vector<LocalScriptFunction>& getFunctions();
    std::vector<NativeScriptFunction>& getNativeFunctions();
};

class Interpreter {
  VirtualMachine& m_vm;

  StackMemory m_stack;

  RegisterValue m_registers[REGISTER_COUNT] = {};
  typeindex m_argTypeIndexes[MAX_ARGS] = {};

  CallFrame m_callFrames[MAX_CALL_DEPTH];
  uint32 m_frameCount = 0;

  void throwScriptError(const std::string& message);

  public:
    explicit Interpreter(VirtualMachine& vm);
    ~Interpreter();

    CallFrame* getCallFrame(uint32 off = 0);
    CallFrame* pushNewFrame();
    void popCallFrame();

    VirtualMachine& getVirtualMachine() const;

    uint64 strConcat(QsArray& lString, uint64 rightObj, typeindex rType);

    void moveExecutionTo(const LocalScriptFunction& func);

    int32 beginExecution(const LocalScriptFunction& func, uint64 argsArrayAddr);

    void run();

  private:
    bool doArrayEqualityCheck(uint8 r1, uint8 r2, uint32 ti) const;
    bool doStructEqualityCheck(uint8 r1, uint8 r2, uint32 ti) const;

    int8 doArrayComparison(uint8 lhs, uint8 rhs, uint32 ti) const;

    void callNativeFunction(NativeScriptFunction* nFunc);
};

#endif //QUICKSCRIPT_INTERPRETER_H
