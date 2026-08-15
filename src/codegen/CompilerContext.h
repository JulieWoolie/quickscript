#ifndef QUICKSCRIPT_COMPILERCONTEXT_H
#define QUICKSCRIPT_COMPILERCONTEXT_H

#include "../stringtable.h"
#include "../analysis/SemanticContext.h"
#include "../types/types.h"
#include "../interpreter/opcodes.h"
#include "../parse/syntaxtree.h"

#define ALL_REGISTERS_USED 0xFFFFFFFFFFFFFFFF
#define NO_REGISTER (-1)

typedef uint8 registerid;
typedef int8 registeridopt;

typedef uint64 StringPoolAddress;


class BytecodeWriter {
  uint8* m_buf = nullptr;
  uint64 m_cap = 0;
  uint64 m_len = 0;

  uint32 m_instrCount = 0;
  uint64 m_argsStart = 0;

  public:
    void reserveSpace(uint64 memsize);

    void startInstr(opcode code);

    void endInstr();

    void appendU8(uint8 u8);
    void appendI8(int8 i8);
    void appendU16(uint16 u16);
    void appendI16(int16 i16);
    void appendU32(uint32 u32);
    void appendI32(int32 i32);
    void appendU64(uint64 u64);
    void appendI64(int64 i64);
    void appendF32(float32 f32);
    void appendF64(float64 f64);

    void writeInstructionCounter(uint64 offset) const;

    void writeU32(uint64 offset, uint32 val);

    uint32 getInstructionCounter() const;

    uint64 getAddress() const;

    uint8* getBuffer() const;

    uint32 getLength() const;
};

class ConstStringPoolWriter {
  uint8* m_data = nullptr;
  uint64 m_cap = 0;
  uint64 m_len = 0;

  std::unordered_map<stringid, uint64> m_idToOffset;

  StringTable& m_table;

  public:
    explicit ConstStringPoolWriter(StringTable& table);

    StringPoolAddress emplace(stringid id);

    uint64 getLength() const;

    uint8* getData() const;
};

struct IncompleteFunctonCall {
  stringid name = EMPTY_STRING;
  FunctionSignature* signature = nullptr;
  uint64 writeOffset = 0;
};

struct CompiledFunction {
  stringid name = EMPTY_STRING;
  StringPoolAddress poolId = 0;
  uint32 bodyStart = 0;
  FunctionSignature* signature;
};

class CompilerContext {
  std::vector<ScriptType*> m_expectedTypes;

  std::unordered_map<ScriptStructType*, uint32> m_structConstructors;

  std::vector<CompiledFunction> m_compiledFuncs;
  std::vector<IncompleteFunctonCall> m_incompleteCalls;
  std::vector<FunctionDeclStatement*> m_funcQueue;

  ConstStringPoolWriter m_stringPool;
  BytecodeWriter m_writer;

  SemanticContext& m_semantics;

  uint64* m_registersInUse;

  Scope* m_currentScope = nullptr;

  public:
    CompilerContext(SemanticContext& ctx, uint64* registryBitset);

    void enqueueFunction(FunctionDeclStatement* stat);
    FunctionDeclStatement* pollQueuedFunction();
    int32 findFunctionIndex(stringid name, FunctionSignature* sign);

    uint32 getStructConstructorIndex(ScriptStructType* type);
    void pushStructConstructor(ScriptStructType* type, uint32 idx);

    void pushIncompleteCall(stringid name, FunctionSignature* sign, uint64 writeoffset);

    uint32 pushCompiledFunction(stringid name, uint32 start, FunctionSignature* sign);

    registeridopt acquireRegister() const;
    bool registerInUse(registerid reg) const;
    void useRegister(registerid reg) const;
    void freeRegister(registerid reg) const;

    BytecodeWriter& getWriter();
    ConstStringPoolWriter& getStringPool();

    SemanticContext& getSemantics() const;

    Scope* getCurrentScope() const;

    void setCurrentScope(Scope* scope);
};


#endif //QUICKSCRIPT_COMPILERCONTEXT_H
