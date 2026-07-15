#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "allocator.h"
#include "common.h"
#include "errors.h"
#include "lexer.h"
#include "syntaxtree.h"
#include "parser.h"

#define PRINTNODEBASE(name) printf(#name);printf("(");printloc(v->location);

struct PrintingVisitor: Visitor {
  private:
    int32 m_indent = 0;
    StringTable* m_table;

    void nli() {
      printf("\n");

      if (m_indent <= 0) {
        return;
      }

      for (uint32 i = 0; i < m_indent; i++) {
        printf("  ");
      }
    }
    void inc() {
      m_indent++;
    }
    void dec() {
      m_indent--;
    }

    void printstr(stringid id) {
      int32 len = m_table->getlen(id);
      char ch[len + 1];
      m_table->getchars(id, ch, len + 1);
      printf(ch);
    }
    void printloc(Location l) {
      printf("location=%i::%i:%i", l.index, l.line, l.column);
    }

  public:
    PrintingVisitor(StringTable* table) {
      m_table = table;
    }

    void acceptTypeNameExpr(TypeNameExpr *v) override {

    }
    void acceptArrayTypeExpr(ArrayTypeExpr *v) override {

    }
    void acceptPrimitiveTypeExpr(PrimitiveTypeExpr *v) override {

    }

    void acceptIdentifier(Identifier *v) override {
      PRINTNODEBASE(Identifier)
      printf(" value='");
      printstr(v->value);
      printf("')");
    }
    void acceptCallExpr(CallExpr *v) override {
      PRINTNODEBASE(CallExpr);
      printf(") {");

      inc();
      nli();

      printf("target = ");
      v->target->acceptVisit(this);

      nli();
      printf("arguments = [");

      if (!v->arguments.empty()) {
        inc();
        for (uint32 i = 0; i < v->arguments.size(); i++) {
          nli();
          printf("[%i] = ", i);
          v->arguments.at(i)->acceptVisit(this);
        }
        dec();
        nli();
      }

      printf("]");

      dec();
      nli();
      printf("}");
    }
    void acceptPropertyAccessExpr(PropertyAccessExpr *v) override {
      PRINTNODEBASE(PropertyAccessExpr)
      printf(") {");
      inc();

      nli();
      printf("property = ");
      v->property->acceptVisit(this);

      nli();
      printf("target = ");
      v->target->acceptVisit(this);

      dec();
      nli();
      printf("}");
    }
    void acceptBooleanLiteral(BooleanLiteral *v) override {
      PRINTNODEBASE(BooleanLiteral)
      printf(" value=");
      if (v->value) {
        printf("true");
      } else {
        printf("false");
      }
      printf(")");
    }
    void acceptCharLiteral(CharLiteral *v) override {
      PRINTNODEBASE(CharLiteral)
      printf(" value='");
      printstr(v->value);
      printf("')");
    }
    void acceptStringLiteral(StringLiteral *v) override {
      PRINTNODEBASE(StringLiteral)
      printf(" value=\"");
      printstr(v->value);
      printf("\")");
    }
    void acceptIntLiteral(IntLiteral *v) override {
      PRINTNODEBASE(IntLiteral)
      printf(" value=%llu)", v->value);
    }
    void acceptFloatLiteral(FloatLiteral *v) override {
      PRINTNODEBASE(FloatLiteral)
      printf(" value=%f)", v->value);
    }
    void acceptBinaryExpr(BinaryExpr *v) override {
      PRINTNODEBASE(BinaryExpr)
      printf(" op=%s) {", binaryop_name(v->op));

      inc();
      nli();
      printf("lhs = ");
      v->lhs->acceptVisit(this);

      nli();
      printf("rhs = ");
      v->rhs->acceptVisit(this);

      dec();
      nli();
      printf("}");
    }
    void acceptUnaryExpr(UnaryExpr *v) override {
      PRINTNODEBASE(UnaryExpr)
      printf("op = %s) {", unaryop_name(v->op));
      inc();
      nli();
      printf("target = ");
      v->target->acceptVisit(this);
      dec();
      nli();
      printf("}");
    }
    void acceptTernaryExpr(TernaryExpr *v) override {
      PRINTNODEBASE(TernaryExpr)
      printf(") {");
      inc();
      nli();
      printf("condition = ");
      v->condition->acceptVisit(this);

      nli();
      printf("left = ");
      v->left->acceptVisit(this);

      nli();
      printf("right = ");
      v->right->acceptVisit(this);

      dec();
      nli();
      printf("}");
    }
    void acceptBlock(Block *v) override {

    }
    void acceptIfStatement(IfStatement *v) override {

    }
    void acceptForStatement(ForStatement *v) override {

    }
    void acceptLexicalDeclaration(LexicalDeclaration *v) override {

    }
    void acceptDoWhileStatement(DoWhileStatement *v) override {

    }
    void acceptWhileStatement(WhileStatement *v) override {

    }
    void acceptControlFlowStatement(ControlFlowStatement *v) override {

    }
    void acceptReturnStatement(ReturnStatement *v) override {

    }
    void acceptScriptFileStatement(ScriptFileStatement *v) override {

    }
    void acceptFunctionParam(FunctionParam *v) override {

    }
    void acceptFunctionDeclStatement(FunctionDeclStatement *v) override {

    }
};

int32 main(int32 argc, cstring argv[]) {
  if (argc < 2) {
    printf("Enter file name to execute!\n");
    return EXIT_FAILURE;
  }

  cstring fname = argv[argc - 1];
  std::ifstream file(fname);

  if (!file) {
    printf("File '%s' doesn't exist or can't be read.\n", fname);
    return EXIT_FAILURE;
  }

  std::string file_contents { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };

  TokenList tokens = TokenList();
  StringTable table = StringTable();

  Lexer l = Lexer(file_contents, &tokens, &table);
  l.lex();

  CompilerErrors errors = CompilerErrors(&file_contents, fname);
  NoFreeAllocator pool = NoFreeAllocator();

  Parser p = Parser(&tokens, &pool, &errors, &table);

  Expr* expr = p.expr();
  PrintingVisitor pv = PrintingVisitor(&table);

  expr->acceptVisit(&pv);

  return EXIT_SUCCESS;
}

