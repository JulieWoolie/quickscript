#include "TypeResolver.h"

void TypeResolver::popScope() {
  m_scopes.pop_back();
}

void TypeResolver::pushScope() {
  LexicalScope* parentScope = getScope();

  LexicalScope s;
  if (parentScope) {
    s.expectedReturnType = parentScope->expectedReturnType;
  }

  m_scopes.push_back(s);
}

LexicalScope* TypeResolver::getScope() {
  if (m_scopes.empty()) {
    return nullptr;
  }
  return m_scopes.data() + (m_scopes.size() - 1);
}

void TypeResolver::pushSymbol(stringid name, ScriptType* type) {
  std::string nameStr = m_strings->getstring(name);
  getScope()->symbols.emplace_back(nameStr, type);
}

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

  v->referencedType = m_lookup->getVoidType();
  m_errors->error(v->location, "Unknown type '%s'", typeName.c_str());
}

void TypeResolver::acceptArrayTypeExpr(ArrayTypeExpr* v) {
  v->componentType->acceptVisit(this);
  ScriptType* compType = v->componentType->getReferencedType();

  if (!compType) {
    compType = m_lookup->getVoidType();
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
  ScriptType* expectedType = nullptr;
  if (!m_expectedTypes.empty()) {
    expectedType = m_expectedTypes.at(m_expectedTypes.size() - 1);
  }

  std::string name = m_strings->getstring(v->value);

  for (int32 i = m_scopes.size() - 1; i >= 0; i--) {
    LexicalScope* scope = m_scopes.data() + i;
    std::vector<LexicalSymbol> symbols = scope->symbols;

    for (uint32 j = 0; j < symbols.size(); j++) {
      LexicalSymbol& symbol = symbols.at(j);

      if (symbol.name != name) {
        continue;
      }

      v->resultType = symbol.type;
      return;
    }
  }

  m_errors->error(v->location, "Unknown variable/function '%s'", name.c_str());
}

void TypeResolver::acceptCallExpr(CallExpr* v) {
  uint32 args = v->arguments.size();

  FunctionSignature expected;
  expected.returnType = nullptr;
  expected.paramCount = args;

  FunctionSignatureParam params[args];
  expected.params = params;

  for (uint32 i = 0; i < args; i++) {
    Expr* e = v->arguments.at(i);
    e->acceptVisit(this);
    params[i].type = e->getResultingType();
    params[i].varargs = false;
  }

  m_expectedTypes.push_back(&expected);
  v->target->acceptVisit(this);
  m_expectedTypes.pop_back();

  v->resultType = m_lookup->getPrimitiveType(PK_UINT32);
}

bool hasProperties(const typekind kind) {
  switch (kind) {
    case TK_STRING:
    case TK_ARRAY:
    case TK_STRUCT:
      return true;
    default:
      return false;
  }
}

void TypeResolver::acceptPropertyAccessExpr(PropertyAccessExpr* v) {
  v->target->acceptVisit(this);
  ScriptType* resType = v->target->getResultingType();

  if (!(resType->typeFlags() & TYPEFLAG_PROPERTIES)) {
    m_errors->error(v->location, "%s has no properties that can be accessed", resType->typeName());
    return;
  }

  std::string queriedProp = m_strings->getstring(v->property->value);
  ScriptType* propertyType = resType->getPropertyType(queriedProp, m_lookup);

  if (propertyType == nullptr) {
    m_errors->error(v->location,
      "No such property '%s' on %s",
      queriedProp.c_str(), resType->typeName()
    );
    propertyType = m_lookup->getVoidType();
  }

  v->resultType = propertyType;
}

void TypeResolver::acceptIndexAccessExpr(IndexAccessExpr* v) {
  v->index->acceptVisit(this);
  ScriptType* indexType = v->index->getResultingType();

  if (!isIntegerType(indexType)) {
    m_errors->error(v->index->location,
      "%s cannot be used to index an array or string",
      indexType->typeName()
    );
  }

  v->target->acceptVisit(this);
  ScriptType* resultType = v->target->getResultingType();

  if (!(resultType->typeFlags() & TYPEFLAG_INDEXABLE)) {
    m_errors->error(v->location, "Type %s cannot be indexed", resultType->typeName());
    v->resultType = m_lookup->getVoidType();
    return;
  }

  ScriptType* indexedType = resultType->getIndexedType(m_lookup);
  v->resultType = indexedType;
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

ScriptType* TypeResolver::getOpResultType(ScriptType* left, ScriptType* right, binaryop op) {
  // Clear the assignment flag
  op &= !BOP_ASSIGN_FLAG;

  if (left->kind() == TK_STRING) {
    if ((right->kind() == TK_STRING || right->kind() == TK_PRIMITIVE) && op == BOP_ADD) {
      return m_lookup->getStringType();
    }
    if (isIntegerType(right) && op == BOP_MUL) {
      return m_lookup->getStringType();
    }
    return nullptr;
  }

  if (left->kind() != TK_PRIMITIVE || right->kind() != TK_PRIMITIVE) {
    if (op == BOP_EQ || op == BOP_NEQ) {
      if (left == right) {
        return left;
      }
      return nullptr;
    }

    return nullptr;
  }

  PrimitiveScriptType* pl = (PrimitiveScriptType*) left;
  PrimitiveScriptType* pr = (PrimitiveScriptType*) right;

  primitivekind lkind = pl->primtype;
  primitivekind rkind = pr->primtype;

  switch (op) {
    case BOP_LOG_AND:
    case BOP_LOG_OR:
    case BOP_SHL:
    case BOP_SHR:
    case BOP_USHR:
    case BOP_BIT_AND:
    case BOP_BIT_OR:
    case BOP_XOR:
      if (!(pkIsIntegerType(lkind) || lkind == PK_BOOL)
        || !(pkIsIntegerType(rkind) || rkind == PK_BOOL)
      ) {
        return nullptr;
      }
    default:
      return widestNumberType(pl, pr);
  }
}

void TypeResolver::acceptBinaryExpr(BinaryExpr* v) {
  v->lhs->acceptVisit(this);
  v->rhs->acceptVisit(this);

  ScriptType* ltype = v->lhs->getResultingType();
  ScriptType* rtype = v->rhs->getResultingType();

  binaryop op = v->op;
  ScriptType* res = getOpResultType(ltype, rtype, op);

  if (!res) {
    m_errors->error(v->location,
      "Cannot use %s operator on %s and %s",
      binaryop_name(op), ltype, rtype
    );

    v->resultType = ltype;
    return;
  }

  v->resultType = res;
}

bool unaryOpIsValidFor(unaryop op, ScriptType* type) {
  if (type->kind() != TK_PRIMITIVE) {
    return false;
  }

  PrimitiveScriptType* p = (PrimitiveScriptType*) type;

  switch (op) {
    case UOP_BIT_NOT:
      return isIntegerType(p);
    case UOP_LOG_NOT:
      return isIntegerType(p) || p->primtype == PK_BOOL;
    default:
      return true;
  }
}

void TypeResolver::acceptUnaryExpr(UnaryExpr* v) {
  v->target->acceptVisit(this);
  v->setResultingType(v->target->getResultingType());

  if (unaryOpIsValidFor(v->op, v->resultType)) {
    return;
  }

  m_errors->error(v->location, "Cannot use %s operator on %s",
    unaryop_name(v->op),
    v->resultType->typeName()
  );
}

void TypeResolver::acceptTernaryExpr(TernaryExpr* v) {
  v->condition->acceptVisit(this);

  if (v->condition->getResultingType()->kind() != TK_PRIMITIVE) {
    m_errors->error(v->condition->location, "%s is not assignable to a bool condition",
      v->condition->getResultingType()->typeName()
    );
  }

  v->left->acceptVisit(this);
  v->right->acceptVisit(this);

  ScriptType* lType = v->left->getResultingType();
  ScriptType* rType = v->right->getResultingType();

  ScriptType* common = getCommonType(lType, rType);

  if (!common) {
    m_errors->error(
      v->location,
      "Ternary operator left and right values have incompatible types: %s and %s",
      lType->typeName(),
      rType->typeName()
    );
    v->resultType = lType;
    return;
  }

  v->resultType = common;
}

void TypeResolver::acceptBlock(Block* v) {
  pushScope();
  for (Statement* statement : v->statements) {
    statement->acceptVisit(this);
  }
  popScope();
}

void TypeResolver::acceptIfStatement(IfStatement* v) {
  v->condition->acceptVisit(this);
  v->body->acceptVisit(this);

  if (v->elseBody) {
    v->elseBody->acceptVisit(this);
  }
}

void TypeResolver::acceptForStatement(ForStatement* v) {
  pushScope();

  v->first->acceptVisit(this);
  v->second->acceptVisit(this);
  v->third->acceptVisit(this);
  v->loopBody->acceptVisit(this);

  popScope();
}

void TypeResolver::acceptLexicalDeclaration(LexicalDeclaration* v) {
  v->typeExpr->acceptVisit(this);

  if (v->value) {
    v->value->acceptVisit(this);

    ScriptType* vartype = v->typeExpr->getReferencedType();
    ScriptType* valtype = v->value->getResultingType();

    if (!isAssignableTo(vartype, valtype)) {
      m_errors->error(v->location,
        "Value of type %s is not assignable to variable with type %s",
        valtype->typeName(),
        vartype->typeName()
      );
    }
  }

  pushSymbol(v->variableName->value, v->typeExpr->getReferencedType());
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
  if (!v->value) {
    return;
  }

  v->value->acceptVisit(this);

  ScriptType* rtype = v->value->getResultingType();
  ScriptType* expected = getScope()->expectedReturnType;

  if (!expected || isAssignableTo(expected, rtype)) {
    return;
  }

  m_errors->error(v->location,
    "Returned value of type %s cannot be assigned to expected type %s",
    rtype->typeName(),
    expected->typeName()
  );
}

void TypeResolver::acceptScriptFileStatement(ScriptFileStatement* v) {
  pushScope();
  getScope()->expectedReturnType = nullptr;

  for (Statement* statement : v->statements) {
    statement->acceptVisit(this);
  }

  popScope();
}

void TypeResolver::acceptFunctionParam(FunctionParam* v) {
  // Should not be called
}

void TypeResolver::acceptFunctionDeclStatement(FunctionDeclStatement* v) {
  v->returnType->acceptVisit(this);

  FunctionSignature* sign = m_lookup->createFunctionSignature(v->arguments.size());
  sign->returnType = v->returnType->getReferencedType();

  pushScope();
  getScope()->expectedReturnType = v->returnType->getReferencedType();

  for (uint32 i = 0; i < v->arguments.size(); i++) {
    FunctionParam* p = v->arguments.at(i);
    FunctionSignatureParam* sp = sign->params + i;

    bool varargs = p->varargs;
    ScriptType* trueType;

    if (varargs) {
      trueType = m_lookup->getArrayType(p->paramType->getReferencedType());
    } else {
      trueType = p->paramType->getReferencedType();
    }

    p->paramType->acceptVisit(this);
    sp->type = trueType;
    sp->varargs = p->varargs;

    pushSymbol(p->name->value, trueType);
  }

  v->functionBody->acceptVisit(this);

  popScope();
}

void TypeResolver::acceptExprStatement(ExprStatement* v) {
  v->expression->acceptVisit(this);
}

void TypeResolver::acceptStructPropertyDecl(StructPropertyDecl* v) {
  // Should not be called
}

void TypeResolver::acceptStructDecl(StructDecl* v) {
  const std::string name = m_strings->getstring(v->name->value);

  uint32 propcount = v->properties.size();
  ScriptStructType* stype = m_lookup->createStructType(name, propcount);

  if (!stype) {
    m_errors->error(v->location, "Double declaration of struct type '%s'", name.c_str());
    return;
  }

  for (uint32 i = 0; i < propcount; i++) {
    StructPropertyDecl* prop = v->properties.at(i);
    prop->propertyType->acceptVisit(this);

    std::string pname = m_strings->getstring(prop->name->value);
    ScriptType* ptype = prop->propertyType->getReferencedType();

    if (prop->value) {
      prop->value->acceptVisit(this);
      ScriptType* vtype = prop->value->getResultingType();

      if (isAssignableTo(ptype, vtype)) {
        m_errors->error(prop->location,
          "Default value of property %s.%s is a %s and cannot be assigned to %s",
          name.c_str(),
          pname.c_str(),
          vtype->typeName(),
          ptype->typeName()
        );
      }
    }

    StructProperty* typeprop = stype->properties + i;
    typeprop->type = ptype;
    typeprop->propertyName = pname;
  }
}
