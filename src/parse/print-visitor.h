#ifndef QUICKSCRIPT_PRINT_VISITOR_H
#define QUICKSCRIPT_PRINT_VISITOR_H

#include "../common.h"
#include "../stringtable.h"
#include "syntaxtree.h"

#define PRINTEXPRBASE printf("%s(", v->nodeType());\
  printloc(v->location);\
  if (v->resultType) {printf(" res-type='%s'", v->resultType->typeName());}

#define PRINTNODEBASE printf("%s(", v->nodeType());printloc(v->location);
#define OBJBEGIN printf(" {");inc();
#define OBJEND dec();nli();printf("}");
#define OBJPROP(name, v) nli();printf("%s = ", name);v->acceptVisit(this);
#define OBJPROPO(name, v) if (v) { nli();printf("%s = ", name);v->acceptVisit(this); }
#define OBJPROPARR(name, v) \
  nli();\
  printf("%s = [", name);\
  if (!v.empty()) {\
    inc();\
    for (uint32 i = 0; i < v.size(); i++) {\
      nli();\
      printf("[%i] = ", i);\
      v.at(i)->acceptVisit(this);\
    }\
    dec();\
    nli();\
  }\
  printf("]");

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
      printf(" type=%s)", parsedprimitivetype_name(v->primType));
    }

    void acceptIdentifier(Identifier *v) override {
      PRINTEXPRBASE
      printf(" value='");
      printstr(v->value);
      printf("')");
    }
    void acceptCallExpr(CallExpr *v) override {
      PRINTEXPRBASE;
      printf(")");
      OBJBEGIN
      OBJPROP("target", v->target)
      OBJPROPARR("arguments", v->arguments)
      OBJEND
    }
    void acceptPropertyAccessExpr(PropertyAccessExpr *v) override {
      PRINTEXPRBASE
      printf(")");
      OBJBEGIN
      OBJPROP("property", v->property)
      OBJPROP("target", v->target)
      OBJEND
    }
    void acceptBooleanLiteral(BooleanLiteral *v) override {
      PRINTEXPRBASE
      printf(" value=");
      if (v->value) {
        printf("true");
      } else {
        printf("false");
      }
      printf(")");
    }
    void acceptCharLiteral(CharLiteral *v) override {
      PRINTEXPRBASE
      printf(" value='");
      printstr(v->value);
      printf("')");
    }
    void acceptStringLiteral(StringLiteral *v) override {
      PRINTEXPRBASE
      printf(" value=\"");
      printstr(v->value);
      printf("\")");
    }
    void acceptIntLiteral(IntLiteral *v) override {
      PRINTEXPRBASE
      printf(" value=%llu)", v->value);
    }
    void acceptFloatLiteral(FloatLiteral *v) override {
      PRINTEXPRBASE
      printf(" value=%f)", v->value);
    }
    void acceptBinaryExpr(BinaryExpr *v) override {
      PRINTEXPRBASE
      printf(" op=%s)", binaryop_name(v->op));
      OBJBEGIN
      OBJPROP("lhs", v->lhs)
      OBJPROP("rhs", v->rhs)
      OBJEND
    }
    void acceptUnaryExpr(UnaryExpr *v) override {
      PRINTEXPRBASE
      printf("op = %s)", unaryop_name(v->op));
      OBJBEGIN
      OBJPROP("target", v->target)
      OBJEND
    }
    void acceptTernaryExpr(TernaryExpr *v) override {
      PRINTEXPRBASE
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
      OBJPROPARR("arguments", v->arguments)
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

    void acceptIndexAccessExpr(IndexAccessExpr *v) override {
      PRINTNODEBASE
      printf(")");
      OBJBEGIN
      OBJPROP("index", v->index)
      OBJPROP("target", v->target)
      OBJEND
    }

    void acceptStructPropertyDecl(StructPropertyDecl *v) override {
      PRINTNODEBASE
      printf(")");
      OBJBEGIN
      OBJPROP("type", v->propertyType)
      OBJPROP("name", v->name)
      OBJPROPO("value", v->value)
      OBJEND
    }

    void acceptStructDecl(StructDecl *v) override {
      PRINTNODEBASE
      printf(")");
      OBJBEGIN
      OBJPROP("name", v->name)
      OBJPROPARR("properties", v->properties)
      OBJEND
    }

    void acceptObjectLiteralProperty(ObjectLiteralProperty* v) override {
      PRINTNODEBASE
      printf(")");
      OBJBEGIN
      OBJPROP("property-name", v->propertyName)
      OBJPROP("value", v->value)
      OBJEND
    }

    void acceptObjectLiteral(ObjectLiteral* v) override {
      PRINTEXPRBASE
      printf(") [");

      std::vector<ObjectLiteralProperty*>& properties = v->properties;
      if (!properties.empty()) {
        inc();

        for (uint32 i = 0; i < properties.size(); i++) {
          nli();
          printf("[%i] = ", i);
          properties[i]->acceptVisit(this);
        }

        dec();
        nli();
      }

      printf("]");
    }
};

#endif //QUICKSCRIPT_PRINT_VISITOR_H
