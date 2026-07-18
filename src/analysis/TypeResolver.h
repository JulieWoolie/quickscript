#ifndef QUICKSCRIPT_TYPERESOLVER_H
#define QUICKSCRIPT_TYPERESOLVER_H

#include "../types.h"
#include "../parse/errors.h"
#include "../parse/syntaxtree.h"


class TypeResolver: public Visitor {
  TypeLookup* m_lookup;
  StringTable* m_strings;
  CompilerErrors* m_errors;

  public:
    explicit TypeResolver(TypeLookup *lookup, StringTable* strings, CompilerErrors* errors);

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
};


#endif //QUICKSCRIPT_TYPERESOLVER_H
