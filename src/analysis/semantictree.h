#ifndef QUICKSCRIPT_SEMANTICTREE_H
#define QUICKSCRIPT_SEMANTICTREE_H

#include <vector>

#include "../stringtable.h"
#include "../interpreter/nativeinterface.h"
#include "../parse/syntaxtree.h"
#include "../parse/token.h"
#include "../types/types.h"

#define PRINTVIEW(x) static_cast<int>(x.length()), x.data()

/*
Source code
   V Given to
Parser
   V Parses into
Syntax Tree
   V Given to
Semantic Analyzer
   V Creates
Semantic Tree (semantic stuff + AST)
   V Given to
Transformer
       - Flatten all function definitions so f() { a() {} }
         Becomes f(){} and f#a(){}
       - Applies optimizations (inlining constants and evaluating const expressions)
       - Create struct constructors
       - (Future) Compile struct method calls to flat
         functions TYPE#METHOD_NAME
   V Give optimized data to
Compiler
   V Compiles into
IR byte code
   V Given to
Interpreter
   V Executes
Result
*/

#define SCOPE_NIL         0
#define SCOPE_MAIN        1
#define SCOPE_FUNCTION    2
#define SCOPE_LOOP        3
#define SCOPE_BLOCK       4
typedef uint8 scopetype;

#define SYM_NIL           0
#define SYM_LocalVar      1
#define SYM_LocalFunc     2
#define SYM_Property      3
#define SYM_LocalStruct   4
typedef uint8 symboltype;

// Symbol is a native binding
#define SYMFLAG_BINDING   (0x1 << 0)

// Symbol is a constant variable
#define SYMFLAG_CONST     (0x1 << 1)

// Symbol can be evaluated by the compiler and can be optimized
#define SYMFLAG_CONSTEXPR (0x1 << 2)

// Symbol is a pure function with no side effects
#define SYMFLAG_PUREFUNC  (0x1 << 3)

// Symbol has been used (read or written to at least once)
#define SYMFLAG_USED      (0x1 << 4)

// Symbol is a variable that's in the middle of being initialised
#define SYMFLAG_MID_INIT  (0x1 << 6)

// Symbol is a local variable that's a function argument
#define SYMFLAG_FUNC_ARG  (0x1 << 7)

typedef uint32 symflags;

class LocalFunction;
class Scope;

class Symbol {
  const stringid m_name;
  ScriptType* const m_type;

  symflags m_flags;

  public:
    Symbol(stringid name, ScriptType* scriptType);
    virtual ~Symbol() = default;

    symflags getFlags() const;

    void setFlags(symflags f);

    void addFlags(symflags flags);

    void removeFlags(symflags flags);

    stringid getName() const;

    ScriptType* getScriptType() const;

    virtual symboltype stype() const = 0;
};

class LocalFuncSymbol: public Symbol {
  LocalFunction& m_function;
  uint32 m_calls;

  public:
    explicit LocalFuncSymbol(LocalFunction& func);

    LocalFunction& getFunction() const;

    uint32 getCalls() const;

    void onCalled();

    symboltype stype() const override;
};

class LocalVarSymbol: public Symbol {
  const uint64 m_size;

  uint64 m_offset;

  std::vector<Location> m_reads;
  std::vector<Location> m_writes;

  Statement* const m_decl;

  public:
    LocalVarSymbol(stringid name, ScriptType* type, uint64 size, uint64 off, Statement* decl);

    Statement* getDecl() const;

    uint64 getStackSize() const;
    uint64 getStackOffset() const;

    void setStackOffset(uint64 off);

    std::vector<Location>& getReads();
    std::vector<Location>& getWrites();

    symboltype stype() const override;
};

class PropertySymbol: public Symbol {
  ScriptType* const m_holderType;

  uint32 m_writes = 0;
  uint32 m_reads = 0;

  public:
    explicit PropertySymbol(ScriptType* holderType, stringid name, ScriptType* type);

    ScriptType* getHolderType() const;

    symboltype stype() const override;

    uint32 getReads() const;
    void setReads(uint32 reads);

    uint32 getWrites() const;
    void setWrites(uint32 writes);
};

class LocalStructSymbol: public Symbol {
  StructDecl* m_decl;
  uint32 m_uses;

  public:
    LocalStructSymbol(stringid name, ScriptStructType* type, StructDecl* decl);

    StructDecl* getDecl() const;

    symboltype stype() const override;

    uint32 getUses() const;

    void setUses(uint32 uses);

    void used();
};

class LocalFunction {
  const stringid m_name;
  FunctionDeclStatement* const m_decl;
  FunctionSignature* m_signature;
  Scope* const m_bodyScope;

  bool m_nested = false;

  public:
    LocalFunction(stringid name, FunctionDeclStatement* decl, Scope* scope);

    stringid getName() const;

    FunctionDeclStatement* getDecl() const;

    Scope* getScope() const;

    FunctionSignature* getSignature() const;

    bool isNested() const;

    void setNested(bool b);
};

class Scope {
  const scopetype m_type;
  Scope* const m_parentScope;

  std::vector<Symbol*> m_symbols;

  stringid m_loopLabel = EMPTY_STRING;
  uint64 m_stackSize;
  ScriptType* m_expectedReturnType = nullptr;

  public:
    Scope(scopetype type, Scope* parent);

    std::vector<Symbol*>& getSymbols();

    void pushSymbol(Symbol* sym);

    void pushSymbol(const Symbol* before, Symbol* sym);

    void removeSymbol(Symbol* sym);

    Symbol* findVariable(stringid name) const;

    Symbol* findSymbol(stringid name, symboltype st) const;

    scopetype getType() const;

    Scope* getParent() const;

    stringid getLoopLabel() const;
    void setLoopLabel(stringid loopLabel);

    uint64 getStackSize() const;
    void setStackSize(uint64 size);

    ScriptType* getExpectedReturnType() const;
    void setExpectedReturnType(ScriptType* type);
};

#endif //QUICKSCRIPT_SEMANTICTREE_H
