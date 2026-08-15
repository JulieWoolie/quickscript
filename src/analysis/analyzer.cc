#include "analyzer.h"

#include <bitset>
#include <functional>
#include <stdint.h>

#include "../types/ConstTypes.h"

#define STAT_PUSH ctx.pushStatement(v);
#define STAT_POP ctx.popStatement();

#define NOT_MAIN(name) \
  if (ctx.getScope()->getType() == SCOPE_MAIN && !ctx.wasWrongScopeReported()) { \
    ctx.getErrors().error(v->location, "%s not allowed here", name);\
    ctx.pushWrongScopeTypeReported(true);\
  } else {\
    ctx.pushWrongScopeTypeReported(false);\
  }

#define NOT_MAIN_RETURN(name) if (ctx.getScope()->getType() == SCOPE_MAIN) { ctx.getErrors().error(v->location, "%s not allowed here", name); return; }
#define NOT_MAIN_TRAILING ctx.popWrongScopeReported();

static Symbol* resolveReferencedSymbol(SemanticContext& ctx, Scope* start, Identifier* id, ScriptType* expectedType) {
  std::unordered_map<Node*, Symbol*>& cache = ctx.getSymbolLookup();
  if (cache.contains(id)) {
    return cache[id];
  }

  const stringid name = id->value;
  const typekind expectedKind = expectedType ? expectedType->kind() : TK_UNKNOWN;

  uint32 scores[10];
  Symbol* signatures[10];
  uint32 scoreLen = 0;

  for (Scope* scope = start; scope != nullptr; scope = scope->getParent()) {
    for (Symbol* symbol : scope->getSymbols()) {
      if (symbol->getName() != name) {
        continue;
      }

      const symboltype symType = symbol->stype();
      if (symType != SYM_LocalVar && symType != SYM_LocalFunc) {
        continue;
      }

      ScriptType* scriptType = symbol->getScriptType();
      if (!expectedType || expectedKind != TK_FUNC) {
        if (scriptType->kind() == TK_FUNC) {
          continue;
        }

        cache[id] = symbol;
        return symbol;
      }

      if (scriptType->kind() != TK_FUNC) {
        continue;
      }

      FunctionSignature* callingSign = static_cast<FunctionSignature*>(expectedType);
      FunctionSignature* funcSign = static_cast<FunctionSignature*>(scriptType);

      const int32 score = FunctionSignature::callSignatureMatches(callingSign, funcSign);
      if (score == SIGN_DOES_NOT_MATCH) {
        continue;
      }

      scores[scoreLen] = score;
      signatures[scoreLen] = symbol;
      scoreLen++;
    }
  }

  if (scoreLen == 0) {
    return nullptr;
  }
  if (scoreLen == 1) {
    return cache[id] = signatures[0];
  }

  uint32 highest = -1;
  Symbol* best = nullptr;

  for (uint32 idx = 0; idx < scoreLen; idx++) {
    const uint32 scr = scores[idx];

    if (scr <= highest) {
      continue;
    }

    highest = scr;
    best = signatures[idx];
  }

  cache[id] = best;
  return best;
}

static ScriptType* getArrayType(const SemanticContext& ctx, ScriptType* componentType) {
  if (!componentType) {
    return nullptr;
  }
  return ctx.getTypes().getArrayType(componentType);
}

static primitivekind parsedPrimitiveToTypeKind(parsedprimitivetype ppt) {
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

static ScriptType* evaluateTypeExpr(SemanticContext& ctx, TypeExpr* v) {
  switch (v->nodeKind()) {
    case AST_PrimitiveTypeExpr: {
      PrimitiveTypeExpr* pr = static_cast<PrimitiveTypeExpr*>(v);
      if (pr->primType == PPT_STRING) {
        return ConstTypes::STRING();
      }
      if (pr->primType == PPT_VOID) {
        return ConstTypes::VOID();
      }
      primitivekind pk = parsedPrimitiveToTypeKind(pr->primType);
      return ConstTypes::getPrimitiveType(pk);
    }
    case AST_ArrayTypeExpr: {
      ArrayTypeExpr* arr = static_cast<ArrayTypeExpr*>(v);
      ScriptType* cType = evaluateTypeExpr(ctx, arr->componentType);
      if (!cType) {
        return nullptr;
      }
      return getArrayType(ctx, cType);
    }
    case AST_TypeNameExpr: {
      TypeNameExpr* tn = static_cast<TypeNameExpr*>(v);
      const std::string typeName = ctx.getStrings().getstring(tn->typeName);
      return ctx.getTypes().lookupByName(typeName);
    }
    default:
      return nullptr;
  }
}

static void resolveTypeExpr(SemanticContext& ctx, TypeExpr* typeExpr) {
  switch (typeExpr->nodeKind()) {
    case AST_TypeNameExpr: {
      TypeNameExpr* v = static_cast<TypeNameExpr*>(typeExpr);
      std::string typeName = ctx.getStrings().getstring(v->typeName);
      v->referencedType = ctx.getTypes().lookupByName(typeName);

      if (!v->referencedType) {
        v->referencedType = ConstTypes::UNKNOWN();
        ctx.getErrors().error(v->location, "Unknown type '%s'", typeName.c_str());
        return;
      }

      Scope* global = ctx.getScope();
      Symbol* structSym = global->findSymbol(v->typeName, SYM_LocalStruct);

      if (structSym) {
        LocalStructSymbol* lss = static_cast<LocalStructSymbol*>(structSym);
        lss->used();
      }
      return;
    }
    case AST_ArrayTypeExpr: {
      ArrayTypeExpr* v = static_cast<ArrayTypeExpr*>(typeExpr);
      resolveTypeExpr(ctx, v->componentType);
      ScriptType* compType = v->componentType->referencedType;

      if (compType->kind() == TK_UNKNOWN) {
        v->referencedType = ConstTypes::UNKNOWN();
        return;
      }

      v->referencedType = getArrayType(ctx, compType);
      return;
    }
    case AST_PrimitiveTypeExpr: {
      PrimitiveTypeExpr* v = static_cast<PrimitiveTypeExpr*>(typeExpr);
      parsedprimitivetype ppt = v->primType;

      if (ppt == PPT_STRING) {
        v->referencedType = ConstTypes::STRING();
        return;
      }
      if (ppt == PPT_VOID) {
        v->referencedType = ConstTypes::VOID();
        Statement* stat = ctx.getCurrentStatement();

        if (stat->nodeKind() == AST_LexicalDeclaration) {
          ctx.getErrors().error(v->location, "'void' type not allowed here");
        }

        return;
      }

      primitivekind pk = parsedPrimitiveToTypeKind(ppt);
      v->referencedType = ConstTypes::getPrimitiveType(pk);
    }

    default:
      break;
  }
}

static bool everyBranchHasReturn(Statement* stat) {
  switch (stat->nodeKind()) {
    case AST_Block: {
      Block* b = static_cast<Block*>(stat);
      for (Statement* s : b->statements) {
        if (everyBranchHasReturn(s)) {
          return true;
        }
      }
      return false;
    }
    case AST_IfStatement: {
      const IfStatement* ifStatement = static_cast<IfStatement*>(stat);
      Statement* elseBody = ifStatement->elseBody;

      return everyBranchHasReturn(ifStatement->body)
          && (!elseBody || everyBranchHasReturn(elseBody));
    }
    case AST_ReturnStatement:
      return true;
    case AST_ForStatement:
      return everyBranchHasReturn(static_cast<ForStatement*>(stat)->loopBody);
    case AST_WhileStatement:
      return everyBranchHasReturn(static_cast<WhileStatement*>(stat)->body);
    default:
      return false;
  }
}

static void putCurrentStatementsDependOn(SemanticContext& ctx, Symbol* referenced) {
  DependencyGraph& graph = ctx.getDependencyGraph();
  std::vector<Statement*>& stack = ctx.getStatementStack();
  std::unordered_map<Node*, Symbol*>& symLookup = ctx.getSymbolLookup();

  for (auto it = stack.rbegin(); it != stack.rend(); ++it) {
    Statement* stat = *it;
    astnodetype kind = stat->nodeKind();

    if (kind != AST_LexicalDeclaration && kind != AST_FunctionDeclStatement) {
      continue;
    }

    Symbol* statSym = symLookup[stat];
    if (!statSym) {
      continue;
    }

    graph.addDependency(statSym, referenced);
  }
}

static void acceptExpr(SemanticContext& ctx, Expr* v);

static void acceptIdentifier(SemanticContext& ctx, Identifier* v) {
  ScriptType* expectedType = ctx.getExpectedType();
  Symbol* referenced = resolveReferencedSymbol(ctx, ctx.getScope(), v, expectedType);

  if (!referenced) {
    std::string_view name = ctx.getStrings().getview(v->value);

    ctx.getErrors().error(v->location, "Unknown symbol '%.*s'",
      PRINTVIEW(name)
    );

    v->resultType = ConstTypes::UNKNOWN();
    return;
  }

  putCurrentStatementsDependOn(ctx, referenced);

  // referenced->readUses++;
  v->resultType = referenced->getScriptType();
}

static void acceptCallExpr(SemanticContext& ctx, CallExpr* v) {
  const uint32 args = v->arguments.size();
  ScriptType* params[args];

  for (uint32 i = 0; i < args; i++) {
    Expr* e = v->arguments.at(i);
    acceptExpr(ctx, e);
    params[i] = e->resultType;
  }

  FunctionSignature sign = FunctionSignature(nullptr, false, args, params);

  ctx.pushExpectedType(&sign);
  acceptExpr(ctx, v->target);
  ctx.popExpectedType();

  ScriptType* targetType = v->target->resultType;
  if (targetType->kind() == TK_UNKNOWN) {
    v->resultType = ConstTypes::UNKNOWN();
    return;
  }

  if (targetType->kind() != TK_FUNC) {
    ctx.getErrors().error(v->location,
      "Expression does not return a callable function, but returns a %s",
      targetType->getTypeName()
    );

    v->resultType = targetType;
    return;
  }

  v->resultType = static_cast<FunctionSignature*>(targetType)->getReturnType();
}

static PropertySymbol* findPropSymbol(Scope* scope, const ScriptType* structType, const stringid propName) {
  while (scope) {
    for (Symbol* symbol : scope->getSymbols()) {
      if (symbol->getName() != propName || symbol->stype() != SYM_Property) {
        continue;
      }

      PropertySymbol* sps = static_cast<PropertySymbol*>(symbol);
      if (sps->getHolderType() != structType) {
        continue;
      }

      return sps;
    }

    scope = scope->getParent();
  }
  return nullptr;
}

static void acceptPropertyAccessExpr(SemanticContext& ctx, PropertyAccessExpr* v) {
  acceptExpr(ctx, v->target);
  ScriptType* resType = v->target->resultType;

  if (resType->kind() == TK_UNKNOWN) {
    v->resultType = ConstTypes::UNKNOWN();
    return;
  }

  if (!(resType->typeFlags() & TFLAG_PROPERTY_HOLDER)) {
    ctx.getErrors().error(v->location,
      "%s has no properties that can be accessed",
      resType->getTypeName()
    );

    v->resultType = ConstTypes::UNKNOWN();
    return;
  }

  std::string_view queriedProp = ctx.getStrings().getview(v->property->value);
  PropertySymbol* sps = findPropSymbol(ctx.getScope(), resType, v->property->value);

  if (!sps) {
    ScriptType* propertyType = resType->getPropertyType(queriedProp);

    if (propertyType) {
      Scope* global = ctx.getGlobalScope();
      NoFreeAllocator& alloc = ctx.getAllocator();

      // Native type, mark as used so no warnings appear
      sps = alloc.make<PropertySymbol>(resType, v->property->value, propertyType);
      sps->addFlags(SYMFLAG_USED);

      global->pushSymbol(sps);
    }
  }

  if (!sps) {
    ctx.getErrors().error(v->location,
      "No such property '%.*s' on %s",
      PRINTVIEW(queriedProp), resType->getTypeName()
    );
    v->resultType = ConstTypes::UNKNOWN();
    return;
  }

  v->resultType = sps->getScriptType();
  ctx.getSymbolLookup()[v->property] = sps;
}

static void acceptIndexAccessExpr(SemanticContext& ctx, IndexAccessExpr* v) {
  acceptExpr(ctx, v->index);
  ScriptType* indexType = v->index->resultType;

  acceptExpr(ctx, v->target);
  ScriptType* resultType = v->target->resultType;

  if (resultType->kind() == TK_UNKNOWN) {
    v->resultType = TK_UNKNOWN;
    return;
  }

  if (!(resultType->typeFlags() & TFLAG_INDEXABLE)) {
    ctx.getErrors().error(v->location, "Type %s cannot be indexed", resultType->getTypeName());
    v->resultType = ConstTypes::VOID();
    return;
  }

  if (indexType->kind() != TK_UNKNOWN && !isIntegerType(indexType)) {
    ctx.getErrors().error(v->index->location,
      "%s cannot be used to index an array or string",
      indexType->getTypeName()
    );
  }

  ScriptType* indexedType = resultType->getIndexReturnType();
  v->resultType = indexedType;
}

#define sgn(val) ((0 < val) - (val < 0))

static void acceptIntLiteral(IntLiteral* v) {
  const int64 val = v->value;
  parsedprimitivetype smallestFitting = PPT_INT64;

  const int8 sign = sgn(val);

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

static void acceptFloatLiteral(FloatLiteral* v) {
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

static void acceptObjectLiteral(SemanticContext& ctx, ObjectLiteral* v) {
  ScriptType* expectedType = ctx.getExpectedType();
  if (expectedType->kind() != TK_STRUCT) {
    ctx.getErrors().error(
      v->location,
      "Type %s cannot be initialized with an object literal",
      expectedType->getTypeName()
    );
    return;
  }

  ScriptStructType* structType = static_cast<ScriptStructType*>(expectedType);

  v->resultType = structType;

  Scope* scope = ctx.getScope();
  StringTable& table = ctx.getStrings();

  for (ObjectLiteralProperty* propExpr : v->properties) {
    stringid nameId = propExpr->propertyName->value;
    PropertySymbol* sym = findPropSymbol(scope, structType, nameId);

    if (!sym) {
      std::string_view propertyName = table.getview(nameId);

      ctx.getErrors().error(propExpr->location, "No such property named '%.*s' on struct %s",
        PRINTVIEW(propertyName),
        structType->getTypeName()
      );

      continue;
    }

    ctx.getSymbolLookup()[propExpr] = sym;

    ScriptType* propType = sym->getScriptType();

    ctx.pushExpectedType(propType);
    acceptExpr(ctx, propExpr->value);
    ctx.popExpectedType();

    ScriptType* valueType = propExpr->value->resultType;

    if (!isAssignableTo(propType, valueType)) {
      ctx.getErrors().error(propExpr->location,
        "Cannot assign value of type %s to property of type %s",
        valueType->getTypeName(),
        propType->getTypeName()
      );
    }
  }
}

static void acceptArrayLiteral(SemanticContext& ctx, ArrayLiteral* v) {
  ScriptType* type = ctx.getExpectedType();
  if (type->kind() != TK_ARRAY) {
    if (type->kind() != TK_UNKNOWN) {
      ctx.getErrors().error(
        v->location,
        "Type %s cannot be initialized with an array literal",
        type->getTypeName()
      );
    }

    v->resultType = ConstTypes::UNKNOWN();
    return;
  }

  ScriptType* componentType = static_cast<ScriptArrayType*>(type)->getComponentType();
  ctx.pushExpectedType(componentType);

  for (Expr* expr : v->values) {
    acceptExpr(ctx, expr);

    ScriptType* valType = expr->resultType;
    if (isAssignableTo(componentType, valType)) {
      continue;
    }
    if (valType->kind() == TK_UNKNOWN) {
      continue;
    }

    ctx.getErrors().error(expr->location,
      "Cannot use value of type %s in array of type %s",
      valType->getTypeName(),
      componentType->getTypeName()
    );
  }

  ctx.popExpectedType();
  v->resultType = type;
}

static bool isArrayTypeComparable(const ScriptArrayType* type) {
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
static ScriptType* getOpResultType(ScriptType* left, ScriptType* right, binaryop op) {
  const bool isAssign = op & BOP_ASSIGN_FLAG;

  if (op == BOP_ASSIGN) {
    if (isAssignableTo(left, right)) {
      return left;
    }
    return nullptr;
  }

  // Clear the assignment flag
  op &= ~BOP_ASSIGN_FLAG;

  const typekind leftKind = left->kind();
  const typekind rightKind = right->kind();

  switch (op) {
    case BOP_GT:
    case BOP_GTE:
    case BOP_LT:
    case BOP_LTE:
      if (isNumberType(left) && isNumberType(right)) {
        return ConstTypes::BOOL();
      }
      if (leftKind == TK_STRING && rightKind == TK_STRING) {
        return ConstTypes::BOOL();
      }
      if (leftKind == TK_ARRAY && rightKind == TK_ARRAY) {
        const ScriptArrayType* larr = static_cast<ScriptArrayType*>(left);
        const ScriptArrayType* rarr = static_cast<ScriptArrayType*>(right);

        if (larr != rarr || !isArrayTypeComparable(larr)) {
          return nullptr;
        }

        return ConstTypes::BOOL();
      }
      return nullptr;

    case BOP_EQ:
    case BOP_NEQ:
      if (leftKind == TK_PRIMITIVE && rightKind == TK_PRIMITIVE) {
        return ConstTypes::BOOL();
      }
      if (left == right) {
        return ConstTypes::BOOL();
      }
      return nullptr;

    case BOP_ADD:
      if (leftKind == TK_STRING) {
        return left;
      }
    case BOP_MUL:
      // Holy nesting, but kinda needed for pass through from above case
      if (op == BOP_MUL) {
        if (leftKind == TK_STRING) {
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
      if (leftKind != TK_PRIMITIVE || rightKind != TK_PRIMITIVE) {
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

static void checkAssignability(SemanticContext& ctx, Expr* expr) {
  switch (expr->nodeKind()) {
    case AST_Identifier: {
      Identifier* id = static_cast<Identifier*>(expr);
      Symbol* sym = resolveReferencedSymbol(ctx, ctx.getScope(), id, id->resultType);

      if (!sym) {
        return;
      }
      if (sym->stype() != SYM_LocalVar) {
        // TODO: Change this message lol
        ctx.getErrors().error(expr->location, "Cannot reassign whatever that is");
      }

      if (sym->getFlags() & SYMFLAG_CONST) {
        const std::string_view view = ctx.getStrings().getview(id->value);
        ctx.getErrors().error(expr->location, "Cannot reassign const variable '%.*s'",
          PRINTVIEW(view)
        );
      }

      static_cast<LocalVarSymbol*>(sym)->getWrites().push_back(expr->location);
      return;
    }

    case AST_PropertyAccessExpr: {
      const PropertyAccessExpr* prop = static_cast<PropertyAccessExpr*>(expr);
      const ScriptType* objType = prop->target->resultType;

      if (prop->resultType->kind() == TK_VOID) {
        // Property not found, don't try to check
        return;
      }

      if (objType->kind() == TK_ARRAY) {
        ctx.getErrors().error(expr->location, "Cannot mutate array length");
      }
      if (objType->kind() == TK_STRING) {
        ctx.getErrors().error(expr->location, "Cannot mutate string length");
      }

      if (objType->kind() == TK_STRUCT) {
        PropertySymbol* pSym = static_cast<PropertySymbol*>(ctx.getSymbolLookup()[prop->property]);

        if (!pSym) {
          return;
        }

        pSym->setWrites(pSym->getWrites() + 1);
      }

      return;
    }

    case AST_IndexAccessExpr: {
      const IndexAccessExpr* idx = static_cast<IndexAccessExpr*>(expr);
      const ScriptType* type = idx->target->resultType;

      if (type->kind() == TK_STRING) {
        ctx.getErrors().error(expr->location, "Cannot mutate strings");
      }

      return;
    }

    default:
      ctx.getErrors().error(expr->location, "Invalid left-hand-side expression; cannot be assigned to");
      break;
  }
}

static void acceptBinaryExpr(SemanticContext& ctx, BinaryExpr* v) {
  acceptExpr(ctx, v->lhs);
  acceptExpr(ctx, v->rhs);

  ScriptType* leftType = v->lhs->resultType;
  ScriptType* rightType = v->rhs->resultType;

  if (leftType->kind() == TK_UNKNOWN || rightType->kind() == TK_UNKNOWN) {
    v->resultType = ConstTypes::UNKNOWN();
    return;
  }

  const binaryop op = v->op;
  ScriptType* res = getOpResultType(leftType, rightType, op);

  if (!res) {
    ctx.getErrors().error(v->location,
      "Cannot use %s operator on %s and %s",
      binaryop_name(op), leftType->getTypeName(), rightType->getTypeName()
    );

    v->resultType = leftType;
    return;
  }

  if (op & BOP_ASSIGN_FLAG) {
    checkAssignability(ctx, v->lhs);
  }

  v->resultType = res;
}

static ScriptType* getUnaryOpResult(const unaryop op, ScriptType* type) {
  if (type->kind() != TK_PRIMITIVE) {
    return nullptr;
  }

  PrimitiveScriptType* p = static_cast<PrimitiveScriptType*>(type);

  switch (op) {
    case UOP_BIT_NOT:
      if (isIntegerType(p)) {
        return type;
      }
      return nullptr;

    case UOP_LOG_NOT:
      if (isIntegerType(p) || p->getPrimitiveType() == PK_BOOL) {
        return type;
      }
      return nullptr;

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

static void acceptUnaryExpr(SemanticContext& ctx, UnaryExpr* v) {
  acceptExpr(ctx, v->target);

  if (v->target->resultType->kind() == TK_UNKNOWN) {
    v->resultType = ConstTypes::UNKNOWN();
    return;
  }

  const unaryop op = v->op;
  ScriptType* resType = getUnaryOpResult(op, v->target->resultType);

  if (resType) {
    switch (op) {
      case UOP_PREDEC:
      case UOP_PREINC:
      case UOP_POSTDEC:
      case UOP_POSTINC:
        checkAssignability(ctx, v->target);
        break;
      default:
        break;
    }

    v->resultType = resType;
    return;
  }

  v->resultType = v->target->resultType;

  ctx.getErrors().error(v->location, "Cannot use %s operator on %s",
    unaryop_name(v->op),
    v->resultType->getTypeName()
  );
}

static void acceptTernaryExpr(SemanticContext& ctx, TernaryExpr* v) {
  acceptExpr(ctx, v->condition);

  ScriptType* condType = v->condition->resultType;

  if (condType->kind() != TK_UNKNOWN && !isAssignableTo(ConstTypes::BOOL(), condType)) {
    ctx.getErrors().error(v->condition->location, "%s is not assignable to a bool condition",
      v->condition->resultType->getTypeName()
    );
  }

  acceptExpr(ctx, v->left);
  acceptExpr(ctx, v->right);

  ScriptType* lType = v->left->resultType;
  ScriptType* rType = v->right->resultType;
  ScriptType* resultType = nullptr;

  if (lType->kind() == TK_UNKNOWN) {
    resultType = rType;
  } else if (rType->kind() == TK_UNKNOWN) {
    resultType = lType;
  } else {
    resultType = getCommonType(lType, rType);
  }

  if (!resultType) {
    ctx.getErrors().error(
      v->location,
      "Ternary operator left and right values have incompatible types: %s and %s",
      lType->getTypeName(),
      rType->getTypeName()
    );
    v->resultType = lType;
    return;
  }

  v->resultType = resultType;
}

static void acceptExpr(SemanticContext& ctx, Expr* v) {
  switch (v->nodeKind()) {
    case AST_Identifier:
      acceptIdentifier(ctx, static_cast<Identifier*>(v));
      break;
    case AST_CallExpr:
      acceptCallExpr(ctx, static_cast<CallExpr*>(v));
      break;
    case AST_PropertyAccessExpr:
      acceptPropertyAccessExpr(ctx, static_cast<PropertyAccessExpr*>(v));
      break;
    case AST_IndexAccessExpr:
      acceptIndexAccessExpr(ctx, static_cast<IndexAccessExpr*>(v));
      break;

    case AST_BooleanLiteral:
      v->resultType = ConstTypes::BOOL();
      break;
    case AST_CharLiteral:
      v->resultType = ConstTypes::INT8();
      break;
    case AST_StringLiteral:
      v->resultType = ConstTypes::STRING();
      break;
    case AST_IntLiteral:
      acceptIntLiteral(static_cast<IntLiteral*>(v));
      break;
    case AST_FloatLiteral:
      acceptFloatLiteral(static_cast<FloatLiteral*>(v));
      break;
    case AST_BinaryExpr:
      acceptBinaryExpr(ctx, static_cast<BinaryExpr*>(v));
      break;
    case AST_UnaryExpr:
      acceptUnaryExpr(ctx, static_cast<UnaryExpr*>(v));
      break;
    case AST_TernaryExpr:
      acceptTernaryExpr(ctx, static_cast<TernaryExpr*>(v));
      break;

    case AST_ArrayLiteral:
      acceptArrayLiteral(ctx, static_cast<ArrayLiteral*>(v));
      break;
    case AST_ObjectLiteral:
      acceptObjectLiteral(ctx, static_cast<ObjectLiteral*>(v));
      break;

    default:
      break;
  }
}

static void acceptStatement(SemanticContext& ctx, Statement* stat);

static void acceptBodyNoScope(SemanticContext& ctx, Statement* block) {
  if (block->nodeKind() == AST_Block) {
    Block* b = static_cast<Block*>(block);
    for (Statement* s : b->statements) {
      acceptStatement(ctx, s);
    }
    return;
  }
  acceptStatement(ctx, block);
}

static void createStructType(SemanticContext& ctx, StructDecl* v) {
  StringTable& strings = ctx.getStrings();
  TypeTable& types = ctx.getTypes();

  const std::string name = strings.getstring(v->name->value);
  const ScriptType* existing = types.lookupByName(name);

  if (existing) {
    ctx.getErrors().error(v->location, "Double declaration of struct type '%s'", name.c_str());
    STAT_POP
    return;
  }

  const uint32 propCount = v->properties.size();
  StructProperty properties[propCount];

  for (uint32 i = 0; i < propCount; i++) {
    const StructPropertyDecl* prop = v->properties.at(i);
    const std::string pName = strings.getstring(prop->name->value);

    ScriptType* propType = evaluateTypeExpr(ctx, prop->propertyType);

    StructProperty* sProp = &properties[i];
    sProp->type = propType;
    sProp->name = pName;
  }

  ScriptStructType* type = ScriptStructType::create(name, properties, propCount);
  types.emplaceType(type);

  v->type = type;

  Scope* scope = ctx.getScope();
  LocalStructSymbol* symb = ctx.getAllocator().make<LocalStructSymbol>(v->name->value, type, v);

  scope->pushSymbol(symb);
  ctx.getSymbolLookup()[v] = symb;
  ctx.getScopeLookup()[symb] = scope;
}

static void resolveMissingProperties(SemanticContext& ctx, const StructDecl* decl) {
  const std::vector<StructPropertyDecl*>& propertyDecls = decl->properties;
  ScriptStructType* type = decl->type;

  const uint32 pCount = propertyDecls.size();
  Scope* scope = ctx.getScope();

  LocalStructSymbol* lss = static_cast<LocalStructSymbol*>(scope->findSymbol(decl->name->value, SYM_LocalStruct));
  bool appendPropSym = lss->getScriptType() == decl->type;

  NoFreeAllocator& alloc = ctx.getAllocator();

  for (uint32 i = 0; i < pCount; i++) {
    StructPropertyDecl* propDecl = propertyDecls[i];
    StructProperty* typeProp = type->getProperty(i);
    typeProp->type = evaluateTypeExpr(ctx, propDecl->propertyType);

    if (!appendPropSym) {
      continue;
    }

    PropertySymbol* sym = alloc.make<PropertySymbol>(type, propDecl->name->value, typeProp->type);
    scope->pushSymbol(sym);
    ctx.getSymbolLookup()[propDecl] = sym;
    ctx.getScopeLookup()[sym] = scope;
  }
}


static bool isPossibleMainFunction(SemanticContext& ctx, FunctionDeclStatement* v) {
  StringTable& strings = ctx.getStrings();
  stringid mainId = strings.findId("main");

  // Even if mainId == EMPTY_STRING, there's surely no way a method would
  // have an empty string as an identifier
  if (v->name->value != mainId) {
    return false;
  }

  FunctionSignature* signature = v->signature;
  ScriptType* returnedType = signature->getReturnType();

  if (returnedType->kind() != TK_VOID && !isIntegerType(returnedType)) {
    return false;
  }

  const uint32 argsLen = signature->getArgumentsLength();
  if (argsLen == 0) {
    return true;
  }
  if (argsLen > 1) {
    return false;
  }

  ScriptType* argType = signature->getArgumentType(0);
  if (argType->kind() != TK_ARRAY) {
    return false;
  }

  ScriptType* componentType = static_cast<ScriptArrayType*>(argType)->getComponentType();
  return componentType == ConstTypes::STRING();
}

static void createFuncSignature(SemanticContext& ctx, FunctionDeclStatement* v, bool nested = false) {
  resolveTypeExpr(ctx, v->returnType);

  const uint32 paramCount = v->arguments.size();
  ScriptType* params[paramCount];
  ScriptType* returnType = v->returnType->referencedType;
  bool varargs = false;
  bool varargsFailReported = false;

  for (uint32 i = 0; i < v->arguments.size(); i++) {
    FunctionParam* p = v->arguments.at(i);

    bool varargsParam = p->varargs;
    ScriptType* trueType;

    resolveTypeExpr(ctx, p->paramType);

    if (varargsParam) {
      trueType = getArrayType(ctx, p->paramType->referencedType);
    } else {
      trueType = p->paramType->referencedType;
    }

    if (varargsParam && varargs && !varargsFailReported) {
      ctx.getErrors().error(p->location, "Function is declared with multiple variadic arguments");
      varargsFailReported = true;
    }

    params[i] = trueType;
    varargs |= varargsParam;
  }

  TypeTable& types = ctx.getTypes();
  FunctionSignature* ftype = types.getSignature(returnType, varargs, paramCount, params);
  v->signature = ftype;

  Scope* scope = ctx.getScope();
  const stringid funcName = v->name->value;

  for (const Symbol* sym : scope->getSymbols()) {
    if (sym->getName() != funcName) {
      continue;
    }
    if (sym->getScriptType() != ftype) {
      continue;
    }

    ctx.getErrors().error(v->location, "Duplicate function definition");
    return;
  }

  NoFreeAllocator& alloc = ctx.getAllocator();

  LocalFunction* lf = alloc.make<LocalFunction>(v->name->value, v, scope);
  lf->setNested(nested);

  ctx.pushLocalFunction(lf);

  LocalFuncSymbol* sym = alloc.make<LocalFuncSymbol>(*lf);
  scope->pushSymbol(sym);
  ctx.getScopeLookup()[sym] = scope;

  ctx.getSymbolLookup()[v] = sym;

  if (isPossibleMainFunction(ctx, v)) {
    ctx.getMainFuncCandidates().push_back(lf);
  }
}

static void reportUnused(const SemanticContext& ctx, Scope* scope) {
  StringTable& strings = ctx.getStrings();
  CompilerErrors& errors = ctx.getErrors();

  for (Symbol* sym : scope->getSymbols()) {
    if (sym->getFlags() & (SYMFLAG_BINDING | SYMFLAG_USED)) {
      continue;
    }

    std::string_view name = strings.getview(sym->getName());

    switch (sym->stype()) {
      case SYM_LocalVar: {
        LocalVarSymbol* lvs = static_cast<LocalVarSymbol*>(sym);
        const uint32 reads = lvs->getReads().size();
        const uint32 writes = lvs->getWrites().size();

        if (reads != 0 && writes != 0) {
          break;
        }

        if (lvs->getFlags() & SYMFLAG_CONST) {
          if (reads == 0) {
            errors.warn("Const variable '%.*s' is unused", PRINTVIEW(name));
          }
          break;
        }

        if (writes != 0 && reads == 0) {
          errors.warn("Variable '%.*s' is written to but never read", PRINTVIEW(name));
        }
        if (writes == 0 && reads == 0) {
          errors.warn("Variable '%.*s' is unused", PRINTVIEW(name));
        }

        break;
      }
      case SYM_LocalFunc:
        if (static_cast<LocalFuncSymbol*>(sym)->getCalls() == 0) {
          errors.warn("Function '%.*s' is never called", PRINTVIEW(name));
        }
        break;
      case SYM_LocalStruct:
        if (static_cast<LocalStructSymbol*>(sym)->getUses() == 0) {
          errors.warn("Struct '%.*s' is unused", PRINTVIEW(name));
        }
        break;
      case SYM_Property: {
        const PropertySymbol* sps = static_cast<PropertySymbol*>(sym);
        const uint32 writes = sps->getWrites();
        const uint32 reads = sps->getReads();

        if (writes != 0 && reads == 0) {
          errors.warn("Struct property '%.*s' is written to but never read", PRINTVIEW(name));
        }
        if (writes == 0 && reads == 0) {
          errors.warn("Struct property '%.*s' is unused", PRINTVIEW(name));
        }
        break;
      }
      default:
        break;
    }
  }
}

static void acceptBlock(SemanticContext& ctx, Block* v) {
  NOT_MAIN("Code Block")

  STAT_PUSH

  Scope* scope = ctx.pushScope(SCOPE_BLOCK);

  for (Statement* statement : v->statements) {
    acceptStatement(ctx, statement);
  }

  reportUnused(ctx, scope);
  ctx.popScope();

  NOT_MAIN_TRAILING
  STAT_POP
}

static bool isAllowedLoopOrIfBody(Statement* stat) {
  switch (stat->nodeKind()) {
    case AST_Block:
    case AST_ForStatement:
    case AST_WhileStatement:
    case AST_IfStatement:
    case AST_ReturnStatement:
    case AST_ControlFlowStatement:
    case AST_ExprStatement:
      return true;
    default:
      return false;
  }
}

static void acceptIfStatement(SemanticContext& ctx, IfStatement* v) {
  NOT_MAIN("If Statement")
  STAT_PUSH

  acceptExpr(ctx, v->condition);

  if (isAllowedLoopOrIfBody(v->body)) {
    acceptStatement(ctx, v->body);
  } else {
    ctx.getErrors().error(v->location, "Statement not allowed as if statement's body");
  }

  if (v->elseBody) {
    if (isAllowedLoopOrIfBody(v->elseBody)) {
      acceptStatement(ctx, v->elseBody);
    } else {
      ctx.getErrors().error(v->location, "Statement not allowed as if statement's else statement");
    }
  }

  NOT_MAIN_TRAILING
  STAT_POP
}

static void acceptForStatement(SemanticContext& ctx, ForStatement* v) {
  NOT_MAIN("For Loop")

  STAT_PUSH
  Scope* scope = ctx.pushScope(SCOPE_LOOP, v->label ? v->label->value : EMPTY_STRING);

  acceptStatement(ctx, v->first);
  acceptExpr(ctx, v->second);
  acceptExpr(ctx, v->third);

  acceptBodyNoScope(ctx, v->loopBody);

  reportUnused(ctx, scope);
  ctx.popScope();

  NOT_MAIN_TRAILING
  STAT_POP
}

static void acceptLexicalDeclaration(SemanticContext& ctx, LexicalDeclaration* v) {
  STAT_PUSH;

  resolveTypeExpr(ctx, v->typeExpr);
  ScriptType* declType = v->typeExpr->referencedType;
  const stringid nameId = v->variableName->value;

  Scope* scope = ctx.getScope();
  if (scope->getType() == SCOPE_MAIN) {
    ctx.getGlobalVariables().push_back(v);
  }

  ctx.getAllVariables().push_back(v);

  if (scope->findVariable(nameId)) {
    ctx.getErrors().error(v->location, "Duplicate variable definition");
    STAT_POP
    return;
  }

  NoFreeAllocator& alloc = ctx.getAllocator();
  const uint64 memSize = declType->stackSizeBytes();

  LocalVarSymbol* lvs = alloc.make<LocalVarSymbol>(nameId, declType, memSize, 0, v);
  scope->pushSymbol(lvs);
  ctx.getScopeLookup()[lvs] = scope;

  if (v->isConstDeclaration) {
    lvs->setFlags(SYMFLAG_CONST);
  }

  ctx.getSymbolLookup()[v] = lvs;

  if (v->value) {
    ctx.pushExpectedType(declType);
    acceptExpr(ctx, v->value);
    ctx.popExpectedType();

    ScriptType* valueType = v->value->resultType;

    if (valueType->kind() != TK_UNKNOWN && !isAssignableTo(declType, valueType)) {
      ctx.getErrors().error(v->location,
        "Value of type %s is not assignable to variable with type %s",
        valueType->getTypeName(),
        declType->getTypeName()
      );
    }
  } else if (v->isConstDeclaration) {
    std::string_view view = ctx.getStrings().getview(nameId);
    ctx.getErrors().error(v->location, "Const variable '%.*s' has no value", PRINTVIEW(view));
  }

  STAT_POP;
}

static void acceptWhileStatement(SemanticContext& ctx, WhileStatement* v) {
  NOT_MAIN(v->doWhile ? "Do While Loop" : "While Loop")

  STAT_PUSH
  Scope* scope = ctx.pushScope(SCOPE_LOOP, v->label ? v->label->value : EMPTY_STRING);

  if (v->doWhile) {
    acceptBodyNoScope(ctx, v->body);
    acceptExpr(ctx, v->condition);
  } else {
    acceptExpr(ctx, v->condition);
    acceptBodyNoScope(ctx, v->body);
  }

  reportUnused(ctx, scope);
  ctx.popScope();

  NOT_MAIN_TRAILING
  STAT_POP
}

static void acceptControlFlowStatement(const SemanticContext& ctx, ControlFlowStatement* v) {
  NOT_MAIN_RETURN(v->type == CFT_BREAK ? "Break" : "Continue")

  //
  // 1. Go up the scope stack until you find a loop
  // 2. If statement has label => check that loop has matching label
  //    If label matches => exit
  //    Else => keep going up the stack
  //
  // 3. If statement doesn't have label => exit
  //
  // 4. If either end of stack reached, or we reached a SCOPE_FUNC or
  //    SCOPE_MAIN scope, then stop searching.
  //
  // 5. If a loop was found but it didn't have a matching label => report that
  // 6. Report that control flow statement was used outside of a loop
  //

  Scope* scope = ctx.getScope();
  bool loopFound = false;

  while (scope) {
    scopetype stype = scope->getType();
    if (stype == SCOPE_FUNCTION || stype == SCOPE_MAIN) {
      break;
    }

    if (stype != SCOPE_LOOP) {
      scope = scope->getParent();
      continue;
    }

    loopFound = true;

    if (!v->label || v->label->value == scope->getLoopLabel()) {
      return;
    }

    scope = scope->getParent();
  }

  if (loopFound) {
    const std::string_view view = ctx.getStrings().getview(v->label->value);
    ctx.getErrors().error(v->location, "No loop with label '%.*s'",
      static_cast<int32>(view.length()), view.data()
    );
    return;
  }

  ctx.getErrors().error(v->location, "Control flow statement used outside of loop");
}

static void acceptReturnStatement(SemanticContext& ctx, ReturnStatement* v) {
  NOT_MAIN_RETURN("Return statement")

  ScriptType* expected = ctx.getScope()->getExpectedReturnType();

  if (!v->value) {
    if (!expected || expected->kind() == TK_VOID) {
      return;
    }

    ctx.getErrors().error(
      v->location,
      "Function expects return value with %s, cannot return nothing",
      expected->getTypeName()
    );

    return;
  }

  STAT_PUSH
  acceptExpr(ctx, v->value);

  ScriptType* rtype = v->value->resultType;

  if (!expected || isAssignableTo(expected, rtype)) {
    STAT_POP
    return;
  }

  if (expected->kind() == TK_VOID) {
    ctx.getErrors().error(v->location, "Cannot return a value in a void method");
    STAT_POP
    return;
  }

  if (rtype->kind() == TK_UNKNOWN) {
    STAT_POP
    return;
  }

  ctx.getErrors().error(v->location,
    "Returned value of type %s cannot be assigned to expected type %s",
    rtype->getTypeName(),
    expected->getTypeName()
  );

  STAT_POP
}

static void acceptFunctionDeclStatement(SemanticContext& ctx, FunctionDeclStatement* v) {
  STAT_PUSH

  Scope* parentScope = ctx.getScope();
  if (parentScope->getType() != SCOPE_MAIN) {
    createFuncSignature(ctx, v, true);
  }

  Scope* scope = ctx.pushScope(SCOPE_FUNCTION);
  scope->setExpectedReturnType(v->returnType->referencedType);

  FunctionSignature* sign = v->signature;
  const uint32 args = v->arguments.size();

  NoFreeAllocator& alloc = ctx.getAllocator();

  for (uint32 i = 0; i < args; i++) {
    ScriptType* signType = sign->getArgumentType(i);
    FunctionParam* arg = v->arguments[i];

    const uint64 memSize = signType->stackSizeBytes();

    LocalVarSymbol* lvs = alloc.make<LocalVarSymbol>(arg->name->value, signType, memSize, 0, arg);
    lvs->addFlags(SYMFLAG_FUNC_ARG);

    scope->pushSymbol(lvs);

    ctx.getScopeLookup()[lvs] = scope;
    ctx.getSymbolLookup()[arg] = lvs;
  }

  acceptBodyNoScope(ctx, v->functionBody);

  if (sign->getReturnType()->kind() != TK_VOID && !everyBranchHasReturn(v->functionBody)) {
    ctx.getErrors().error(v->location, "Non-void function has no return value");
  }

  reportUnused(ctx, scope);
  ctx.popScope();

  STAT_POP
}

static void acceptExprStatement(SemanticContext& ctx, ExprStatement* v) {
  NOT_MAIN("Expression")
  STAT_PUSH
  acceptExpr(ctx, v->expression);
  NOT_MAIN_TRAILING
  STAT_POP
}

static void acceptStructDecl(SemanticContext& ctx, StructDecl* v) {
  STAT_PUSH

  Scope* scope = ctx.getScope();
  if (scope->getType() != SCOPE_MAIN) {
    ctx.getErrors().error(v->location, "Structs can only be declared in the global scope");
    STAT_POP
    return;
  }

  ctx.getDeclaredStructs().push_back(v);

  const uint32 propCount = v->properties.size();
  conststring typeName = v->type->getTypeName();

  for (uint32 i = 0; i < propCount; i++) {
    StructPropertyDecl* prop = v->properties.at(i);
    StructProperty* typeProp = v->type->getProperty(i);

    std::string& propertyName = typeProp->name;
    ScriptType* propertyType = typeProp->type;

    if (prop->value) {
      acceptExpr(ctx, prop->value);
      ScriptType* valueType = prop->value->resultType;

      if (valueType->kind() == TK_UNKNOWN) {
        continue;
      }

      if (!isAssignableTo(propertyType, valueType)) {
        ctx.getErrors().error(prop->location,
          "Cannot assign %s value to property %s %s.%s",
          valueType->getTypeName(),
          propertyType->getTypeName(),
          typeName,
          propertyName.c_str()
        );
      }
    }
  }

  STAT_POP
}

static void acceptAssertStatement(SemanticContext& ctx, const AssertStatement* v) {
  acceptExpr(ctx, v->condition);
  ScriptType* condType = v->condition->resultType;

  if (condType->kind() != TK_UNKNOWN && !isAssignableTo(ConstTypes::BOOL(), condType)) {
    ctx.getErrors().error(
      v->condition->location,
      "assert statement condition cannot be assigned to boolean"
    );
  }

  if (v->message) {
    acceptExpr(ctx, v->message);

    const ScriptType* msgType = v->message->resultType;
    const typekind msgKind = msgType->kind();

    if (msgKind != TK_UNKNOWN && msgKind != TK_STRING) {
      ctx.getErrors().error(
        v->message->location,
        "assert statement message cannot be assigned to string"
      );
    }
  }
}

static void acceptStatement(SemanticContext& ctx, Statement* stat) {
  switch (stat->nodeKind()) {
    case AST_Block:
      acceptBlock(ctx, static_cast<Block*>(stat));
      break;
    case AST_IfStatement:
      acceptIfStatement(ctx, static_cast<IfStatement*>(stat));
      break;
    case AST_ForStatement:
      acceptForStatement(ctx, static_cast<ForStatement*>(stat));
      break;
    case AST_LexicalDeclaration:
      acceptLexicalDeclaration(ctx, static_cast<LexicalDeclaration*>(stat));
      break;
    case AST_WhileStatement:
      acceptWhileStatement(ctx, static_cast<WhileStatement*>(stat));
      break;
    case AST_ControlFlowStatement:
      acceptControlFlowStatement(ctx, static_cast<ControlFlowStatement*>(stat));
      break;
    case AST_ReturnStatement:
      acceptReturnStatement(ctx, static_cast<ReturnStatement*>(stat));
      break;
    case AST_FunctionDeclStatement:
      acceptFunctionDeclStatement(ctx, static_cast<FunctionDeclStatement*>(stat));
      break;
    case AST_ExprStatement:
      acceptExprStatement(ctx, static_cast<ExprStatement*>(stat));
      break;
    case AST_StructDecl:
      acceptStructDecl(ctx, static_cast<StructDecl*>(stat));
      break;
    case AST_AssertStatement:
      acceptAssertStatement(ctx, static_cast<AssertStatement*>(stat));
      break;
    default:
      break;
  }
}

static void addDefaultSymbols(SemanticContext& ctx, Scope* scope) {
  NoFreeAllocator& alloc = ctx.getAllocator();
  StringTable& strings = ctx.getStrings();

  stringid lengthId = strings.allocate("length");

  PropertySymbol* stringLength = alloc.make<PropertySymbol>(ConstTypes::STRING(), lengthId, ConstTypes::UINT32());
  stringLength->addFlags(SYMFLAG_BINDING);

  scope->pushSymbol(stringLength);
}

static void reportInvalidDependencies(SemanticContext& ctx, DependencyGraph& graph, Symbol* sym) {
  if (!graph.hasDependencies(sym)) {
    return;
  }

  const std::vector<Symbol*>& dependsOn = graph.getDependencies(sym);

  Location loc;
  symboltype stype = sym->stype();

  switch (stype) {
    case SYM_LocalVar:
      loc = static_cast<LocalVarSymbol*>(sym)->getDecl()->location;
      break;
    case SYM_LocalFunc:
      loc = static_cast<LocalFuncSymbol*>(sym)->getFunction().getDecl()->location;
      break;
    default:
      break;
  }

  for (Symbol* dependentSym : dependsOn) {
    if (dependentSym->getFlags() & SYMFLAG_MID_INIT) {
      std::string_view view = ctx.getStrings().getview(dependentSym->getName());
      ctx.getErrors().error(loc, "Accessing '%.*s' before its initialized", PRINTVIEW(view));
      continue;
    }
    reportInvalidDependencies(ctx, graph, dependentSym);
  }
}

static void checkForInvalidDependencies(SemanticContext& ctx) {
  DependencyGraph& graph = ctx.getDependencyGraph();
  std::unordered_map<Node*, Symbol*>& lookup = ctx.getSymbolLookup();
  std::vector<LexicalDeclaration*>& allVars = ctx.getAllVariables();

  if (allVars.empty()) {
    return;
  }

  for (LexicalDeclaration* gvar : allVars) {
    LocalVarSymbol* sym = static_cast<LocalVarSymbol*>(lookup[gvar]);
    sym->addFlags(SYMFLAG_MID_INIT);
  }

  for (LexicalDeclaration* gvar : allVars) {
    LocalVarSymbol* sym = static_cast<LocalVarSymbol*>(lookup[gvar]);

    if (!graph.hasDependencies(sym)) {
      sym->removeFlags(SYMFLAG_MID_INIT);
      continue;
    }

    reportInvalidDependencies(ctx, graph, sym);
    sym->removeFlags(SYMFLAG_MID_INIT);
  }
}

void runSemanticAnalysis(ScriptFileStatement* v, SemanticContext& ctx) {
  STAT_PUSH
  Scope* scope = ctx.pushScope(SCOPE_MAIN);
  ctx.setGlobalScope(scope);

  addDefaultSymbols(ctx, scope);

  scope->setExpectedReturnType(nullptr);

  std::vector<StructDecl*> structDeclarations;
  std::vector<FunctionDeclStatement*> functionDecls;

  for (Statement* s : v->statements) {
    astnodetype kind = s->nodeKind();
    switch (kind) {
      case AST_StructDecl:
        structDeclarations.push_back(static_cast<StructDecl*>(s));
        break;
      case AST_FunctionDeclStatement:
        functionDecls.push_back(static_cast<FunctionDeclStatement*>(s));
        break;
      default:
        break;
    }
  }

  for (StructDecl* decl : structDeclarations) {
    createStructType(ctx, decl);
  }
  for (StructDecl* decl : structDeclarations) {
    resolveMissingProperties(ctx, decl);
  }

  for (FunctionDeclStatement* decl : functionDecls) {
    createFuncSignature(ctx, decl);
  }

  for (Statement* s : v->statements) {
    acceptStatement(ctx, s);
  }

  checkForInvalidDependencies(ctx);

  std::vector<LocalFunction*>& mainCandidates = ctx.getMainFuncCandidates();
  if (mainCandidates.empty()) {
    ctx.getErrors().warn(
      "No possible main(string[]) functions found. Script will have no entry point"
    );
  } else {
    if (mainCandidates.size() > 1) {
      ctx.getErrors().warn("Multiple main functions found. First one will be used");
    }

    LocalFunction* main = mainCandidates.at(0);
    LocalFuncSymbol* mainSym = static_cast<LocalFuncSymbol*>(ctx.getSymbolLookup()[main->getDecl()]);

    if (mainSym->getCalls() != 0) {
      ctx.getErrors().warn("Main function called manually");
    }

    // Prevent "'main' not used" warnings
    mainSym->onCalled();
  }

  reportUnused(ctx, scope);
  ctx.popScope();
  STAT_POP
}
