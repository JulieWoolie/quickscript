#ifndef QUICKSCRIPT_TYPERESOLVER_H
#define QUICKSCRIPT_TYPERESOLVER_H

#include "../types/types.h"
#include "../interpreter/nativeinterface.h"
#include "../errors.h"
#include "../parse/syntaxtree.h"

#define SCOPE_NIL 0
#define SCOPE_MAIN 1
#define SCOPE_FUNCTION 2
#define SCOPE_LOOP 3
#define SCOPE_BLOCK 4
typedef uint8 scopetype;

#define SYM_NIL 0
#define SYM_VAR 1
#define SYM_CONST 2
#define SYM_FUNC 3
#define SYM_STRUCT 4
#define SYM_PROP 5
typedef uint8 symboltype;

#define SYMFLAG_BINDING 0x1

struct Symbol {
  stringid name = EMPTY_STRING;
  symboltype stype = SYM_NIL;
  uint32 flags = 0;
  uint64 offset = 0;

  ScriptType* scriptType = nullptr;

  uint32 readUses = 0;
  uint32 writeUses = 0;
};

class Scope {
  std::vector<Symbol> m_symbols;
  ScriptType* m_expectedReturnType = nullptr;

  const stringid m_currentLabel;
  const scopetype m_scopeType;

  uint64 m_size = 0;

  public:
    Scope(stringid label, scopetype type);

    Symbol* pushSymbol(stringid name, ScriptType* type, symboltype lexType = SYM_VAR, uint32 flags = 0);

    Symbol* findVariable(stringid name);

    Symbol* findSymbol(stringid name, symboltype st);

    std::vector<Symbol>* getSymbols();

    ScriptType* getExpectedReturnType() const;

    void setExpectedReturnType(ScriptType* type);

    stringid getLabel() const;

    scopetype getType() const;
};

class SemanticAnalyzer: public Visitor {
  TypeTable* m_lookup;
  StringTable* m_strings;
  CompilerErrors* m_errors;
  Bindings* m_bindings;

  std::vector<Scope> m_scopes;
  std::vector<ScriptType*> m_expectedTypes;
  std::vector<Statement*> m_statementStack;

  std::vector<TypeNameExpr*> m_failedTypeNames;

  std::vector<bool> m_wrongScopeReports;

  void popScope();

  Scope* pushScope(scopetype stype, stringid label = EMPTY_STRING);

  Scope* getScope(uint32 off = 0);

  ScriptType* getOpResultType(ScriptType* left, ScriptType* right, binaryop op) const;

  void acceptBodyNoScope(Statement* block);

  void createStructType(StructDecl* decl);
  void resolveMissingProperties(const StructDecl* decl);

  void createFuncSignature(FunctionDeclStatement* v);

  ScriptType* resolveTypeExpr(TypeExpr* v);

  ScriptType* getArrayType(ScriptType* componentType) const;

  void checkAssignability(Expr* expr);

  Symbol* resolveReferencedSymbol(stringid name, ScriptType* expectedType);

  bool everyBranchHasReturn(Statement* stat);

  void pushWrongScopeTypeReported(bool reported);

  void popWrongScopeReported();

  bool wasWrongScopeReported() const;

  public:
    explicit SemanticAnalyzer(
      TypeTable *lookup,
      StringTable* strings,
      CompilerErrors* errors,
      Bindings* bindings
    );

    void acceptTypeNameExpr(TypeNameExpr *v) override;
    void acceptArrayTypeExpr(ArrayTypeExpr *v) override;
    void acceptPrimitiveTypeExpr(PrimitiveTypeExpr *v) override;

    void acceptIdentifier(Identifier *v) override;
    void acceptCallExpr(CallExpr *v) override;
    void acceptPropertyAccessExpr(PropertyAccessExpr *v) override;
    void acceptIndexAccessExpr(IndexAccessExpr *v) override;
    void acceptBooleanLiteral(BooleanLiteral *v) override;
    void acceptCharLiteral(CharLiteral *v) override;
    void acceptStringLiteral(StringLiteral *v) override;
    void acceptIntLiteral(IntLiteral *v) override;
    void acceptFloatLiteral(FloatLiteral *v) override;
    void acceptObjectLiteral(ObjectLiteral* v) override;
    void acceptObjectLiteralProperty(ObjectLiteralProperty* v) override;
    void acceptArrayLiteral(ArrayLiteral* v) override;
    void acceptBinaryExpr(BinaryExpr *v) override;
    void acceptUnaryExpr(UnaryExpr *v) override;
    void acceptTernaryExpr(TernaryExpr *v) override;

    void acceptBlock(Block *v) override;
    void acceptIfStatement(IfStatement *v) override;
    void acceptForStatement(ForStatement *v) override;
    void acceptLexicalDeclaration(LexicalDeclaration *v) override;
    void acceptDoWhileStatement(DoWhileStatement *v) override;
    void acceptWhileStatement(WhileStatement *v) override;
    void acceptControlFlowStatement(ControlFlowStatement *v) override;
    void acceptReturnStatement(ReturnStatement *v) override;
    void acceptScriptFileStatement(ScriptFileStatement *v) override;
    void acceptFunctionParam(FunctionParam *v) override;
    void acceptFunctionDeclStatement(FunctionDeclStatement *v) override;
    void acceptExprStatement(ExprStatement *v) override;
    void acceptStructPropertyDecl(StructPropertyDecl *v) override;
    void acceptStructDecl(StructDecl *v) override;
    void acceptAssertStatement(AssertStatement* v) override;
};


#endif //QUICKSCRIPT_TYPERESOLVER_H
