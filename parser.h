#ifndef QUICKSCRIPT_PARSER_H
#define QUICKSCRIPT_PARSER_H

#include "errors.h"
#include "syntaxtree.h"
#include "token.h"

// === Parser ===

class Parser {
  TokenList* m_tokens;
  NodePool* m_pool = nullptr;
  CompilerErrors* m_errors = nullptr;
  uint32 m_tokenCursor = 0;

  StringTable* m_nameTable;

  public:
    Parser(TokenList* tokens, NodePool* pool, CompilerErrors* errors, StringTable* table);

    Token* gettoken(uint32 idx) const;

    Token* next();
    Token* peek() const;
    Token* peek(uint32 ahead) const;

    void skip();

    bool hasNext() const;

    bool is(tokentype tt) const;
    bool is(uint32 ahead, tokentype tt) const;

    Token* expect(tokentype tt);

    NodeRef<ScriptFileStatement> parse();

    // statements

    NodeRef<Statement> statement();

    NodeRef<FunctionDeclStatement> funcDecl();

    NodeRef<Block> block();

    NodeRef<ControlFlowStatement> controlFlow();

    // Expressions

    NodeRef<Expr> expr();
    NodeRef<Expr> ternaryExpr();
    NodeRef<Expr> assignExpr();
    NodeRef<Expr> logicalOrExpr();
    NodeRef<Expr> logicalAndExpr();
    NodeRef<Expr> bitwiseOrExpr();
    NodeRef<Expr> bitwiseAndExpr();
    NodeRef<Expr> bitwiseXorExpr();
    NodeRef<Expr> equalityExpr();
    NodeRef<Expr> relationalExpr();
    NodeRef<Expr> shiftExpr();
    NodeRef<Expr> additiveExpr();
    NodeRef<Expr> multiplicativeExpr();
    NodeRef<Expr> exponentialExpr();
    NodeRef<Expr> unaryExpr();
    NodeRef<Expr> memberExpr(bool allowCall);
    NodeRef<Expr> memberExprTail(bool allowCall, NodeRef<Expr> target);
    NodeRef<Expr> propertyAccess(NodeRef<Expr> target);
    NodeRef<Expr> callExpr(NodeRef<Expr> target);
    NodeRef<Expr> primaryExpr();

    NodeRef<Expr> parenthesizedExpr();

    NodeRef<IntLiteral> intLiteral() const;
    NodeRef<BooleanLiteral> boolLiteral() const;
    NodeRef<FloatLiteral> floatLiteral();
    NodeRef<StringLiteral> stringLiteral();
    NodeRef<Identifier> id();
};

#endif //QUICKSCRIPT_PARSER_H
