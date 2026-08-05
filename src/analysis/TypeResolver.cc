#include "TypeResolver.h"

#include <functional>
#include <stdint.h>

#include "../types/ConstTypes.h"

#define STATPUSH m_statementStack.push_back(v);
#define STATPOP m_statementStack.pop_back();

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

LexicalScope* TypeResolver::getScope(uint32 off) {
  if (m_scopes.empty()) {
    return nullptr;
  }
  return m_scopes.data() + (m_scopes.size() - (1 + off));
}

void TypeResolver::pushSymbol(stringid name, ScriptType* type) {
  pushSymbol(getScope(), name, type);
}

void TypeResolver::pushSymbol(LexicalScope* scope, stringid name, ScriptType* type) const {
  std::string nameStr = m_strings->getstring(name);
  scope->symbols.emplace_back(nameStr, type);
}

TypeResolver::TypeResolver(
  TypeTable* lookup,
  StringTable* strings,
  CompilerErrors* errors,
  Bindings* bindings
) {
  m_lookup = lookup;
  m_strings = strings;
  m_errors = errors;
  m_bindings = bindings;
}

void TypeResolver::acceptTypeNameExpr(TypeNameExpr* v) {
  std::string typeName = m_strings->getstring(v->typeName);
  v->referencedType = m_lookup->lookupByName(typeName);

  if (v->referencedType) {
    return;
  }

  v->referencedType = ConstTypes::VOID();
  m_errors->error(v->location, "Unknown type '%s'", typeName.c_str());
}

void TypeResolver::acceptArrayTypeExpr(ArrayTypeExpr* v) {
  v->componentType->acceptVisit(this);
  ScriptType* compType = v->componentType->referencedType;

  if (!compType) {
    compType = ConstTypes::VOID();
  }

  std::string compName = compType->getTypeName();
  compName.append("[]");

  ScriptType* arrType = m_lookup->lookupByName(compName);
  if (!arrType) {
    arrType = new ScriptArrayType(compType);
    m_lookup->emplaceType(arrType);
  }

  v->referencedType = arrType;
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
    v->referencedType = ConstTypes::STRING();
    return;
  }
  if (ppt == PPT_VOID) {
    v->referencedType = ConstTypes::VOID();
    Statement* stat = m_statementStack.back();

    if (stat->nodeKind() == AST_LexicalDeclaration) {
      m_errors->error(v->location, "'void' type not allowed here");
    }

    return;
  }

  primitivekind pk = parsedPrimitiveToTypeKind(ppt);
  v->referencedType = ConstTypes::getPrimitiveType(pk);
}

void TypeResolver::acceptIdentifier(Identifier* v) {
  ScriptType* expectedType = nullptr;
  uint32 expectedKind = TK_NIL;

  if (!m_expectedTypes.empty()) {
    expectedType = m_expectedTypes.at(m_expectedTypes.size() - 1);
    expectedKind = expectedType->kind();
  }

  std::string name = m_strings->getstring(v->value);

  uint32 scores[10];
  FunctionSignature* signatures[10];
  uint32 scorerlen = 0;

  for (int32 i = m_scopes.size() - 1; i >= 0; i--) {
    LexicalScope* scope = m_scopes.data() + i;
    std::vector<LexicalSymbol> symbols = scope->symbols;

    for (LexicalSymbol& symbol: symbols) {
      if (symbol.name != name) {
        continue;
      }
      ScriptType* stype = symbol.type;

      if (!expectedType || expectedKind != TK_FUNC) {
        if (stype->kind() == TK_FUNC) {
          continue;
        }
        v->resultType = symbol.type;
        return;
      }

      if (stype->kind() != TK_FUNC) {
        continue;
      }

      FunctionSignature* callingSign = static_cast<FunctionSignature*>(expectedType);
      FunctionSignature* funcSign = static_cast<FunctionSignature*>(stype);
      
      const int32 score = FunctionSignature::callSignatureMatches(callingSign, funcSign);
      if (score == SIGN_DOES_NOT_MATCH) {
        continue;
      }

      scores[scorerlen] = score;
      signatures[scorerlen] = funcSign;
      scorerlen++;
    }
  }

  if (scorerlen == 0) {
    m_errors->error(v->location, "Unknown variable/function '%s'", name.c_str());
    v->resultType = ConstTypes::VOID();
    return;
  }
  if (scorerlen == 1) {
    v->resultType = signatures[0];
    return;
  }

  int32 highest = -1;
  FunctionSignature* best = nullptr;

  for (uint32 idx = 0; idx < scorerlen; idx++) {
    int32 scr = scores[idx];

    if (scr <= highest) {
      continue;
    }

    highest = scr;
    best = signatures[idx];
  }

  v->resultType = best;
}

void TypeResolver::acceptCallExpr(CallExpr* v) {
  uint32 args = v->arguments.size();

  ScriptType* params[args];

  for (uint32 i = 0; i < args; i++) {
    Expr* e = v->arguments.at(i);
    e->acceptVisit(this);
    params[i] = e->resultType;
  }
  
  FunctionSignature sign = FunctionSignature(nullptr, false, args, params);

  m_expectedTypes.push_back(&sign);
  v->target->acceptVisit(this);
  m_expectedTypes.pop_back();

  ScriptType* targetType = v->target->resultType;
  if (targetType->kind() != TK_FUNC) {
    m_errors->error(v->location,
      "Expression does not return a callable function, but returns a %s",
      targetType->getTypeName()
    );

    v->resultType = targetType;
    return;
  }

  v->resultType = static_cast<FunctionSignature*>(targetType)->getReturnType();
}

void TypeResolver::acceptPropertyAccessExpr(PropertyAccessExpr* v) {
  v->target->acceptVisit(this);
  ScriptType* resType = v->target->resultType;

  if (!(resType->typeFlags() & TFLAG_PROPERTY_HOLDER)) {
    m_errors->error(v->location, 
      "%s has no properties that can be accessed", 
      resType->getTypeName()
    );
    return;
  }

  std::string queriedProp = m_strings->getstring(v->property->value);
  ScriptType* propertyType = resType->getPropertyType(queriedProp);

  if (propertyType == nullptr) {
    m_errors->error(v->location,
      "No such property '%s' on %s",
      queriedProp.c_str(), resType->getTypeName()
    );
    propertyType = ConstTypes::VOID();
  }

  v->resultType = propertyType;
}

void TypeResolver::acceptIndexAccessExpr(IndexAccessExpr* v) {
  v->index->acceptVisit(this);
  ScriptType* indexType = v->index->resultType;

  if (!isIntegerType(indexType)) {
    m_errors->error(v->index->location,
      "%s cannot be used to index an array or string",
      indexType->getTypeName()
    );
  }

  v->target->acceptVisit(this);
  ScriptType* resultType = v->target->resultType;

  if (!(resultType->typeFlags() & TFLAG_INDEXABLE)) {
    m_errors->error(v->location, "Type %s cannot be indexed", resultType->getTypeName());
    v->resultType = ConstTypes::VOID();
    return;
  }

  ScriptType* indexedType = resultType->getIndexReturnType();
  v->resultType = indexedType;
}

void TypeResolver::acceptBooleanLiteral(BooleanLiteral* v) {
  v->resultType = ConstTypes::BOOL();
}

void TypeResolver::acceptCharLiteral(CharLiteral* v) {
  v->resultType = ConstTypes::INT8();
}

void TypeResolver::acceptStringLiteral(StringLiteral* v) {
  v->resultType = ConstTypes::STRING();
}

// Source - https://stackoverflow.com/a/4609795
// Posted by user79758, modified by community. See post 'Timeline' for change history
// Retrieved 2026-07-19, License - CC BY-SA 4.0
template <typename T> int8 sgn(T val) {
  return (T(0) < val) - (val < T(0));
}


void TypeResolver::acceptIntLiteral(IntLiteral* v) {
  int64 val = v->value;
  parsedprimitivetype smallestFitting = PPT_INT64;

  int8 sign = sgn(val);

  if (sign == 0) {
    smallestFitting = PPT_UINT8;
  } else if (sign == -1) {
    if (val >= -128) {
      smallestFitting = PPT_INT8;
    } else if (val >= INT16_MIN) {
      smallestFitting = PPT_INT16;
    } else if (val >= INT32_MIN) {
      smallestFitting = PPT_INT32;
    } else {
      smallestFitting = PPT_INT64;
    }
  } else {
    if (val <= INT8_MAX) {
      smallestFitting = PPT_INT8;
    } else if (val <= UINT8_MAX) {
      smallestFitting = PPT_UINT8;
    } else if (val <= INT16_MAX) {
      smallestFitting = PPT_INT16;
    } else if (val <= UINT16_MAX) {
      smallestFitting = PPT_UINT16;
    } else if (val <= INT32_MAX) {
      smallestFitting = PPT_INT32;
    } else if (val <= UINT32_MAX) {
      smallestFitting = PPT_UINT32;
    } else {
      smallestFitting = PPT_UINT64;
    }
  }

  primitivekind pk = parsedPrimitiveToTypeKind(smallestFitting);

  v->resultType = ConstTypes::getPrimitiveType(pk);
  v->smallestFittingType = smallestFitting;
}

void TypeResolver::acceptFloatLiteral(FloatLiteral* v) {
  const float64 val = v->value;
  parsedprimitivetype smallestFitting = PPT_FLOAT64;

  float32 sv = static_cast<float32>(val);
  if (val == sv) {
    smallestFitting = PPT_FLOAT32;
  } else {
    smallestFitting = PPT_FLOAT64;
  }

  v->resultType = ConstTypes::getPrimitiveType(smallestFitting);
  v->smallestFittingType = smallestFitting;
}

void TypeResolver::acceptObjectLiteral(ObjectLiteral* v) {
  ScriptType* expectedType = m_expectedTypes.back();
  if (expectedType->kind() != TK_STRUCT) {
    m_errors->error(
      v->location,
      "Type %s cannot be initialized with an object literal",
      expectedType->getTypeName()
    );
    return;
  }

  ScriptStructType* structType = static_cast<ScriptStructType*>(expectedType);
  uint32 pcount = structType->getPropertyCount();

  v->resultType = structType;

  for (ObjectLiteralProperty* prop : v->properties) {
    std::string propertyName = m_strings->getstring(prop->propertyName->value);
    ScriptType* proptype = nullptr;

    for (uint32 pi = 0; pi < pcount; pi++) {
      StructProperty* prop = structType->getProperty(pi);

      if (prop->name != propertyName) {
        continue;
      }

      proptype = prop->type;
      break;
    }

    if (!proptype) {
      m_errors->error(prop->location, "No such property named '%s' on struct %s",
        propertyName.c_str(),
        structType->getTypeName()
      );
      continue;
    }

    m_expectedTypes.push_back(proptype);
    prop->value->acceptVisit(this);
    m_expectedTypes.pop_back();

    ScriptType* pvalType = prop->value->resultType;

    if (!isAssignableTo(proptype, pvalType)) {
      m_errors->error(prop->location,
        "Cannot assign value of type %s to property of type %s",
        pvalType->getTypeName(),
        proptype->getTypeName()
      );
    }
  }
}

void TypeResolver::acceptObjectLiteralProperty(ObjectLiteralProperty* v) {

}

void TypeResolver::acceptArrayLiteral(ArrayLiteral* v) {
  ScriptType* type = m_expectedTypes.back();
  if (type->kind() != TK_ARRAY) {
    m_errors->error(
      v->location,
      "Type %s cannot be initialized with an array literal",
      type->getTypeName()
    );

    v->resultType = ConstTypes::VOID();
    return;
  }

  ScriptType* componentType = static_cast<ScriptArrayType*>(type)->getComponentType();
  m_expectedTypes.push_back(componentType);

  for (Expr* expr : v->values) {
    expr->acceptVisit(this);

    ScriptType* valType = expr->resultType;
    if (isAssignableTo(componentType, valType)) {
      continue;
    }

    m_errors->error(expr->location,
      "Cannot use value of type %s in array of type %s",
      valType->getTypeName(),
      componentType->getTypeName()
    );
  }

  m_expectedTypes.pop_back();
  v->resultType = type;
}

static bool isArrayTypeComparable(ScriptArrayType* type) {
  ScriptType* compType = type->getComponentType();
  switch (compType->kind()) {
    case TK_PRIMITIVE:
    case TK_STRING:
      return true;
    case TK_ARRAY:
      return isArrayTypeComparable(static_cast<ScriptArrayType*>(compType));
    default:
      return false;
  }
}

// Operations and allowed operand types and their resulting types:
//
// - GT, GTE, LT, LTE => bool:
//     - Any two numerical operands are allowed
//     - Any two arrays with comparable component types operands are allowed
//     - Any two string operands are allowed
// - EQ, NEQ => bool:
//     - Any two operands of the same type are allowed
//     - Any two numerical operands are allowed
// - ADD:
//     - Two string operands are a allowed => string
//     - String and any type are allowed => concatenated string
//     - Any two numerical operands are allowed => wider number type
// - MUL:
//     - String and any integer operand are allowed => repeated string
//     - Any two numerical operands are allowed => wider number type
// - SUB, MOD, DIV, POW:
//     - Any two numerical operands are allowed => wider number type
// - SHL, SHR, USHR, BIT_OR, BIT_AND
//     - Any two integer operands are allowed => wider number type
// - XOR:
//     - Any two integer operands are allowed => wider number type
//     - Two boolean operands are allowed => bool
// - LOG_AND, LOG_OR => bool:
//     - Two boolean operands area allowed
//
// If the ASSIGN flag is set, meaning the operation will assign the
// right operand to the left side, the left side must be able to hold
// the right's value, and the left side's type will be returned as
// the result.
//
// That is, unless this is one of the exceptions carved out for
// strings ('+' for string concatenating, or '*' for repeating strings)
//
ScriptType* TypeResolver::getOpResultType(ScriptType* left, ScriptType* right, binaryop op) const {
  bool isAssign = op & BOP_ASSIGN_FLAG;

  if (op == BOP_ASSIGN) {
    if (isAssignableTo(left, right)) {
      return left;
    }
    return nullptr;
  }

  // Clear the assignment flag
  op &= ~BOP_ASSIGN_FLAG;

  typekind lkind = left->kind();
  typekind rkind = right->kind();

  switch (op) {
    case BOP_GT:
    case BOP_GTE:
    case BOP_LT:
    case BOP_LTE:
      if (isNumberType(left) && isNumberType(right)) {
        return ConstTypes::BOOL();
      }
      if (lkind == TK_STRING && rkind == TK_STRING) {
        return ConstTypes::BOOL();
      }
      if (lkind == TK_ARRAY && rkind == TK_ARRAY) {
        ScriptArrayType* larr = static_cast<ScriptArrayType*>(left);
        ScriptArrayType* rarr = static_cast<ScriptArrayType*>(right);

        if (larr != rarr || !isArrayTypeComparable(larr)) {
          return nullptr;
        }

        return ConstTypes::BOOL();
      }
      return nullptr;

    case BOP_EQ:
    case BOP_NEQ:
      if (lkind == TK_PRIMITIVE && rkind == TK_PRIMITIVE) {
        return ConstTypes::BOOL();
      }
      if (left == right) {
        return ConstTypes::BOOL();
      }
      return nullptr;

    case BOP_ADD:
      if (lkind == TK_STRING) {
        return left;
      }
    case BOP_MUL:
      // Holy nesting, but kinda needed for pass through from above case
      if (op == BOP_MUL) {
        if (lkind == TK_STRING) {
          if (isIntegerType(right)) {
            return left;
          }
          return nullptr;
        }
      }
    case BOP_SUB:
    case BOP_DIV:
    case BOP_POW:
    case BOP_MOD:
      if (lkind != TK_PRIMITIVE || rkind != TK_PRIMITIVE) {
        return nullptr;
      }
      if (isAssign) {
        if (isAssignableTo(left, right)) {
          return right;
        }
        return nullptr;
      }
      return widestNumberType(
        static_cast<PrimitiveScriptType*>(left),
        static_cast<PrimitiveScriptType*>(right)
      );

    case BOP_SHL:
    case BOP_SHR:
    case BOP_USHR:
    case BOP_BIT_OR:
    case BOP_BIT_AND:
      if (!isIntegerType(left) || !isIntegerType(right)) {
        return nullptr;
      }
      if (isAssign) {
        if (isAssignableTo(left, right)) {
          return right;
        }
        return nullptr;
      }
      return widestNumberType(
        static_cast<PrimitiveScriptType*>(left),
        static_cast<PrimitiveScriptType*>(right)
      );

    case BOP_XOR:
    case BOP_LOG_AND:
    case BOP_LOG_OR:
      if (isIntegerType(left) && isIntegerType(right)) {
        if (isAssign) {
          if (isAssignableTo(left, right)) {
            return right;
          }
          return nullptr;
        }
        return widestNumberType(
          static_cast<PrimitiveScriptType*>(left),
          static_cast<PrimitiveScriptType*>(right)
        );
      }
      if (isBooleanType(left) && isBooleanType(right)) {
        return left;
      }
      return nullptr;

    default:
      return nullptr;
  }
}

bool isLiteral(astnodetype type) {
  switch (type) {
    case AST_BooleanLiteral:
    case AST_FloatLiteral:
    case AST_IntLiteral:
    case AST_StringLiteral:
    case AST_CharLiteral:
    case AST_ObjectLiteral:
      return true;
    default:
      return false;
  }
}

void checkAssignability(Expr* expr, CompilerErrors* errors) {
  astnodetype kind = expr->nodeKind();

  switch (kind) {
    case AST_Identifier:
    case AST_PropertyAccessExpr:
    case AST_IndexAccessExpr:
      break;

    default:
      errors->error(expr->location, "Invalid left-hand-side expression; cannot be assigned to");
      break;
  }
}

void testTypeAssignability(Expr* expr, CompilerErrors* errors) {
  if (expr->nodeKind() == AST_IndexAccessExpr) {
    IndexAccessExpr* idx = static_cast<IndexAccessExpr*>(expr);
    ScriptType* type = idx->target->resultType;

    if (type->kind() == TK_STRING) {
      errors->error(expr->location, "Cannot mutate strings");
    }
    return;
  }

  if (expr->nodeKind() == AST_PropertyAccessExpr) {
    PropertyAccessExpr* prop = static_cast<PropertyAccessExpr*>(expr);
    ScriptType* objType = prop->target->resultType;

    if (prop->resultType->kind() == TK_VOID) {
      // Property not found, don't try to check
      return;
    }

    // Void means it's already failed in the TypeResolver stage
    if (objType->kind() == TK_ARRAY) {
      errors->error(expr->location, "Cannot mutate array length");
    }
    if (objType->kind() == TK_STRING) {
      errors->error(expr->location, "Cannot mutate string length");
    }
  }
}

void TypeResolver::acceptBinaryExpr(BinaryExpr* v) {
  v->lhs->acceptVisit(this);
  v->rhs->acceptVisit(this);

  ScriptType* ltype = v->lhs->resultType;
  ScriptType* rtype = v->rhs->resultType;

  binaryop op = v->op;
  ScriptType* res = getOpResultType(ltype, rtype, op);

  if (!res) {
    m_errors->error(v->location,
      "Cannot use %s operator on %s and %s",
      binaryop_name(op), ltype->getTypeName(), rtype->getTypeName()
    );

    v->resultType = ltype;
    return;
  }

  if (v->op & BOP_ASSIGN_FLAG) {
    checkAssignability(v->lhs, m_errors);
    testTypeAssignability(v->lhs, m_errors);
  }

  v->resultType = res;
}

ScriptType* checkUnaryOperation(unaryop op, ScriptType* type, TypeTable* lookup) {
  if (type->kind() != TK_PRIMITIVE) {
    return nullptr;
  }

  PrimitiveScriptType* p = (PrimitiveScriptType*) type;

  switch (op) {
    case UOP_BIT_NOT:
      if (isIntegerType(p)) {
        return type;
      }
    case UOP_LOG_NOT:
      if (isIntegerType(p) || p->getPrimitiveType() == PK_BOOL) {
        return type;
      }
    case UOP_NEG:
      // Unsigned type becomes signed
      switch (p->getPrimitiveType()) {
        case PK_UINT8:
          return ConstTypes::INT8();
        case PK_UINT16:
          return ConstTypes::INT16();
        case PK_UINT32:
          return ConstTypes::INT32();
        case PK_UINT64:
          return ConstTypes::INT64();
        case PK_BOOL:
          return nullptr;
        default:
          return type;
      }
    default:
      return type;
  }
}

void TypeResolver::acceptUnaryExpr(UnaryExpr* v) {
  v->target->acceptVisit(this);

  ScriptType* resType = checkUnaryOperation(v->op, v->target->resultType, m_lookup);

  if (resType) {
    testTypeAssignability(v->target, m_errors);
    v->resultType = resType;
    return;
  }

  v->resultType = v->target->resultType;

  m_errors->error(v->location, "Cannot use %s operator on %s",
    unaryop_name(v->op),
    v->resultType->getTypeName()
  );
}

void TypeResolver::acceptTernaryExpr(TernaryExpr* v) {
  v->condition->acceptVisit(this);

  if (v->condition->resultType->kind() != TK_PRIMITIVE) {
    m_errors->error(v->condition->location, "%s is not assignable to a bool condition",
      v->condition->resultType->getTypeName()
    );
  }

  v->left->acceptVisit(this);
  v->right->acceptVisit(this);

  ScriptType* lType = v->left->resultType;
  ScriptType* rType = v->right->resultType;

  ScriptType* common = getCommonType(lType, rType);

  if (!common) {
    m_errors->error(
      v->location,
      "Ternary operator left and right values have incompatible types: %s and %s",
      lType->getTypeName(),
      rType->getTypeName()
    );
    v->resultType = lType;
    return;
  }

  v->resultType = common;
}

void TypeResolver::acceptBlock(Block* v) {
  STATPUSH
  pushScope();

  for (Statement* statement : v->statements) {
    statement->acceptVisit(this);
  }

  popScope();
  STATPOP
}

void TypeResolver::acceptIfStatement(IfStatement* v) {
  STATPUSH

  v->condition->acceptVisit(this);
  v->body->acceptVisit(this);

  if (v->elseBody) {
    v->elseBody->acceptVisit(this);
  }

  STATPOP
}

void TypeResolver::acceptForStatement(ForStatement* v) {
  STATPUSH
  pushScope();

  v->first->acceptVisit(this);
  v->second->acceptVisit(this);
  v->third->acceptVisit(this);
  v->loopBody->acceptVisit(this);

  popScope();
  STATPOP
}

void TypeResolver::acceptLexicalDeclaration(LexicalDeclaration* v) {
  STATPUSH;

  v->typeExpr->acceptVisit(this);
  ScriptType* declType = v->typeExpr->referencedType;

  if (v->value) {
    m_expectedTypes.push_back(declType);
    v->value->acceptVisit(this);
    m_expectedTypes.pop_back();

    ScriptType* vartype = v->typeExpr->referencedType;
    ScriptType* valtype = v->value->resultType;

    if (!isAssignableTo(vartype, valtype)) {
      m_errors->error(v->location,
        "Value of type %s is not assignable to variable with type %s",
        valtype->getTypeName(),
        vartype->getTypeName()
      );
    }
  }

  pushSymbol(v->variableName->value, v->typeExpr->referencedType);

  STATPOP;
}

void TypeResolver::acceptDoWhileStatement(DoWhileStatement* v) {
  STATPUSH
  v->body->acceptVisit(this);
  v->condition->acceptVisit(this);
  STATPOP
}

void TypeResolver::acceptWhileStatement(WhileStatement* v) {
  STATPUSH
  v->condition->acceptVisit(this);
  v->body->acceptVisit(this);
  STATPOP
}

void TypeResolver::acceptControlFlowStatement(ControlFlowStatement* v) {

}

void TypeResolver::acceptReturnStatement(ReturnStatement* v) {
  ScriptType* expected = getScope()->expectedReturnType;

  if (!v->value) {
    if (!expected || expected->kind() == TK_VOID) {
      return;
    }

    m_errors->error(
      v->location,
      "Function expects return value with %s, cannot return nothing",
      expected->getTypeName()
    );

    return;
  }

  STATPUSH
  v->value->acceptVisit(this);

  ScriptType* rtype = v->value->resultType;

  if (!expected || isAssignableTo(expected, rtype)) {
    STATPOP
    return;
  }

  m_errors->error(v->location,
    "Returned value of type %s cannot be assigned to expected type %s",
    rtype->getTypeName(),
    expected->getTypeName()
  );

  STATPOP
}

void TypeResolver::acceptScriptFileStatement(ScriptFileStatement* v) {
  STATPUSH
  pushScope();

  LexicalScope* scope = getScope();
  scope->expectedReturnType = nullptr;

  std::vector<NativeBinding>* bindings = m_bindings->getBindings();
  uint32 bindingCount = bindings->size();

  for (uint32 i = 0; i < bindingCount; i++) {
    const NativeBinding& bind = bindings->at(i);
    scope->symbols.emplace_back(bind.name, bind.type);
  }

  for (Statement* statement : v->statements) {
    statement->acceptVisit(this);
  }

  popScope();
  STATPOP
}

void TypeResolver::acceptFunctionParam(FunctionParam* v) {
  // Should not be called
}

void TypeResolver::acceptFunctionDeclStatement(FunctionDeclStatement* v) {
  STATPUSH

  v->returnType->acceptVisit(this);

  const uint32 paramCount = v->arguments.size();
  ScriptType* params[paramCount];
  ScriptType* retType = v->returnType->referencedType;
  bool varargs = false;
  bool varargsFailReported = false;

  pushScope();
  getScope()->expectedReturnType = retType;

  for (uint32 i = 0; i < v->arguments.size(); i++) {
    FunctionParam* p = v->arguments.at(i);

    bool varargsParam = p->varargs;
    ScriptType* trueType;

    p->paramType->acceptVisit(this);

    if (varargsParam) {
      trueType = new ScriptArrayType(p->paramType->referencedType);
    } else {
      trueType = p->paramType->referencedType;
    }

    if (varargsParam && varargs && !varargsFailReported) {
      m_errors->error(p->location, "Function is declared with multiple variadic arguments");
      varargsFailReported = true;
    }

    params[i] = trueType;
    varargs |= varargsParam;

    pushSymbol(p->name->value, trueType);
  }

  FunctionSignature* ftype = FunctionSignature::create(retType, varargs, paramCount, params);
  pushSymbol(getScope(1), v->name->value, ftype);

  v->signature = ftype;

  for (Statement* stat : v->functionBody->statements) {
    stat->acceptVisit(this);
  }

  popScope();
  STATPUSH
}

void TypeResolver::acceptExprStatement(ExprStatement* v) {
  STATPUSH
  v->expression->acceptVisit(this);
  STATPOP
}

void TypeResolver::acceptStructPropertyDecl(StructPropertyDecl* v) {
  // Should not be called
}

void TypeResolver::acceptStructDecl(StructDecl* v) {
  STATPUSH

  const std::string name = m_strings->getstring(v->name->value);
  ScriptType* existing = m_lookup->lookupByName(name);

  if (existing) {
    m_errors->error(v->location, "Double declaration of struct type '%s'", name.c_str());
    STATPOP
    return;
  }

  const uint32 propCount = v->properties.size();
  StructProperty properties[propCount];

  for (uint32 i = 0; i < propCount; i++) {
    StructPropertyDecl* prop = v->properties.at(i);
    prop->propertyType->acceptVisit(this);

    std::string pname = m_strings->getstring(prop->name->value);
    ScriptType* ptype = prop->propertyType->referencedType;

    if (prop->value) {
      prop->value->acceptVisit(this);
      ScriptType* vtype = prop->value->resultType;

      if (!isAssignableTo(ptype, vtype)) {
        m_errors->error(prop->location,
          "Default value of property %s.%s is a %s and cannot be assigned to %s",
          name.c_str(),
          pname.c_str(),
          vtype->getTypeName(),
          ptype->getTypeName()
        );
      }
    }

    StructProperty* sProp = &properties[i];
    sProp->type = ptype;
    sProp->name = pname;
  }

  ScriptStructType* type = ScriptStructType::create(name, properties, propCount);
  m_lookup->emplaceType(type);

  STATPOP
}

void TypeResolver::acceptAssertStatement(AssertStatement* v) {
  if (v->message) {
    v->message->acceptVisit(this);

    ScriptType* msgType = v->message->resultType;

    if (msgType->kind() != TK_STRING) {
      m_errors->error(
        v->message->location,
        "assert statement message cannot be assigned to string"
      );
    }
  }

  v->condition->acceptVisit(this);

  ScriptType* condType = v->condition->resultType;
  PrimitiveScriptType* boolType = ConstTypes::BOOL();

  if (isAssignableTo(boolType, condType)) {
    return;
  }

  m_errors->error(
    v->condition->location,
    "assert statement condition cannot be assigned to boolean"
  );
}
