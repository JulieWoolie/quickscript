#ifndef QUICKSCRIPT_PARSER_H
#define QUICKSCRIPT_PARSER_H

#include "allocator.h"
#include "errors.h"
#include "syntaxtree.h"
#include "token.h"

// === Parser ===

#define LFDL_NONE 0
#define LFDL_LEX 1
#define LFDL_FUNC 2
#define LFDL_LABELLED_LOOP 3

class Parser {
  TokenList* m_tokens;
  NoFreeAllocator* m_pool = nullptr;
  CompilerErrors* m_errors = nullptr;
  uint32 m_tokenCursor = 0;

  StringTable* m_nameTable;

  public:
    Parser(TokenList* tokens, NoFreeAllocator* pool, CompilerErrors* errors, StringTable* table);

    Token* gettoken(uint32 idx) const;

    Token* next();
    Token* peek() const;
    Token* peek(uint32 ahead) const;

    void skip();

    bool hasNext() const;

    bool is(tokentype tt) const;
    bool is(uint32 ahead, tokentype tt) const;

    Token* expect(tokentype tt);

    ScriptFileStatement* parse();

    // statements

    uint8 isLexOrFuncDecl();

    Statement* statement();

    FunctionParam* funcParam();

    FunctionDeclStatement* funcDecl();

    Block* block();

    ControlFlowStatement* controlFlow();

    IfStatement* ifStatement();

    Statement* labelledStatement();

    ForStatement* forStatement(Identifier* label);

    DoWhileStatement* doWhileStatement(Identifier* label);

    WhileStatement* whileStatement(Identifier* label);

    LexicalDeclaration* lexDecl();

    ReturnStatement* returnStatement();

    StructDecl* structDecl();

    StructPropertyDecl* structProperty();

    // Type expressions

    TypeExpr* typeExpr();

    TypeExpr* arrayType();

    TypeExpr* primaryTypeExpr();

    TypeExpr* typeName();

    PrimitiveTypeExpr* primitiveType();

    // Expressions

    Expr* expr();
    Expr* ternaryExpr();
    Expr* assignExpr();
    Expr* logicalOrExpr();
    Expr* logicalAndExpr();
    Expr* bitwiseOrExpr();
    Expr* bitwiseAndExpr();
    Expr* bitwiseXorExpr();
    Expr* equalityExpr();
    Expr* relationalExpr();
    Expr* shiftExpr();
    Expr* additiveExpr();
    Expr* multiplicativeExpr();
    Expr* exponentialExpr();
    Expr* unaryExpr();
    Expr* memberExpr(bool allowCall);
    Expr* memberExprTail(bool allowCall, Expr *target);
    Expr* propertyAccess(Expr* target);
    Expr* indexAccess(Expr* target);
    Expr* callExpr(Expr* target);
    Expr* primaryExpr();

    Expr* parenthesizedExpr();

    CharLiteral* charLiteral();
    IntLiteral* intLiteral();
    BooleanLiteral* boolLiteral();
    FloatLiteral* floatLiteral();
    StringLiteral* stringLiteral();
    Identifier* id();
};

#endif //QUICKSCRIPT_PARSER_H
