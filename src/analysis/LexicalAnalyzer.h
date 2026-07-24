#ifndef QUICKSCRIPT_LEXICALANALYZER_H
#define QUICKSCRIPT_LEXICALANALYZER_H

#include "../interpreter/nativeinterface.h"
#include "../errors.h"
#include "../parse/syntaxtree.h"

//
// Goals:
//
// This analyzer should server to both analyze any non-type related errors that weren't reported by
// the TypeResolver and warn of anything that doesn't constitute an error.
//
// This analyzer should also be executed after the type resolver has finished as it expects
// type information to be filled out already.
//
// Warn for:
//  - Unused symbols
//
// Errors for:
//   - Incorrect contexts (using non-variable statements in main scope)
//   - continue/break statements with unknown label
//   - duplicate struct declarations
//   - duplicate variable declarations
//   - duplicate function declarations
//

#define SCOPE_NIL 0
#define SCOPE_MAIN 1
#define SCOPE_FUNCTION 2
#define SCOPE_FORLOOP 3
#define SCOPE_DOWHILE 4
#define SCOPE_WHILE 5

typedef uint8 scopetype;

#define SYM_NIL 0
#define SYM_VAR 1
#define SYM_CONST 2
#define SYM_FUNC 3
#define SYM_STRUCT 4
#define SYM_PROP 5

typedef uint8 symboltype;

struct Symbol {
  stringid name = EMPTY_STRING;
  symboltype type = SYM_NIL;
  uint32 uses = 0;
  ScriptType* scriptType = nullptr;
};

struct AnalyzerScope {
  std::vector<Symbol> symbols = {};
  scopetype stype = SCOPE_NIL;
  stringid currentLabel = EMPTY_STRING;

  Symbol* pushSymbol(stringid name, symboltype type);

  Symbol* getSymbol(stringid name, symboltype symType);
};

class LexicalAnalyzer: public Visitor {
  StringTable* m_strings;
  CompilerErrors* m_errors;
  Bindings* m_bindings;

  std::vector<AnalyzerScope> m_scopes;

  AnalyzerScope* pushScope(scopetype type);

  void popScope();

  AnalyzerScope* getScope(uint32 back = 0);

  Symbol* getSymbol(stringid name, symboltype stype);

  public:
    LexicalAnalyzer(StringTable* strings, CompilerErrors* errors, Bindings* bindings);

    void acceptTypeNameExpr(TypeNameExpr* v) override;
    void acceptArrayTypeExpr(ArrayTypeExpr* v) override;
    void acceptPrimitiveTypeExpr(PrimitiveTypeExpr* v) override;

    void acceptIdentifier(Identifier* v) override;
    void acceptCallExpr(CallExpr* v) override;
    void acceptPropertyAccessExpr(PropertyAccessExpr* v) override;
    void acceptIndexAccessExpr(IndexAccessExpr* v) override;
    void acceptBooleanLiteral(BooleanLiteral* v) override;
    void acceptCharLiteral(CharLiteral* v) override;
    void acceptStringLiteral(StringLiteral* v) override;
    void acceptIntLiteral(IntLiteral* v) override;
    void acceptFloatLiteral(FloatLiteral* v) override;
    void acceptObjectLiteral(ObjectLiteral* v) override;
    void acceptObjectLiteralProperty(ObjectLiteralProperty* v) override;
    void acceptBinaryExpr(BinaryExpr* v) override;
    void acceptUnaryExpr(UnaryExpr* v) override;
    void acceptTernaryExpr(TernaryExpr* v) override;

    void acceptBlock(Block* v) override;
    void acceptIfStatement(IfStatement* v) override;
    void acceptForStatement(ForStatement* v) override;
    void acceptLexicalDeclaration(LexicalDeclaration* v) override;
    void acceptDoWhileStatement(DoWhileStatement* v) override;
    void acceptWhileStatement(WhileStatement* v) override;
    void acceptControlFlowStatement(ControlFlowStatement* v) override;
    void acceptReturnStatement(ReturnStatement* v) override;
    void acceptScriptFileStatement(ScriptFileStatement* v) override;
    void acceptFunctionParam(FunctionParam* v) override;
    void acceptFunctionDeclStatement(FunctionDeclStatement* v) override;
    void acceptExprStatement(ExprStatement* v) override;
    void acceptStructPropertyDecl(StructPropertyDecl* v) override;
    void acceptStructDecl(StructDecl* v) override;
    void acceptAssertStatement(AssertStatement* v) override;
};


#endif //QUICKSCRIPT_LEXICALANALYZER_H
