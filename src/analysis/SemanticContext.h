#ifndef QUICKSCRIPT_SEMANTICCONTEXT_H
#define QUICKSCRIPT_SEMANTICCONTEXT_H
#include "semantictree.h"
#include "../allocator.h"
#include "../compiler_opts.h"
#include "../errors.h"
#include "../nativeinterface.h"
#include "../stringtable.h"
#include "../parse/syntaxtree.h"
#include "../types/TypeTable.h"

class DependencyGraph {
  std::unordered_map<Symbol*, std::vector<Symbol*>> m_graph;

  public:
    std::vector<Symbol*>& getDependencies(Symbol* symb);

    bool hasDependencies(Symbol* sym) const;

    bool addDependency(Symbol* sym, Symbol* dependency);

    std::unordered_map<Symbol*, std::vector<Symbol*>>& getUnderlyingMap();
};

class SemanticContext {
  TypeTable& m_types;
  StringTable& m_strings;
  CompilerErrors& m_errors;
  NoFreeAllocator& m_allocator;
  const CompilationOptions& m_options;
  const BindingsObject* m_bindings;

  std::vector<ScriptType*> m_expectedTypes;
  std::vector<Statement*> m_statementStack;
  std::vector<bool> m_wrongScopeReports;

  std::vector<LocalFunction*> m_localFunctions;
  std::vector<LocalFunction*> m_mainCandidates;
  std::vector<LexicalDeclaration*> m_globalVars;
  std::vector<LexicalDeclaration*> m_allVars;

  std::vector<StructDecl*> m_declaredStructs;
  std::unordered_map<ScriptStructType*, LocalFuncSymbol*> m_structConstructors;

  //
  // Notes on what types of keys map to what types of values:
  //
  // Identifier* keys => Symbol the ID is referencing
  // PropertyAccessExpr* keys => PropertySymbol* being referenced
  // Statement* keys => Symbol the statement is declaring
  //
  std::unordered_map<Node*, Symbol*> m_symbolLookup;
  std::unordered_map<Symbol*, Scope*> m_scopeLookup;
  std::unordered_map<Node*, Scope*> m_astScopeLookup;

  DependencyGraph m_dependencyGraph;

  Scope* m_globalScope = nullptr;
  Scope* m_currentScope = nullptr;

  LocalFuncSymbol* m_entryPoint = nullptr;

  public:
    SemanticContext(
      TypeTable& types,
      StringTable& strings,
      CompilerErrors& errors,
      NoFreeAllocator& allocator,
      const CompilationOptions& options,
      const BindingsObject* bindings
    );

    void pushLocalFunction(LocalFunction* func);

    std::vector<LocalFunction*>& getLocalFunctions();

    std::vector<LocalFunction*>& getMainFuncCandidates();

    std::vector<LexicalDeclaration*>& getGlobalVariables();

    std::vector<LexicalDeclaration*>& getAllVariables();

    void pushWrongScopeTypeReported(bool reported);

    void popWrongScopeReported();

    bool wasWrongScopeReported() const;

    void pushExpectedType(ScriptType* type);

    ScriptType* getExpectedType() const;

    void popExpectedType();

    void pushStatement(Statement* stat);

    Statement* getCurrentStatement() const;

    std::vector<Statement*>& getStatementStack();

    void popStatement();

    void popScope();

    Scope* pushScope(scopetype stype, stringid label = EMPTY_STRING);

    Scope* getScope(uint32 off = 0) const;

    Scope* getGlobalScope() const;

    void setGlobalScope(Scope* scope);

    TypeTable& getTypes() const;

    StringTable& getStrings() const;

    CompilerErrors& getErrors() const;

    NoFreeAllocator& getAllocator();

    const CompilationOptions& getOptions() const;

    const BindingsObject* getBindings() const;

    std::unordered_map<Node*, Symbol*>& getSymbolLookup();

    std::unordered_map<Symbol*, Scope*>& getScopeLookup();

    std::unordered_map<Node*, Scope*>& getAstScopeLookup();

    std::unordered_map<ScriptStructType*, LocalFuncSymbol*>& getConstructors();

    DependencyGraph& getDependencyGraph();

    std::vector<StructDecl*>& getDeclaredStructs();

    LocalFuncSymbol* getEntryPoint() const;

    void setEntryPoint(LocalFuncSymbol* sym);
};


#endif //QUICKSCRIPT_SEMANTICCONTEXT_H
