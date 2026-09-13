#ifndef QS_PARSER_H
#define QS_PARSER_H

#include "qs/allocator.hpp"
#include "qs/errors.hpp"
#include "qs/parse/syntaxtree.hpp"
#include "qs/parse/token.hpp"

// === Parser ===

#define LFDL_NONE 0
#define LFDL_LEX 1
#define LFDL_FUNC 2
#define LFDL_LABELLED_LOOP 3
#define LFDL_STRUCT 4
#define LFDL_MODULE 5

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

    void skip();

    bool hasNext() const;

    bool is(tokentype tt) const;

    Token* expect(tokentype tt);

    ScriptFileStatement* parse();

    declflags parseDeclFlags();

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

    WhileStatement* doWhileStatement(Identifier* label);

    WhileStatement* whileStatement(Identifier* label);

    LexicalDeclaration* lexDecl();

    ReturnStatement* returnStatement();

    StructDecl* structDecl();

    StructPropertyDecl* structProperty();

    AssertStatement* assertStatement();

    // Type expressions

    TypeExpr* typeExpr();

    TypeExpr* arrayType();

    TypeExpr* primaryTypeExpr();

    TypeExpr* typeName();

    PrimitiveTypeExpr* primitiveType();

    ModuleDeclaration* moduleDecl();

    ImportStatement* importStatement();

    stringid modulePath();

    // Expressions

    Expr* loopConditionExpr();

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
    ObjectLiteral* objectLiteral();
    ArrayLiteral* arrayLiteral();
    Identifier* id();

    stringid getDocComment() const;
};

#endif //QS_PARSER_H
