#ifndef QUICKSCRIPT_JSONPRINTER_H
#define QUICKSCRIPT_JSONPRINTER_H
#include "syntaxtree.h"


class JsonPrinter: public Visitor {
  std::string m_result;

  public:
    JsonPrinter();

    std::string& getResult();

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
    void acceptArrayLiteral(ArrayLiteral* v) override;
    void acceptBinaryExpr(BinaryExpr* v) override;
    void acceptUnaryExpr(UnaryExpr* v) override;
    void acceptTernaryExpr(TernaryExpr* v) override;
    void acceptBlock(Block* v) override;
    void acceptIfStatement(IfStatement* v) override;
    void acceptForStatement(ForStatement* v) override;
    void acceptLexicalDeclaration(LexicalDeclaration* v) override;
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
    void acceptObjectAllocExpr(ObjectAllocExpr* v) override;
};


#endif //QUICKSCRIPT_JSONPRINTER_H
