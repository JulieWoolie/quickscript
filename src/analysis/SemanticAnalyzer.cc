#include "SemanticAnalyzer.h"

#include <bitset>
#include <functional>
#include <stdint.h>

#include "../types/ConstTypes.h"

#define STATPUSH m_statementStack.push_back(v);
#define STATPOP m_statementStack.pop_back();

#define NOT_MAIN(name) \
  if (getScope()->getType() == SCOPE_MAIN && !wasWrongScopeReported()) { \
    m_errors->error(v->location, "%s not allowed here", name);\
    pushWrongScopeTypeReported(true);\
  } else {\
    pushWrongScopeTypeReported(false);\
  }

#define NOT_MAINR(name) if (getScope()->getType() == SCOPE_MAIN) { m_errors->error(v->location, "%s not allowed here", name); return; }
#define NOT_MAIN_TRAILING popWrongScopeReported();

#define PRINTVIEW(x) static_cast<int>(x.length()), x.data()

Scope::Scope(const stringid label, const scopetype type)
  : m_currentLabel(label), m_scopeType(type)
{

}

Symbol* Scope::pushSymbol(stringid name, ScriptType* type, symboltype lexType, uint32 flags) {
  uint64 off = m_size;
  Symbol* sym = &m_symbols.emplace_back(name, lexType, flags, off, type, 0, 0);

  m_size += type->stackSizeBytes();

  return sym;
}

Symbol* Scope::findVariable(stringid name) {
  for (Symbol& sym : m_symbols) {
    if (sym.name != name) {
      continue;
    }
    if (sym.stype != SYM_VAR && sym.stype != SYM_CONST) {
      continue;
    }
    return &sym;
  }
  return nullptr;
}

std::vector<Symbol>* Scope::getSymbols() {
  return &m_symbols;
}

ScriptType* Scope::getExpectedReturnType() const {
  return m_expectedReturnType;
}

void Scope::setExpectedReturnType(ScriptType* type) {
  m_expectedReturnType = type;
}

stringid Scope::getLabel() const {
  return m_currentLabel;
}

scopetype Scope::getType() const {
  return m_scopeType;
}

Symbol* Scope::findSymbol(const stringid name, symboltype st) {
  for (Symbol& sym : m_symbols) {
    if (sym.name != name) {
      continue;
    }
    if (st != SYM_NIL && sym.stype != st) {
      continue;
    }
    return &sym;
  }
  return nullptr;
}

void SemanticAnalyzer::popScope() {
  Scope* scope = getScope();

  for (Symbol& sym : *scope->getSymbols()) {
    if (sym.flags & SYMFLAG_BINDING || (sym.readUses != 0 && sym.writeUses != 0)) {
      continue;
    }

    std::string_view name = m_strings->getview(sym.name);

    switch (sym.stype) {
      case SYM_CONST:
        if (sym.readUses == 0) {
          m_errors->warn("Const variable '%.*s' is unused", PRINTVIEW(name));
        }
        break;
      case SYM_VAR:
        if (sym.writeUses != 0 && sym.readUses == 0) {
          m_errors->warn("Variable '%.*s' is written to but never read", PRINTVIEW(name));
        }
        if (sym.writeUses == 0 && sym.readUses == 0) {
          m_errors->warn("Variable '%.*s' is unused", PRINTVIEW(name));
        }
        break;
      case SYM_FUNC:
        if (sym.readUses == 0) {
          m_errors->warn("Function '%.*s' is never called", PRINTVIEW(name));
        }
        break;
      case SYM_STRUCT:
        if (sym.readUses == 0) {
          m_errors->warn("Struct '%.*s' is unused", PRINTVIEW(name));
        }
        break;
      case SYM_PROP:
        if (sym.writeUses != 0 && sym.readUses == 0) {
          m_errors->warn("Struct property '%.*s' is written to but never read", PRINTVIEW(name));
        }
        if (sym.writeUses == 0 && sym.readUses == 0) {
          m_errors->warn("Struct property '%.*s' is unused", PRINTVIEW(name));
        }
        break;
      default:
        break;
    }
  }

  m_scopes.pop_back();
}

Scope* SemanticAnalyzer::pushScope(scopetype stype, stringid label) {
  Scope& scope = m_scopes.emplace_back(label, stype);

  if (m_scopes.size() > 1) {
    Scope* parent = &m_scopes.back() - 1;
    scope.setExpectedReturnType(parent->getExpectedReturnType());
  }

  return &scope;
}

Scope* SemanticAnalyzer::getScope(uint32 off) {
  if (off >= m_scopes.size()) {
    return nullptr;
  }
  return &m_scopes.back() - off;
}


SemanticAnalyzer::SemanticAnalyzer(
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

void SemanticAnalyzer::acceptTypeNameExpr(TypeNameExpr* v) {
  std::string typeName = m_strings->getstring(v->typeName);
  v->referencedType = m_lookup->lookupByName(typeName);

  if (!v->referencedType) {
    v->referencedType = ConstTypes::VOID();
    m_errors->error(v->location, "Unknown type '%s'", typeName.c_str());
    return;
  }

  Scope* global = &m_scopes[0];
  Symbol* structSym = global->findSymbol(v->typeName, SYM_STRUCT);

  if (structSym) {
    structSym->readUses++;
  }
}

void SemanticAnalyzer::acceptArrayTypeExpr(ArrayTypeExpr* v) {
  v->componentType->acceptVisit(this);
  ScriptType* compType = v->componentType->referencedType;

  if (!compType) {
    compType = ConstTypes::VOID();
  }

  v->referencedType = getArrayType(compType);
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

void SemanticAnalyzer::acceptPrimitiveTypeExpr(PrimitiveTypeExpr* v) {
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

Symbol* SemanticAnalyzer::resolveReferencedSymbol(stringid name, ScriptType* expectedType) {
  typekind expectedKind = expectedType ? expectedType->kind() : TK_NIL;

  uint32 scores[10];
  Symbol* signatures[10];
  uint32 scorerlen = 0;

  for (uint32 i = 0; i < m_scopes.size(); i++) {
    Scope* scope = getScope(i);
    std::vector<Symbol>* symbols = scope->getSymbols();

    for (Symbol& symbol : *symbols) {
      if (symbol.name != name) {
        continue;
      }

      ScriptType* stype = symbol.scriptType;
      if (!expectedType || expectedKind != TK_FUNC) {
        if (stype->kind() == TK_FUNC) {
          continue;
        }
        return &symbol;
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
      signatures[scorerlen] = &symbol;
      scorerlen++;
    }
  }

  if (scorerlen == 0) {
    return nullptr;
  }
  if (scorerlen == 1) {
    return signatures[0];
  }

  int32 highest = -1;
  Symbol* best = nullptr;

  for (uint32 idx = 0; idx < scorerlen; idx++) {
    int32 scr = scores[idx];

    if (scr <= highest) {
      continue;
    }

    highest = scr;
    best = signatures[idx];
  }

  return best;
}

bool SemanticAnalyzer::everyBranchHasReturn(Statement* stat) {
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
    case AST_DoWhileStatement:
      return everyBranchHasReturn(static_cast<DoWhileStatement*>(stat)->body);
    default:
      return false;
  }
}

void SemanticAnalyzer::pushWrongScopeTypeReported(bool reported) {
  m_wrongScopeReports.push_back(reported);
}

void SemanticAnalyzer::popWrongScopeReported() {
  if (m_wrongScopeReports.empty()) {
    return;
  }
  m_wrongScopeReports.pop_back();
}

bool SemanticAnalyzer::wasWrongScopeReported() const {
  if (m_wrongScopeReports.empty()) {
    return false;
  }
  return m_wrongScopeReports.back();
}

void SemanticAnalyzer::acceptIdentifier(Identifier* v) {
  ScriptType* expectedType = nullptr;
  if (!m_expectedTypes.empty()) {
    expectedType = m_expectedTypes.back();
  }

  Symbol* referenced = resolveReferencedSymbol(v->value, expectedType);
  if (!referenced) {
    std::string_view name = m_strings->getview(v->value);

    m_errors->error(v->location, "Unknown symbol '%.*s'",
      PRINTVIEW(name)
    );

    v->resultType = ConstTypes::VOID();
    return;
  }

  referenced->readUses++;
  v->resultType = referenced->scriptType;
}

void SemanticAnalyzer::acceptCallExpr(CallExpr* v) {
  const uint32 args = v->arguments.size();
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

void SemanticAnalyzer::acceptPropertyAccessExpr(PropertyAccessExpr* v) {
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

void SemanticAnalyzer::acceptIndexAccessExpr(IndexAccessExpr* v) {
  v->index->acceptVisit(this);
  ScriptType* indexType = v->index->resultType;

  v->target->acceptVisit(this);
  ScriptType* resultType = v->target->resultType;

  if (!(resultType->typeFlags() & TFLAG_INDEXABLE)) {
    m_errors->error(v->location, "Type %s cannot be indexed", resultType->getTypeName());
    v->resultType = ConstTypes::VOID();
    return;
  }

  if (!isIntegerType(indexType)) {
    m_errors->error(v->index->location,
      "%s cannot be used to index an array or string",
      indexType->getTypeName()
    );
  }

  ScriptType* indexedType = resultType->getIndexReturnType();
  v->resultType = indexedType;
}

void SemanticAnalyzer::acceptBooleanLiteral(BooleanLiteral* v) {
  v->resultType = ConstTypes::BOOL();
}

void SemanticAnalyzer::acceptCharLiteral(CharLiteral* v) {
  v->resultType = ConstTypes::INT8();
}

void SemanticAnalyzer::acceptStringLiteral(StringLiteral* v) {
  v->resultType = ConstTypes::STRING();
}

#define sgn(val) ((0 < val) - (val < 0))

void SemanticAnalyzer::acceptIntLiteral(IntLiteral* v) {
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

void SemanticAnalyzer::acceptFloatLiteral(FloatLiteral* v) {
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

void SemanticAnalyzer::acceptObjectLiteral(ObjectLiteral* v) {
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

void SemanticAnalyzer::acceptObjectLiteralProperty(ObjectLiteralProperty* v) {

}

void SemanticAnalyzer::acceptArrayLiteral(ArrayLiteral* v) {
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
ScriptType* SemanticAnalyzer::getOpResultType(ScriptType* left, ScriptType* right, binaryop op) const {
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

void SemanticAnalyzer::acceptBodyNoScope(Statement* block) {
  if (block->nodeKind() == AST_Block) {
    Block* b = static_cast<Block*>(block);
    for (Statement* s : b->statements) {
      s->acceptVisit(this);
    }
    return;
  }
  block->acceptVisit(this);
}

void SemanticAnalyzer::createStructType(StructDecl* v) {
  const std::string name = m_strings->getstring(v->name->value);
  const ScriptType* existing = m_lookup->lookupByName(name);

  if (existing) {
    m_errors->error(v->location, "Double declaration of struct type '%s'", name.c_str());
    STATPOP
    return;
  }

  const uint32 propCount = v->properties.size();
  StructProperty properties[propCount];

  for (uint32 i = 0; i < propCount; i++) {
    const StructPropertyDecl* prop = v->properties.at(i);
    const std::string pName = m_strings->getstring(prop->name->value);

    ScriptType* propType = resolveTypeExpr(prop->propertyType);

    StructProperty* sProp = &properties[i];
    sProp->type = propType;
    sProp->name = pName;
  }

  ScriptStructType* type = ScriptStructType::create(name, properties, propCount);
  m_lookup->emplaceType(type);

  v->type = type;

  Scope* scope = getScope();
  scope->pushSymbol(v->name->value, type, SYM_STRUCT);
}

void SemanticAnalyzer::resolveMissingProperties(const StructDecl* decl) {
  const std::vector<StructPropertyDecl*>& propertyDecls = decl->properties;
  ScriptStructType* type = decl->type;

  const uint32 pCount = propertyDecls.size();
  Scope* scope = getScope();

  for (uint32 i = 0; i < pCount; i++) {
    StructPropertyDecl* propDecl = propertyDecls[i];
    StructProperty* typeProp = type->getProperty(i);
    typeProp->type = resolveTypeExpr(propDecl->propertyType);

    std::string symbolName;
    symbolName.append(type->getTypeName());
    symbolName.append(".");
    symbolName.append(typeProp->name);

    const stringid symId = m_strings->allocate(symbolName);
    scope->pushSymbol(symId, typeProp->type, SYM_PROP);
  }
}

void SemanticAnalyzer::createFuncSignature(FunctionDeclStatement* v) {
  v->returnType->acceptVisit(this);

  const uint32 paramCount = v->arguments.size();
  ScriptType* params[paramCount];
  ScriptType* returnType = v->returnType->referencedType;
  bool varargs = false;
  bool varargsFailReported = false;

  for (uint32 i = 0; i < v->arguments.size(); i++) {
    FunctionParam* p = v->arguments.at(i);

    bool varargsParam = p->varargs;
    ScriptType* trueType;

    p->paramType->acceptVisit(this);

    if (varargsParam) {
      trueType = getArrayType(p->paramType->referencedType);
    } else {
      trueType = p->paramType->referencedType;
    }

    if (varargsParam && varargs && !varargsFailReported) {
      m_errors->error(p->location, "Function is declared with multiple variadic arguments");
      varargsFailReported = true;
    }

    params[i] = trueType;
    varargs |= varargsParam;
  }

  std::string funcSignStr;
  FunctionSignature::composeName(funcSignStr, returnType, paramCount, params);

  FunctionSignature* ftype = static_cast<FunctionSignature*>(m_lookup->lookupByName(funcSignStr));

  if (!ftype) {
    ftype = FunctionSignature::create(returnType, varargs, paramCount, params);
    m_lookup->emplaceType(ftype);
  }

  v->signature = ftype;

  Scope* scope = getScope();
  const stringid funcName = v->name->value;

  for (const Symbol& sym : *scope->getSymbols()) {
    if (sym.name != funcName) {
      continue;
    }
    if (sym.scriptType != ftype) {
      continue;
    }

    m_errors->error(v->location, "Duplicate function definition");
    return;
  }

  scope->pushSymbol(v->name->value, ftype, SYM_FUNC);
}

ScriptType* SemanticAnalyzer::resolveTypeExpr(TypeExpr* v) {
  astnodetype kind = v->nodeKind();

  switch (kind) {
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
      ScriptType* cType = resolveTypeExpr(arr->componentType);
      if (!cType) {
        return nullptr;
      }
      return getArrayType(cType);
    }
    case AST_TypeNameExpr: {
      TypeNameExpr* tn = static_cast<TypeNameExpr*>(v);
      const std::string typeName = m_strings->getstring(tn->typeName);
      return m_lookup->lookupByName(typeName);
    }
    default:
      return nullptr;
  }
}

ScriptType* SemanticAnalyzer::getArrayType(ScriptType* componentType) const {
  if (!componentType) {
    return nullptr;
  }

  std::string compName = componentType->getTypeName();
  compName.append("[]");

  ScriptType* found = m_lookup->lookupByName(compName);
  if (found) {
    return found;
  }

  found = new ScriptArrayType(componentType);
  m_lookup->emplaceType(found);

  return found;
}

void SemanticAnalyzer::checkAssignability(Expr* expr) {
  astnodetype kind = expr->nodeKind();

  switch (kind) {
    case AST_Identifier: {
      const Identifier* id = static_cast<Identifier*>(expr);
      Symbol* sym = resolveReferencedSymbol(id->value, id->resultType);

      if (!sym) {
        return;
      }

      if (sym->stype == SYM_CONST) {
        std::string_view view = m_strings->getview(id->value);
        m_errors->error(expr->location, "Cannot reassign const variable '%.*s'",
          PRINTVIEW(view)
        );
      } else if (sym->stype != SYM_VAR) {
        // TODO: Change this message lol
        m_errors->error(expr->location, "Cannot reassign whatever that is");
        sym->writeUses++;
      }

      return;
    }

    case AST_PropertyAccessExpr: {
      const PropertyAccessExpr* prop = static_cast<PropertyAccessExpr*>(expr);
      ScriptType* objType = prop->target->resultType;

      if (prop->resultType->kind() == TK_VOID) {
        // Property not found, don't try to check
        return;
      }

      if (objType->kind() == TK_ARRAY) {
        m_errors->error(expr->location, "Cannot mutate array length");
      }
      if (objType->kind() == TK_STRING) {
        m_errors->error(expr->location, "Cannot mutate string length");
      }

      if (objType->kind() == TK_STRUCT) {
        ScriptStructType* structType = static_cast<ScriptStructType*>(objType);

        std::string symName = structType->getTypeName();
        symName.append(".");
        symName.append(m_strings->getview(prop->property->value));
        stringid symId = m_strings->findId(symName);

        Symbol* propSym = m_scopes[0].findSymbol(symId, SYM_PROP);
        if (propSym) {
          propSym->writeUses++;
        }
      }

      return;
    }

    case AST_IndexAccessExpr: {
      const IndexAccessExpr* idx = static_cast<IndexAccessExpr*>(expr);
      ScriptType* type = idx->target->resultType;

      if (type->kind() == TK_STRING) {
        m_errors->error(expr->location, "Cannot mutate strings");
      }

      return;
    }

    default:
      m_errors->error(expr->location, "Invalid left-hand-side expression; cannot be assigned to");
      break;
  }
}

void SemanticAnalyzer::acceptBinaryExpr(BinaryExpr* v) {
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

  if (op & BOP_ASSIGN_FLAG) {
    checkAssignability(v->lhs);
  }

  v->resultType = res;
}

static ScriptType* getUnaryOpResult(unaryop op, ScriptType* type) {
  if (type->kind() != TK_PRIMITIVE) {
    return nullptr;
  }

  PrimitiveScriptType* p = (PrimitiveScriptType*) type;

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

void SemanticAnalyzer::acceptUnaryExpr(UnaryExpr* v) {
  v->target->acceptVisit(this);

  ScriptType* resType = getUnaryOpResult(v->op, v->target->resultType);

  if (resType) {
    checkAssignability(v->target);
    v->resultType = resType;
    return;
  }

  v->resultType = v->target->resultType;

  m_errors->error(v->location, "Cannot use %s operator on %s",
    unaryop_name(v->op),
    v->resultType->getTypeName()
  );
}

void SemanticAnalyzer::acceptTernaryExpr(TernaryExpr* v) {
  v->condition->acceptVisit(this);

  if (!isAssignableTo(ConstTypes::BOOL(), v->condition->resultType)) {
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

void SemanticAnalyzer::acceptBlock(Block* v) {
  NOT_MAIN("Code Block")

  STATPUSH
  pushScope(SCOPE_BLOCK);

  for (Statement* statement : v->statements) {
    statement->acceptVisit(this);
  }

  popScope();
  NOT_MAIN_TRAILING
  STATPOP
}

static bool isAllowedLoopOrIfBody(Statement* stat) {
  switch (stat->nodeKind()) {
    case AST_Block:
    case AST_ForStatement:
    case AST_DoWhileStatement:
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

void SemanticAnalyzer::acceptIfStatement(IfStatement* v) {
  NOT_MAIN("If Statement")
  STATPUSH

  v->condition->acceptVisit(this);

  if (isAllowedLoopOrIfBody(v->body)) {
    v->body->acceptVisit(this);
  } else {
    m_errors->error(v->location, "Statement not allowed as if statement's body");
  }

  if (v->elseBody) {
    if (isAllowedLoopOrIfBody(v->elseBody)) {
      v->elseBody->acceptVisit(this);
    } else {
      m_errors->error(v->location, "Statement not allowed as if statement's else statement");
    }
  }

  NOT_MAIN_TRAILING
  STATPOP
}

void SemanticAnalyzer::acceptForStatement(ForStatement* v) {
  NOT_MAIN("For Loop")

  STATPUSH
  pushScope(SCOPE_LOOP, v->label ? v->label->value : EMPTY_STRING);

  v->first->acceptVisit(this);
  v->second->acceptVisit(this);
  v->third->acceptVisit(this);

  acceptBodyNoScope(v->loopBody);

  popScope();

  NOT_MAIN_TRAILING
  STATPOP
}

void SemanticAnalyzer::acceptLexicalDeclaration(LexicalDeclaration* v) {
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
  } else if (v->isConstDeclaration) {
    std::string_view view = m_strings->getview(v->variableName->value);
    m_errors->error(v->location, "Const variable '%.*s' has no value", PRINTVIEW(view));
  }

  Scope* scope = getScope();
  Symbol* existingVar = scope->findVariable(v->variableName->value);
  if (existingVar) {
    m_errors->error(v->location, "Duplicate variable definition");
    return;
  }

  symboltype stype = v->isConstDeclaration ? SYM_CONST : SYM_VAR;
  scope->pushSymbol(v->variableName->value, v->typeExpr->referencedType, stype);

  STATPOP;
}

void SemanticAnalyzer::acceptDoWhileStatement(DoWhileStatement* v) {
  NOT_MAIN("Do While Loop")

  STATPUSH
  pushScope(SCOPE_LOOP, v->label ? v->label->value : EMPTY_STRING);

  acceptBodyNoScope(v->body);
  v->condition->acceptVisit(this);

  popScope();

  NOT_MAIN_TRAILING
  STATPOP
}

void SemanticAnalyzer::acceptWhileStatement(WhileStatement* v) {
  NOT_MAIN("While Loop")

  STATPUSH
  pushScope(SCOPE_LOOP, v->label ? v->label->value : EMPTY_STRING);

  v->condition->acceptVisit(this);
  acceptBodyNoScope(v->body);

  popScope();

  NOT_MAIN_TRAILING
  STATPOP
}

void SemanticAnalyzer::acceptControlFlowStatement(ControlFlowStatement* v) {
  NOT_MAINR(v->type == CFT_BREAK ? "Break" : "Continue")

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

  uint32 scopeOff = 0;
  Scope* scope = nullptr;
  bool loopFound = false;

  while (true) {
    scope = getScope(scopeOff++);

    if (!scope) {
      break;
    }

    scopetype stype = scope->getType();
    if (stype == SCOPE_FUNCTION || stype == SCOPE_MAIN) {
      break;
    }

    if (stype != SCOPE_LOOP) {
      continue;
    }

    loopFound = true;

    if (!v->label || v->label->value == scope->getLabel()) {
      return;
    }
  }

  if (loopFound) {
    const std::string_view view = m_strings->getview(v->label->value);
    m_errors->error(v->location, "No loop with label '%.*s'",
      static_cast<int32>(view.length()), view.data()
    );
    return;
  }

  m_errors->error(v->location, "Control flow statement used outside of loop");
}

void SemanticAnalyzer::acceptReturnStatement(ReturnStatement* v) {
  NOT_MAINR("Return statement")

  ScriptType* expected = getScope()->getExpectedReturnType();

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

  if (expected->kind() == TK_VOID) {
    m_errors->error(v->location, "Cannot return a value in a void method");
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

void SemanticAnalyzer::acceptScriptFileStatement(ScriptFileStatement* v) {
  STATPUSH
  pushScope(SCOPE_MAIN);

  Scope* scope = getScope();
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
    createStructType(decl);
  }
  for (StructDecl* decl : structDeclarations) {
    resolveMissingProperties(decl);
  }

  for (FunctionDeclStatement* decl : functionDecls) {
    createFuncSignature(decl);
  }

  for (Statement* s : v->statements) {
    s->acceptVisit(this);
  }

  popScope();
  STATPOP
}

void SemanticAnalyzer::acceptFunctionParam(FunctionParam* v) {
  // Should not be called
}

void SemanticAnalyzer::acceptFunctionDeclStatement(FunctionDeclStatement* v) {
  STATPUSH

  Scope* parentScope = getScope();
  if (parentScope->getType() != SCOPE_MAIN) {
    createFuncSignature(v);
  }

  Scope* scope = pushScope(SCOPE_FUNCTION);
  scope->setExpectedReturnType(v->returnType->referencedType);

  FunctionSignature* sign = v->signature;
  const uint32 args = v->arguments.size();

  for (uint32 i = 0; i < args; i++) {
    ScriptType* signType = sign->getArgumentType(i);
    FunctionParam* arg = v->arguments[i];
    scope->pushSymbol(arg->name->value, signType, SYM_VAR);
  }

  acceptBodyNoScope(v->functionBody);

  if (sign->getReturnType()->kind() != TK_VOID && !everyBranchHasReturn(v->functionBody)) {
    m_errors->error(v->location, "Non-void function has no return value");
  }

  popScope();

  STATPOP
}

void SemanticAnalyzer::acceptExprStatement(ExprStatement* v) {
  NOT_MAIN("Expression")
  STATPUSH
  v->expression->acceptVisit(this);

  NOT_MAIN_TRAILING
  STATPOP
}

void SemanticAnalyzer::acceptStructPropertyDecl(StructPropertyDecl* v) {
  // Should not be called
}

void SemanticAnalyzer::acceptStructDecl(StructDecl* v) {
  STATPUSH

  Scope* scope = getScope();
  if (scope->getType() != SCOPE_MAIN) {
    m_errors->error(v->location, "Structs can only be declared in the global scope");
    return;
  }

  const uint32 propCount = v->properties.size();
  conststring typeName = v->type->getTypeName();

  for (uint32 i = 0; i < propCount; i++) {
    StructPropertyDecl* prop = v->properties.at(i);
    StructProperty* typeProp = v->type->getProperty(i);

    std::string& propertyName = typeProp->name;
    ScriptType* propertyType = typeProp->type;

    if (prop->value) {
      prop->value->acceptVisit(this);
      ScriptType* valueType = prop->value->resultType;

      if (!isAssignableTo(propertyType, valueType)) {
        m_errors->error(prop->location,
          "Cannot assign %s value to property %s %s.%s",
          valueType->getTypeName(),
          propertyType->getTypeName(),
          typeName,
          propertyName.c_str()
        );
      }
    }
  }

  STATPOP
}

void SemanticAnalyzer::acceptAssertStatement(AssertStatement* v) {
  v->condition->acceptVisit(this);
  ScriptType* condType = v->condition->resultType;

  if (!isAssignableTo(ConstTypes::BOOL(), condType)) {
    m_errors->error(
      v->condition->location,
      "assert statement condition cannot be assigned to boolean"
    );
  }

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
}
