#include "TypeResolver.h"



TypeResolver::TypeResolver(TypeLookup* lookup, StringTable* strings, CompilerErrors* errors) {
  m_lookup = lookup;
  m_strings = strings;
  m_errors = errors;
}

void TypeResolver::acceptTypeNameExpr(TypeNameExpr* v) {
  std::string typeName = m_strings->getstring(v->typeName);
  v->referencedType = m_lookup->findReferencedType(typeName);

  if (v->referencedType) {
    return;
  }
  m_errors->error(v->location, "Unknwon type '%s'", typeName.c_str());
}

void TypeResolver::acceptArrayTypeExpr(ArrayTypeExpr* v) {
  v->componentType->acceptVisit(this);
  ScriptType* compType = v->componentType->getReferencedType();

  if (!compType) {
    return;
  }

  v->referencedType = m_lookup->getArrayType(compType);
}

primitivekind parsedPrimitiveToTypeKind(parsedprimitivetype ppt) {
  switch (ppt) {
    case PPT_BOOL: return PK_BOOL;
    case PPT_UINT8: return PK_UINT8;
    case PPT_INT8: return PK_INT8;
    case PPT_UINT16: return PK_UINT16;
    case PPT_INT16: return PK_INT16;
    case PPT_UINT32: return PK_UINT32;
    case PPT_INT32: return PK_INT32;
    case PPT_UINT64: return PK_UINT64;
    case PPT_INT64: return PK_INT64;
    case PPT_FLOAT32: return PK_FLOAT32;
    case PPT_FLOAT64: return PK_FLOAT64;
    default: return PK_NIL;
  }
}

void TypeResolver::acceptPrimitiveTypeExpr(PrimitiveTypeExpr* v) {
  parsedprimitivetype ppt = v->primType;

  if (ppt == PPT_STRING) {
    v->referencedType = m_lookup->getStringType();
    return;
  }

  primitivekind pk = parsedPrimitiveToTypeKind(ppt);
  v->referencedType = m_lookup->getPrimitiveType(pk);
}

void TypeResolver::acceptIdentifier(Identifier* v) {

}

void TypeResolver::acceptCallExpr(CallExpr* v) {
  v->target->acceptVisit(this);
  for (Expr* arg : v->arguments) {
    arg->acceptVisit(this);
  }
}

void TypeResolver::acceptPropertyAccessExpr(PropertyAccessExpr* v) {
  v->target->acceptVisit(this);
}

void TypeResolver::acceptIndexAccessExpr(IndexAccessExpr* v) {
  v->index->acceptVisit(this);
  v->target->acceptVisit(this);
}

void TypeResolver::acceptBooleanLiteral(BooleanLiteral* v) {
  v->resultType = m_lookup->getPrimitiveType(PK_BOOL);
}

void TypeResolver::acceptCharLiteral(CharLiteral* v) {
  v->resultType = m_lookup->getPrimitiveType(PK_UINT16);
}

void TypeResolver::acceptStringLiteral(StringLiteral* v) {
  v->resultType = m_lookup->getStringType();
}

void TypeResolver::acceptIntLiteral(IntLiteral* v) {
  v->resultType = m_lookup->getPrimitiveType(PK_UINT64);
}

void TypeResolver::acceptFloatLiteral(FloatLiteral* v) {
  v->resultType = m_lookup->getPrimitiveType(PK_FLOAT64);
}

void TypeResolver::acceptBinaryExpr(BinaryExpr* v) {
  v->lhs->acceptVisit(this);
  v->rhs->acceptVisit(this);
}

void TypeResolver::acceptUnaryExpr(UnaryExpr* v) {
  v->target->acceptVisit(this);
  v->setResultingType(v->target->getResultingType());
}

void TypeResolver::acceptTernaryExpr(TernaryExpr* v) {
  v->condition->acceptVisit(this);
  v->left->acceptVisit(this);
  v->right->acceptVisit(this);
}

void TypeResolver::acceptBlock(Block* v) {
  for (Statement* statement : v->statements) {
    statement->acceptVisit(this);
  }
}

void TypeResolver::acceptIfStatement(IfStatement* v) {
  v->condition->acceptVisit(this);
  v->body->acceptVisit(this);

  if (v->elseBody) {
    v->elseBody->acceptVisit(this);
  }
}

void TypeResolver::acceptForStatement(ForStatement* v) {
  v->first->acceptVisit(this);
  v->second->acceptVisit(this);
  v->third->acceptVisit(this);
  v->loopBody->acceptVisit(this);
}

void TypeResolver::acceptLexicalDeclaration(LexicalDeclaration* v) {
  v->typeExpr->acceptVisit(this);
  if (v->value) {
    v->value->acceptVisit(this);
  }
}

void TypeResolver::acceptDoWhileStatement(DoWhileStatement* v) {
  v->body->acceptVisit(this);
  v->condition->acceptVisit(this);
}

void TypeResolver::acceptWhileStatement(WhileStatement* v) {
  v->condition->acceptVisit(this);
  v->body->acceptVisit(this);
}

void TypeResolver::acceptControlFlowStatement(ControlFlowStatement* v) {

}

void TypeResolver::acceptReturnStatement(ReturnStatement* v) {
  if (v->value) {
    v->value->acceptVisit(this);
  }
}

void TypeResolver::acceptScriptFileStatement(ScriptFileStatement* v) {
  acceptBlock(v);
}

void TypeResolver::acceptFunctionParam(FunctionParam* v) {
  v->paramType->acceptVisit(this);
}
void TypeResolver::acceptFunctionDeclStatement(FunctionDeclStatement* v) {
  v->returnType->acceptVisit(this);
  for (FunctionParam* p : v->arguments) {
    p->acceptVisit(this);
  }
}

void TypeResolver::acceptExprStatement(ExprStatement* v) {
  v->expression->acceptVisit(this);
}

void TypeResolver::acceptStructPropertyDecl(StructPropertyDecl* v) {
  v->propertyType->acceptVisit(this);
  ScriptType* propType = v->propertyType->getReferencedType();

  if (!propType) {
    return;
  }

  ScriptStructType* containingType = v->structDeclStatement->type;
  std::string propname = m_strings->getstring(v->name->value);

  StructProperty prop = {
    .propertyName = propname,
    .type = propType
  };

  containingType->properties.push_back(prop);
}
void TypeResolver::acceptStructDecl(StructDecl* v) {
  std::string name = m_strings->getstring(v->name->value);
  ScriptStructType* stype = m_lookup->createStructType(name);

  if (!stype) {
    m_errors->error(v->location, "Double declaration of struct type '%s'", name.c_str());
    return;
  }

  for (StructPropertyDecl* prop : v->properties) {
    prop->acceptVisit(this);
  }
}
