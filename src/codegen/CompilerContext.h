#ifndef QUICKSCRIPT_COMPILERCONTEXT_H
#define QUICKSCRIPT_COMPILERCONTEXT_H

#include "../stringtable.h"
#include "../analysis/SemanticContext.h"
#include "../interpreter/opcodes.h"
#include "../interpreter/registers.h"
#include "../parse/syntaxtree.h"

#define ALL_REGISTERS_USED 0xFFFFFFFFFFFFFFFF
#define NO_REGISTER (-1)

typedef uint64 StringPoolAddress;

class BytecodeWriter {
  uint8* m_buf = nullptr;
  uint64 m_cap = 0;
  uint64 m_len = 0;

  uint32 m_instrCount = 0;
  uint64 m_argsStart = 0;
  bool m_instrStarted = false;

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

    void writeInstructionCounter(uint64 offset, uint32 instrCount) const;

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

  StringPoolAddress emplaceData(conststring data, uint32 len);

  public:
    explicit ConstStringPoolWriter(StringTable& table);

    StringPoolAddress emplace(stringid id);

    StringPoolAddress emplaceString(conststring data, uint32 len);

    uint64 getLength() const;

    uint8* getData() const;
};

struct IncompleteFunctonCall {
  LocalFuncSymbol* functionSymbol = nullptr;
  uint64 writeOffset = 0;
};

struct CompiledFunction {
  LocalFuncSymbol* functionSymbol = nullptr;
  StringPoolAddress poolId = 0;
  uint32 bodyStart = 0;
};

struct ControlFlowCall {
  controlflowtype type = CFT_NIL;
  stringid label = nullptr;
  uint64 writeAddress = 0;
};

class TypeReferenceCounter {
  uint32 m_size = 0;
  uint32* m_counters = nullptr;

  public:
    explicit TypeReferenceCounter(uint32 size);

    void incrementCounter(typeindex index) const;

    uint32 getReferenceCount(typeindex index) const;

    uint32 getReferencedNonConstTypes() const;
};

class CompilerContext {
  std::vector<ScriptType*> m_expectedTypes;

  std::vector<CompiledFunction> m_compiledFuncs;
  std::vector<IncompleteFunctonCall> m_incompleteCalls;
  std::vector<FunctionDeclStatement*> m_funcQueue;

  std::vector<ControlFlowCall> m_controlFlowCalls;

  ConstStringPoolWriter m_stringPool;
  BytecodeWriter m_writer;
  TypeReferenceCounter m_typeRefCounter;

  SemanticContext& m_semantics;

  RegisterBitSet* m_registersInUse;

  Scope* m_currentScope = nullptr;
  bool m_returned = false;

  public:
    CompilerContext(SemanticContext& ctx, RegisterBitSet* registryBitset);

    void enqueueFunction(FunctionDeclStatement* stat);
    FunctionDeclStatement* pollQueuedFunction();
    int32 findFunctionIndex(LocalFuncSymbol* sym) const;

    void pushIncompleteCall(LocalFuncSymbol* lfs, uint64 writeoffset);

    uint32 pushCompiledFunction(LocalFuncSymbol* lfs, uint32 start);

    RegisterIdOpt acquireRegister() const;
    bool registerInUse(RegisterId reg) const;
    void useRegister(RegisterId reg) const;
    void freeRegister(RegisterId reg) const;

    BytecodeWriter& getWriter();
    ConstStringPoolWriter& getStringPool();
    TypeReferenceCounter& getTypeRefCounter();

    SemanticContext& getSemantics() const;

    Scope* getCurrentScope() const;

    void setCurrentScope(Scope* scope);

    std::vector<ControlFlowCall>& getControlFlowCalls();

    std::vector<CompiledFunction>& getCompiledFunctions();

    bool wasReturnCalled() const;
    void setReturnCalled(bool b);

    void countTypeReference(const ScriptType* type) const;
};


#endif //QUICKSCRIPT_COMPILERCONTEXT_H
