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

#define PRINTNODEBASE printf("%s(", v->nodeType());printloc(v->location);
#define OBJBEGIN printf(" {");inc();
#define OBJEND dec();nli();printf("}");
#define OBJPROP(name, v) nli();printf("%s = ", name);v->acceptVisit(this);
#define OBJPROPO(name, v) if (v) { nli();printf("%s = ", name);v->acceptVisit(this); }

struct PrintingVisitor: Visitor {
  private:
    int32 m_indent = 0;
    StringTable* m_table;
    conststring m_filename;

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
    PrintingVisitor(StringTable* table, conststring filename) {
      m_table = table;
      m_filename = filename;
    }

    void acceptTypeNameExpr(TypeNameExpr *v) override {
      PRINTNODEBASE
      printf(" type-name='");
      printstr(v->typeName);
      printf(")");
    }
    void acceptArrayTypeExpr(ArrayTypeExpr *v) override {
      PRINTNODEBASE
      printf(")");
      OBJBEGIN
      OBJPROP("component-type", v->componentType)
      OBJEND
    }
    void acceptPrimitiveTypeExpr(PrimitiveTypeExpr *v) override {
      PRINTNODEBASE
      printf(" type=%s)", primitivetype_name(v->primType));
    }

    void acceptIdentifier(Identifier *v) override {
      PRINTNODEBASE
      printf(" value='");
      printstr(v->value);
      printf("')");
    }
    void acceptCallExpr(CallExpr *v) override {
      PRINTNODEBASE;
      printf(")");
      OBJBEGIN
      OBJPROP("target", v->target)

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

      OBJEND
    }
    void acceptPropertyAccessExpr(PropertyAccessExpr *v) override {
      PRINTNODEBASE
      printf(")");
      OBJBEGIN
      OBJPROP("property", v->property)
      OBJPROP("target", v->target)
      OBJEND
    }
    void acceptBooleanLiteral(BooleanLiteral *v) override {
      PRINTNODEBASE
      printf(" value=");
      if (v->value) {
        printf("true");
      } else {
        printf("false");
      }
      printf(")");
    }
    void acceptCharLiteral(CharLiteral *v) override {
      PRINTNODEBASE
      printf(" value='");
      printstr(v->value);
      printf("')");
    }
    void acceptStringLiteral(StringLiteral *v) override {
      PRINTNODEBASE
      printf(" value=\"");
      printstr(v->value);
      printf("\")");
    }
    void acceptIntLiteral(IntLiteral *v) override {
      PRINTNODEBASE
      printf(" value=%llu)", v->value);
    }
    void acceptFloatLiteral(FloatLiteral *v) override {
      PRINTNODEBASE
      printf(" value=%f)", v->value);
    }
    void acceptBinaryExpr(BinaryExpr *v) override {
      PRINTNODEBASE
      printf(" op=%s)", binaryop_name(v->op));
      OBJBEGIN
      OBJPROP("lhs", v->lhs)
      OBJPROP("rhs", v->rhs)
      OBJEND
    }
    void acceptUnaryExpr(UnaryExpr *v) override {
      PRINTNODEBASE
      printf("op = %s)", unaryop_name(v->op));
      OBJBEGIN
      OBJPROP("target", v->target)
      OBJEND
    }
    void acceptTernaryExpr(TernaryExpr *v) override {
      PRINTNODEBASE
      printf(")");
      OBJBEGIN
      OBJPROP("condition", v->condition)
      OBJPROP("left", v->left)
      OBJPROP("right", v->right)
      OBJEND
    }
    void acceptBlock(Block *v) override {
      PRINTNODEBASE
      printf(") [");

      if (v->statements.empty()) {
        printf("]");
        return;
      }

      inc();

      for (uint32 i = 0; i < v->statements.size(); i++) {
        Statement* s = v->statements.at(i);
        nli();
        printf("[%i] = ", i);
        s->acceptVisit(this);
      }

      dec();
      nli();
      printf("]");
    }
    void acceptIfStatement(IfStatement *v) override {
      PRINTNODEBASE
      printf(")");
      OBJBEGIN
      OBJPROP("condition", v->condition)
      OBJPROP("body", v->body)
      OBJPROPO("else", v->body)
      OBJEND
    }
    void acceptForStatement(ForStatement *v) override {
      PRINTNODEBASE
      printf(")");
      OBJBEGIN
      OBJPROPO("label", v->label)
      OBJPROP("first", v->first)
      OBJPROP("second", v->second)
      OBJPROP("third", v->third)
      OBJPROP("body", v->loopBody)
      OBJEND
    }
    void acceptLexicalDeclaration(LexicalDeclaration *v) override {
      PRINTNODEBASE
      printf(" const=");
      if (v->isConstDeclaration) {
        printf("true");
      } else {
        printf("false");
      }

      printf(")");
      OBJBEGIN
      OBJPROP("type", v->typeExpr)
      OBJPROP("variable-name", v->variableName)
      OBJPROPO("value", v->value)
      OBJEND
    }
    void acceptDoWhileStatement(DoWhileStatement *v) override {
      PRINTNODEBASE
      printf(")");
      OBJBEGIN
      OBJPROPO("label", v->label)
      OBJPROP("condition", v->condition)
      OBJPROP("body", v->body)
      OBJEND
    }
    void acceptWhileStatement(WhileStatement *v) override {
      PRINTNODEBASE
      printf(")");
      OBJBEGIN
      OBJPROPO("label", v->label)
      OBJPROP("condition", v->condition)
      OBJPROP("body", v->body)
      OBJEND
    }
    void acceptControlFlowStatement(ControlFlowStatement *v) override {
      PRINTNODEBASE
      printf(" type=%s)", controlflowtype_name(v->type));
      OBJBEGIN
      OBJPROPO("label", v->label)
    }
    void acceptReturnStatement(ReturnStatement *v) override {
      PRINTNODEBASE
      printf(")");
      OBJBEGIN
      OBJPROPO("value", v->value)
      OBJEND
    }
    void acceptScriptFileStatement(ScriptFileStatement *v) override {
      PRINTNODEBASE
      printf(" file-name='%s') [", m_filename);

      if (v->statements.empty()) {
        printf("]");
        return;
      }

      inc();

      for (uint32 i = 0; i < v->statements.size(); i++) {
        Statement* s = v->statements.at(i);
        nli();
        printf("[%i] = ", i);
        s->acceptVisit(this);
      }

      dec();
      nli();
      printf("]");
    }
    void acceptFunctionParam(FunctionParam *v) override {
      PRINTNODEBASE
      printf(")");
      OBJBEGIN
      OBJPROP("type", v->paramType)
      OBJPROP("name", v->name)
      OBJEND
    }
    void acceptFunctionDeclStatement(FunctionDeclStatement *v) override {
      PRINTNODEBASE
      printf(")");
      OBJBEGIN
      OBJPROP("name", v->name)
      OBJPROP("return-type", v->returnType)

      nli();
      printf(" arguments = [");
      if (!v->arguments.empty()) {
        inc();
        for (uint32 i = 0; i < v->arguments.size(); i++) {
          FunctionParam * p = v->arguments.at(i);
          nli();
          printf("[%i] = ", i);
          p->acceptVisit(this);
        }
        dec();
        nli();
      }
      printf("]");

      OBJPROP("body", v->functionBody);
      OBJEND
    }

    void acceptExprStatement(ExprStatement *v) override {
      PRINTNODEBASE
      printf(")");
      OBJBEGIN
      OBJPROP("expression", v->expression)
      OBJEND
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

  ScriptFileStatement* sfs = p.parse();
  PrintingVisitor pv = PrintingVisitor(&table, fname);

  sfs->acceptVisit(&pv);

  return EXIT_SUCCESS;
}

