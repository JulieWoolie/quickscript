#include "JsonPrinter.h"

#define START_NODE \
  m_result.append("{\"type\":\"");\
  m_result.append(v->nodeType());\
  m_result.append("\"");

#define END_NODE m_result.append("}");

#define PROP(name, val) \
  m_result.append(",\"" name "\":\"");\
  m_result.append(val);\
  m_result.append("\"");\

#define PROP_BEGIN(name) \
  m_result.append(",\"" name "\":");

#define AST_PROP(name) PROP_BEGIN(#name) v->name->acceptVisit(this);

#define AST_ARRAY(arr) \
  m_result.append("[");\
  for (uint32 i = 0; i < arr.size(); i++) {\
    if (i != 0) m_result.append(",");\
    arr.at(i)->acceptVisit(this);\
  }\
  m_result.append("]");


std::string& JsonPrinter::getResult() {
  return m_result;
}
JsonPrinter::JsonPrinter() {

}

void JsonPrinter::acceptTypeNameExpr(TypeNameExpr* v) {
  START_NODE
  PROP("name", v->typeName->view())
  END_NODE
}
void JsonPrinter::acceptArrayTypeExpr(ArrayTypeExpr* v) {
  START_NODE
  AST_PROP(componentType)
  END_NODE
}

void JsonPrinter::acceptPrimitiveTypeExpr(PrimitiveTypeExpr* v) {
  START_NODE
  PROP("ptype", parsedprimitivetype_name(v->primType))
  END_NODE
}

void JsonPrinter::acceptIdentifier(Identifier* v) {
  START_NODE
  PROP("value", v->value->view())
  END_NODE
}

void JsonPrinter::acceptCallExpr(CallExpr* v) {
  START_NODE
  AST_PROP(target)

  PROP_BEGIN("arguments")
  std::vector<Expr*>& args = v->arguments;
  AST_ARRAY(args)

  END_NODE
}
void JsonPrinter::acceptPropertyAccessExpr(PropertyAccessExpr* v) {
  START_NODE
  AST_PROP(target)
  AST_PROP(property)
  END_NODE
}
void JsonPrinter::acceptIndexAccessExpr(IndexAccessExpr* v) {
  START_NODE
  AST_PROP(target)
  AST_PROP(index)
  END_NODE
}
void JsonPrinter::acceptBooleanLiteral(BooleanLiteral* v) {
  START_NODE
  PROP("value", v->value ? "true" : "false")
  END_NODE
}
void JsonPrinter::acceptCharLiteral(CharLiteral* v) {
  START_NODE
  PROP("value", v->value ? "true" : "false")
  END_NODE
}
void JsonPrinter::acceptStringLiteral(StringLiteral* v) {
  START_NODE
  PROP("value", v->value->view())
  END_NODE
}
void JsonPrinter::acceptIntLiteral(IntLiteral* v) {
  START_NODE
  PROP("value", std::to_string(v->value))
  END_NODE
}
void JsonPrinter::acceptFloatLiteral(FloatLiteral* v) {
  START_NODE
  PROP("value", std::to_string(v->value))
  END_NODE
}
void JsonPrinter::acceptObjectLiteral(ObjectLiteral* v) {
  START_NODE
  PROP_BEGIN("properties")

  std::vector<ObjectLiteralProperty*>& props = v->properties;
  AST_ARRAY(props)

  END_NODE
}
void JsonPrinter::acceptObjectLiteralProperty(ObjectLiteralProperty* v) {
  START_NODE
  AST_PROP(propertyName)
  AST_PROP(value)
  END_NODE
}
void JsonPrinter::acceptArrayLiteral(ArrayLiteral* v) {
  START_NODE
  PROP_BEGIN("values")

  std::vector<Expr*>& values = v->values;
  AST_ARRAY(values)

  END_NODE
}
void JsonPrinter::acceptBinaryExpr(BinaryExpr* v) {
  START_NODE
  PROP("op", binaryop_name(v->op))
  AST_PROP(lhs)
  AST_PROP(rhs)
  END_NODE
}
void JsonPrinter::acceptUnaryExpr(UnaryExpr* v) {
  START_NODE
  PROP("op", unaryop_name(v->op))
  AST_PROP(target)
  END_NODE
}
void JsonPrinter::acceptTernaryExpr(TernaryExpr* v) {
  START_NODE
  AST_PROP(condition)
  AST_PROP(left)
  AST_PROP(right)
  END_NODE
}
void JsonPrinter::acceptBlock(Block* v) {
  START_NODE
  PROP_BEGIN("statements")

  std::vector<Statement*>& stats = v->statements;
  AST_ARRAY(stats)

  END_NODE
}
void JsonPrinter::acceptIfStatement(IfStatement* v) {
  START_NODE
  AST_PROP(condition)
  AST_PROP(body)
  if (v->elseBody) {
    AST_PROP(elseBody)
  }
  END_NODE
}
void JsonPrinter::acceptForStatement(ForStatement* v) {
  START_NODE
  if (v->label) {
    AST_PROP(label)
  }
  AST_PROP(first)
  AST_PROP(second)
  AST_PROP(third)
  AST_PROP(loopBody)
  END_NODE
}
void JsonPrinter::acceptLexicalDeclaration(LexicalDeclaration* v) {
  START_NODE
  PROP("const", v->isConstDeclaration ? "true" : "false")
  AST_PROP(typeExpr)
  AST_PROP(variableName)
  if (v->value) {
    AST_PROP(value)
  }
  END_NODE
}
void JsonPrinter::acceptWhileStatement(WhileStatement* v) {
  START_NODE
  if (v->label) {
    AST_PROP(label)
  }
  PROP("doWhile", v->doWhile ? "true" : "false")
  AST_PROP(condition)
  AST_PROP(body)
  END_NODE
}
void JsonPrinter::acceptControlFlowStatement(ControlFlowStatement* v) {
  START_NODE
  PROP("type", controlflowtype_name(v->type))
  if (v->label) {
    AST_PROP(label)
  }
  END_NODE
}
void JsonPrinter::acceptReturnStatement(ReturnStatement* v) {
  START_NODE
  if (v->value) {
    AST_PROP(value)
  }
  END_NODE
}
void JsonPrinter::acceptScriptFileStatement(ScriptFileStatement* v) {
  acceptBlock(v);
}
void JsonPrinter::acceptFunctionParam(FunctionParam* v) {
  START_NODE
  AST_PROP(paramType)
  AST_PROP(name)
  PROP("varargs", v->varargs ? "true" : "false")
  END_NODE
}
void JsonPrinter::acceptFunctionDeclStatement(FunctionDeclStatement* v) {
  START_NODE
  AST_PROP(returnType)
  AST_PROP(name)

  PROP_BEGIN("arguments")
  std::vector<FunctionParam*>& args = v->arguments;
  AST_ARRAY(args)

  AST_PROP(functionBody)

  END_NODE
}
void JsonPrinter::acceptExprStatement(ExprStatement* v) {
  START_NODE
  AST_PROP(expression)
  END_NODE
}
void JsonPrinter::acceptStructPropertyDecl(StructPropertyDecl* v) {
  START_NODE
  AST_PROP(propertyType)
  AST_PROP(name)
  if (v->value) {
    AST_PROP(value)
  }
  END_NODE
}
void JsonPrinter::acceptStructDecl(StructDecl* v) {
  START_NODE
  AST_PROP(name)

  std::vector<StructPropertyDecl*>& props = v->properties;
  AST_ARRAY(props)

  END_NODE
}
void JsonPrinter::acceptAssertStatement(AssertStatement* v) {
  START_NODE
  AST_PROP(condition)
  if (v->message) {
    AST_PROP(message)
  }
  END_NODE
}
