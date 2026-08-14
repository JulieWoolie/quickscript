#include "transformer.h"

#include <cmath>

#include "../types/ConstTypes.h"

#define IS_NUM_LITERAL(v) (v == AST_IntLiteral || v == AST_FloatLiteral)

Expr* SemanticTransformer::optimizeStringConcat(StringLiteral* lhs, Expr* rhs) const {
  StringTable& strings = ctx.getStrings();

  const astnodetype rkind = rhs->nodeKind();
  const stringid lId = lhs->value;
  const uint32 leftLen = strings.getlen(lId);

  if (rkind == AST_StringLiteral || rkind == AST_CharLiteral) {
    stringid rId = EMPTY_STRING;

    if (rkind == AST_StringLiteral) {
      rId = static_cast<StringLiteral*>(rhs)->value;
    } else {
      rId = static_cast<CharLiteral*>(rhs)->value;
    }

    const uint32 rightLen = strings.getlen(rId);
    const uint32 totalLen = leftLen + rightLen;

    if (totalLen == 0) {
      // Both strings are empty????
      return lhs;
    }

    int8 concatenated[totalLen];

    strings.copychars(lId, concatenated, leftLen);
    strings.copychars(rId, concatenated + leftLen, rightLen);

    lhs->value = strings.allocate(concatenated, totalLen);
    return lhs;
  }

  std::string rString = strings.getstring(lId);

  switch (rkind) {
    case AST_IntLiteral: {
      IntLiteral* il = static_cast<IntLiteral*>(rhs);
      rString.append(std::to_string(il->value));
      break;
    }
    case AST_FloatLiteral: {
      FloatLiteral* fl = static_cast<FloatLiteral*>(rhs);
      rString.append(std::to_string(fl->value));
      break;
    }
    case AST_BooleanLiteral: {
      BooleanLiteral* bl = static_cast<BooleanLiteral*>(rhs);
      rString.append(bl->value ? "true" : "false");
      break;
    }
    default:
      return nullptr;
  }

  stringid newId = strings.allocate(rString);
  lhs->value = newId;

  return lhs;
}

Expr* SemanticTransformer::optimizeStringRepeat(StringLiteral* lhs, Expr* rhs) const {
  StringTable& table = ctx.getStrings();

  const stringid lId = lhs->value;
  const uint32 leftLen = table.getlen(lId);

  if (leftLen == 0) {
    return lhs;
  }

  const astnodetype rkind = rhs->nodeKind();
  if (rkind != AST_IntLiteral) {
    return nullptr;
  }

  int64 repeats = static_cast<IntLiteral*>(rhs)->value;
  if (repeats == 1) {
    return lhs;
  }
  if (repeats < 1) {
    lhs->value = EMPTY_STRING;
    return lhs;
  }

  const uint32 totalSize = leftLen * repeats;
  int8 buf[totalSize];

  const std::string_view view = table.getview(lId);

  for (uint64 i = 0; i < totalSize; i += leftLen) {
    memcpy(buf + i, view.data(), leftLen);
  }

  stringid newId = table.allocate(buf, totalSize);
  lhs->value = newId;

  return lhs;
}

Expr* SemanticTransformer::optimizeBinary(BinaryExpr* e) const {
  const binaryop op = e->op;

  Expr* lhs = e->lhs = optimizeExpr(e->lhs);
  Expr* rhs = e->rhs = optimizeExpr(e->rhs);
  
  const astnodetype lkind = e->lhs->nodeKind();
  const astnodetype rkind = e->rhs->nodeKind();
  
  NoFreeAllocator& alloc = ctx.getAllocator();

  if (lkind == AST_StringLiteral) {
    Expr* res;

    switch (op) {
      case BOP_ADD:
        res = optimizeStringConcat(static_cast<StringLiteral*>(lhs), rhs);
        break;
      case BOP_MUL:
        res = optimizeStringRepeat(static_cast<StringLiteral*>(lhs), rhs);
      default:
        return e;
    }

    if (!res) {
      return e;
    }

    return res;
  }

  if (lkind == AST_IntLiteral && rkind == AST_IntLiteral) {
    IntLiteral* lLit = (IntLiteral*) lhs;
    IntLiteral* rLit = (IntLiteral*) rhs;

    const int64 l = lLit->value;
    const int64 r = rLit->value;

    BooleanLiteral bl;
    bl.location = lLit->location;
    bl.resultType = ConstTypes::BOOL();

    switch (op) {
      case BOP_ADD:
        lLit->value += r;
        break;
      case BOP_SUB:
        lLit->value -= r;
        break;
      case BOP_MUL:
        lLit->value *= r;
        break;
      case BOP_DIV:
        lLit->value /= r;
        break;
      case BOP_POW:
        lLit->value = std::pow(l, r);
        break;
      case BOP_MOD:
        lLit->value = l % r;
        break;
      case BOP_SHL:
        lLit->value <<= r;
        break;
      case BOP_SHR:
        lLit->value >>= r;
        break;
      case BOP_USHR:
        lLit->value = ((uint64) l) >> ((uint64) r);
        break;
      case BOP_XOR:
        lLit->value ^= r;
        break;
      case BOP_LOG_OR:
        lLit->value = l || r;
        break;
      case BOP_LOG_AND:
        lLit->value = l && r;
        break;
      case BOP_BIT_OR:
        lLit->value = l | r;
        break;
      case BOP_BIT_AND:
        lLit->value = l & r;
        break;
      case BOP_GT:
        bl.value = l > r;
        return alloc.emplace(bl);
      case BOP_GTE:
        bl.value = l >= r;
        return alloc.emplace(bl);
      case BOP_LT:
        bl.value = l < r;
        return alloc.emplace(bl);
      case BOP_LTE:
        bl.value = l <= r;
        return alloc.emplace(bl);
      case BOP_EQ:
        bl.value = l == r;
        return alloc.emplace(bl);
      case BOP_NEQ:
        bl.value = l != r;
        return alloc.emplace(bl);
      default:
        return e;
    }

    return lLit;
  }

  if (lkind == AST_BooleanLiteral && rkind == AST_BooleanLiteral) {
    BooleanLiteral* bl = (BooleanLiteral*) lhs;
    BooleanLiteral* br = (BooleanLiteral*) rhs;

    const bool l = bl->value;
    const bool r = br->value;

    switch (op) {
      case BOP_EQ: bl->value = l == r; break;
      case BOP_NEQ: bl->value = l != r; break;

      case BOP_LOG_AND:
      case BOP_BIT_AND: bl->value = l && r; break;

      case BOP_LOG_OR:
      case BOP_BIT_OR: bl->value = l || r; break;

      case BOP_XOR: bl->value = l ^ r; break;

      case BOP_GT: bl->value = l > r; break;
      case BOP_GTE: bl->value = l >= r; break;
      case BOP_LT: bl->value = l < r; break;
      case BOP_LTE: bl->value = l <= r; break;

      default:
        return e;
    }

    return bl;
  }

  if (IS_NUM_LITERAL(lkind) && IS_NUM_LITERAL(rkind)) {
    FloatLiteral* fr;

    float64 l = 0.0;
    float64 r = 0.0;

    if (lkind == AST_FloatLiteral) {
      fr = (FloatLiteral*) lhs;
    } else {
      fr = (FloatLiteral*) rhs;
    }

    if (lkind == AST_FloatLiteral) {
      l = ((FloatLiteral*) lhs)->value;
    } else if (lkind == AST_IntLiteral) {
      l = ((IntLiteral*) lhs)->value;
    }

    if (rkind == AST_FloatLiteral) {
      r = ((FloatLiteral*) rhs)->value;
    } else if (rkind == AST_IntLiteral) {
      r = ((IntLiteral*) rhs)->value;
    }

    BooleanLiteral bl;
    bl.location = e->location;
    bl.resultType = ConstTypes::BOOL();

    switch (op) {
      case BOP_GT:
        bl.value = l > r;
        return alloc.emplace(bl);
      case BOP_GTE:
        bl.value = l >= r;
        return alloc.emplace(bl);
      case BOP_LT:
        bl.value = l < r;
        return alloc.emplace(bl);
      case BOP_LTE:
        bl.value = l <= r;
        return alloc.emplace(bl);
      case BOP_EQ:
        bl.value = l == r;
        return alloc.emplace(bl);
      case BOP_NEQ:
        bl.value = l != r;
        return alloc.emplace(bl);

      case BOP_ADD:
        fr->value = l + r;
        break;
      case BOP_SUB:
        fr->value = l - r;
        break;
      case BOP_MUL:
        fr->value = l * r;
        break;
      case BOP_DIV:
        fr->value = l / r;
        break;
      case BOP_MOD:
        fr->value = std::fmod(l, r);
        break;
      case BOP_POW:
        fr->value = std::pow(l, r);
        break;
      default:
        return e;
    }

    fr->location = e->location;
    return fr;
  }

  return e;
}

Expr* SemanticTransformer::optimizeUnary(UnaryExpr* u) const {
  Expr* target = u->target = optimizeExpr(u->target);

  const unaryop op = u->op;
  const astnodetype kind = target->nodeKind();

  // Optimize away the unary expression if we're evaluating  a literal value
  switch (op) {
    case UOP_NEG: {
      if (kind == AST_FloatLiteral) {
        FloatLiteral* fl = static_cast<FloatLiteral*>(target);
        fl->value = -fl->value;
        return fl;
      }
      if (kind == AST_IntLiteral) {
        IntLiteral* il = static_cast<IntLiteral*>(target);
        il->value = -il->value;
        return il;
      }
      return u;
    }

    case UOP_BIT_NOT: {
      if (kind == AST_BooleanLiteral) {
        BooleanLiteral* bl = static_cast<BooleanLiteral*>(target);
        bl->value = !bl->value;
        return bl;
      }
      if (kind == AST_IntLiteral) {
        IntLiteral* il = static_cast<IntLiteral*>(target);
        il->value = ~il->value;
        return il;
      }
      return u;
    }

    case UOP_LOG_NOT: {
      if (kind == AST_BooleanLiteral) {
        BooleanLiteral* bl = static_cast<BooleanLiteral*>(target);
        bl->value = !bl->value;
        return bl;
      }
      if (kind == AST_IntLiteral) {
        IntLiteral* il = static_cast<IntLiteral*>(target);
        il->value = !il->value;
        return il;
      }
      return u;
    }

    case UOP_POS:
      return target;

    default:
      return u;
  }

  return u;
}

Expr* SemanticTransformer::optimizeTernary(TernaryExpr* t) const {
  Expr* cond = t->condition = optimizeExpr(t->condition);
  Expr* left = t->left = optimizeExpr(t->left);
  Expr* right = t->left = optimizeExpr(t->right);

  switch (cond->nodeKind()) {
    case AST_BooleanLiteral: {
      const BooleanLiteral* bl = static_cast<BooleanLiteral*>(cond);
      return bl->value ? left : right;
    }
    case AST_IntLiteral: {
      const IntLiteral* il = static_cast<IntLiteral*>(cond);
      return il->value != 0 ? left : right;
    }
    case AST_FloatLiteral: {
      const FloatLiteral* il = static_cast<FloatLiteral*>(cond);
      return il->value != 0 ? left : right;
    }
    default:
      return t;
  }
}

Expr* SemanticTransformer::optimizeExpr(Expr* expr) const {
  switch (expr->nodeKind()) {
    case AST_BinaryExpr:
      return optimizeBinary(static_cast<BinaryExpr*>(expr));
    case AST_UnaryExpr:
      return optimizeUnary(static_cast<UnaryExpr*>(expr));
    case AST_TernaryExpr:
      return optimizeTernary(static_cast<TernaryExpr*>(expr));
    default:
      return expr;
  }
}

static bool isLiteral(Expr* node) {
  switch (node->nodeKind()) {
    case AST_FloatLiteral:
    case AST_StringLiteral:
    case AST_IntLiteral:
    case AST_CharLiteral:
    case AST_BooleanLiteral:
      return true;
    default:
      return false;
  }
}

static bool isBooleanAssignableLiteral(Expr* expr) {
  switch (expr->nodeKind()) {
    case AST_FloatLiteral:
    case AST_IntLiteral:
    case AST_BooleanLiteral:
      return true;
    default:
      return false;
  }
}

static bool literalToBoolean(Expr* expr) {
  switch (expr->nodeKind()) {
    case AST_FloatLiteral:
      return static_cast<FloatLiteral*>(expr)->value != 0;
    case AST_IntLiteral:
      return static_cast<IntLiteral*>(expr)->value != 0;
    case AST_BooleanLiteral:
      return static_cast<BooleanLiteral*>(expr)->value;
    default:
      return false;
  }
}

static bool isPointlessStandalone(Expr* expr) {
  switch (expr->nodeKind()) {
    case AST_FloatLiteral:
    case AST_StringLiteral:
    case AST_IntLiteral:
    case AST_CharLiteral:
    case AST_BooleanLiteral:
    case AST_Identifier:
      return true;

    case AST_UnaryExpr: {
      const UnaryExpr* u = static_cast<UnaryExpr*>(expr);
      if (!isPointlessStandalone(u->target)) {
        return false;
      }

      switch (u->op) {
        case UOP_PREINC:
        case UOP_POSTINC:
        case UOP_PREDEC:
        case UOP_POSTDEC:
          return false;
        default:
          return true;
      }
    }

    case AST_BinaryExpr: {
      const BinaryExpr* bin = static_cast<BinaryExpr*>(expr);
      if (!isPointlessStandalone(bin->lhs)) {
        return false;
      }
      if (!isPointlessStandalone(bin->rhs)) {
        return false;
      }
      const binaryop op = bin->op;
      return !(op & BOP_ASSIGN_FLAG);
    }

    case AST_TernaryExpr: {
      const TernaryExpr* tern = static_cast<TernaryExpr*>(expr);

      return isPointlessStandalone(tern->condition)
          && isPointlessStandalone(tern->left)
          && isPointlessStandalone(tern->right);
    }

    case AST_ObjectLiteral: {
      const ObjectLiteral* literal = static_cast<ObjectLiteral*>(expr);
      for (const ObjectLiteralProperty* prop : literal->properties) {
        if (isPointlessStandalone(prop->value)) {
          continue;
        }
        return false;
      }
      return true;
    }

    case AST_ArrayLiteral: {
      const ArrayLiteral* literal = static_cast<ArrayLiteral*>(expr);
      for (Expr* v : literal->values) {
        if (isPointlessStandalone(v)) {
          continue;
        }
        return false;
      }
      return true;
    }

    default:
      return false;
  }
}

Statement* SemanticTransformer::optimizeStatement(Statement* stat, const bool emptyBlocksAsNull) {
  switch (stat->nodeKind()) {
    case AST_Block: {
      Block* b = static_cast<Block*>(stat);
      std::vector<Statement*>& stats = b->statements;

      auto it = stats.begin();
      while (it != stats.end()) {
        Statement* blockStat = optimizeStatement(*it, true);

        if (!blockStat) {
          it = stats.erase(it);
          continue;
        }

        *it = blockStat;
        ++it;
      }

      if (emptyBlocksAsNull && stats.empty()) {
        return nullptr;
      }
      return b;
    }
    case AST_IfStatement: {
      IfStatement* ifStatement = static_cast<IfStatement*>(stat);
      Expr* cond = ifStatement->condition = optimizeExpr(ifStatement->condition);

      if (!isBooleanAssignableLiteral(cond)) {
        return ifStatement;
      }

      bool condResult = literalToBoolean(cond);
      if (condResult) {
        return optimizeStatement(ifStatement->body, true);
      }

      if (!ifStatement->elseBody) {
        return nullptr;
      }

      return optimizeStatement(ifStatement->elseBody, true);
    }
    case AST_ForStatement: {
      ForStatement* forStat = static_cast<ForStatement*>(stat);

      LexicalDeclaration* first = forStat->first;
      first->value = optimizeExpr(first->value);

      forStat->second = optimizeExpr(forStat->second);
      forStat->third = optimizeExpr(forStat->third);

      forStat->loopBody = optimizeStatement(forStat->loopBody, true);
      if (!forStat->loopBody) {
        return nullptr;
      }

      return forStat;
    }
    case AST_LexicalDeclaration: {
      LexicalDeclaration* lex = static_cast<LexicalDeclaration*>(stat);
      if (!lex->value) {
        return stat;
      }
      lex->value = optimizeExpr(lex->value);
      return stat;
    }
    case AST_WhileStatement: {
      WhileStatement* whileLoop = static_cast<WhileStatement*>(stat);
      whileLoop->condition = optimizeExpr(whileLoop->condition);
      whileLoop->body = static_cast<Block*>(optimizeStatement(whileLoop->body, true));
      if (!whileLoop->body) {
        return nullptr;
      }
      return whileLoop;
    }
    case AST_FunctionDeclStatement: {
      FunctionDeclStatement* decl = static_cast<FunctionDeclStatement*>(stat);
      optimizeStatement(decl->functionBody, false);
      return decl;
    }
    case AST_ExprStatement: {
      ExprStatement* exprStat = static_cast<ExprStatement*>(stat);
      Expr* expr = exprStat->expression = optimizeExpr(exprStat->expression);

      if (isPointlessStandalone(expr)) {
        return nullptr;
      }

      if (expr->nodeKind() == AST_UnaryExpr) {
        UnaryExpr* un = static_cast<UnaryExpr*>(expr);
        const unaryop op = un->op;
        if (op == UOP_POSTDEC) {
          un->op = UOP_PREDEC;
        } else if (op == UOP_POSTINC) {
          un->op = UOP_PREINC;
        }
      }

      return exprStat;
    }

    case AST_ReturnStatement: {
      ReturnStatement* ret = static_cast<ReturnStatement*>(stat);
      if (ret->value) {
        ret->value = optimizeExpr(ret->value);
      }
      return ret;
    }

    default:
      return stat;
  }
  return stat;
}

Identifier* SemanticTransformer::makeId(std::string& string) {
  stringid nameId = ctx.getStrings().allocate(string);
  Identifier* id = ctx.getAllocator().make<Identifier>();
  id->value = nameId;
  return id;
}

Identifier* SemanticTransformer::makeId(stringid id) {
  Identifier* idExpr = ctx.getAllocator().make<Identifier>();
  idExpr->value = id;
  return idExpr;
}

void SemanticTransformer::runOptimizer() {
  for (Statement* statement : sfs->statements) {
    optimizeStatement(statement, false);
  }
}

void SemanticTransformer::createStructConstructors() {
  NoFreeAllocator& alloc = ctx.getAllocator();
  StringTable& strings = ctx.getStrings();
  std::vector<StructDecl*>& decls = ctx.getDeclaredStructs();
  TypeTable& types = ctx.getTypes();
  std::unordered_map<Node*, Symbol*>& lookup = ctx.getSymbolLookup();

  constexpr std::string thisStr = "this";
  stringid thisId = strings.allocate(thisStr);

  Scope* global = ctx.getGlobalScope();

  for (StructDecl* decl : decls) {
    LocalStructSymbol* lss = static_cast<LocalStructSymbol*>(lookup[decl]);
    Scope* scope = ctx.getScopeLookup()[lss];

    ScriptStructType* structType = decl->type;

    std::string ctorName = std::string(decl->name->value->view());
    ctorName.append(".<constructor>");

    Block* funcBlock = alloc.make<Block>();

    FunctionDeclStatement* ctorDecl = alloc.make<FunctionDeclStatement>();
    ctorDecl->name = makeId(ctorName);
    ctorDecl->functionBody = funcBlock;
    ctorDecl->signature = types.getSignature(structType, false, 0, nullptr);
    funcBlock->parentStatement = ctorDecl;
    ctorDecl->parentStatement = sfs;

    LocalFunction* lf = alloc.make<LocalFunction>(ctorDecl->name->value, ctorDecl, scope);
    LocalFuncSymbol* lfs = alloc.make<LocalFuncSymbol>(*lf);

    ctx.pushLocalFunction(lf);
    global->pushSymbol(lfs);

    LexicalDeclaration* lexDecl = alloc.make<LexicalDeclaration>();
    lexDecl->isConstDeclaration = true;
    lexDecl->variableName = makeId(thisId);

    ObjectAllocExpr* allocExpr = alloc.make<ObjectAllocExpr>();
    allocExpr->resultType = decl->type;
    lexDecl->value = allocExpr;

    std::vector<Statement*>& stats = funcBlock->statements;
    stats.push_back(lexDecl);
    lexDecl->parentStatement = funcBlock;

    Scope* bodyScope = alloc.make<Scope>(SCOPE_FUNCTION, global);
    LocalVarSymbol* thisSym = alloc.make<LocalVarSymbol>(thisId, structType, POINTER_SIZE, 0, lexDecl);

    bodyScope->pushSymbol(thisSym);
    ctx.getSymbolLookup()[lexDecl] = thisSym;
    ctx.getScopeLookup()[thisSym] = bodyScope;

    for (StructPropertyDecl* prop : decl->properties) {
      if (!prop->value) {
        continue;
      }

      Expr* propValue = prop->value;

      Identifier* thisExpr = makeId(thisId);
      thisExpr->resultType = structType;

      Identifier* propId = makeId(prop->name->value);
      PropertySymbol* propSym = static_cast<PropertySymbol*>(ctx.getSymbolLookup()[prop]);
      propId->resultType = propSym->getScriptType();

      PropertyAccessExpr* access = alloc.make<PropertyAccessExpr>();
      access->target = thisExpr;
      access->property = propId;
      access->resultType = propSym->getScriptType();

      ctx.getSymbolLookup()[thisExpr] = thisSym;
      ctx.getSymbolLookup()[propId] = propSym;

      BinaryExpr* assignExpr = alloc.make<BinaryExpr>();
      assignExpr->op = BOP_ASSIGN;
      assignExpr->lhs = access;
      assignExpr->rhs = propValue;
      assignExpr->resultType = propSym->getScriptType();

      ExprStatement* stat = alloc.make<ExprStatement>();
      stat->expression = assignExpr;

      stats.push_back(stat);
      stat->parentStatement = funcBlock;

      prop->value = nullptr;
    }

    sfs->statements.push_back(ctorDecl);
  }
}

FunctionSignature* createMainSignature(TypeTable& table) {
  ScriptType* argsType = table.getArrayType(ConstTypes::STRING());
  return table.getSignature(ConstTypes::INT32(), false, 1, &argsType);
}

void SemanticTransformer::createFileInitMethod() {
  NoFreeAllocator& alloc = ctx.getAllocator();
  StringTable& strings = ctx.getStrings();
  TypeTable& types = ctx.getTypes();
  std::unordered_map<Node*, Symbol*>& lookup = ctx.getSymbolLookup();

  Scope* global = ctx.getGlobalScope();
  stringid finitName = strings.allocate("<finit>");

  FunctionDeclStatement* fdecl = alloc.make<FunctionDeclStatement>();
  fdecl->name = makeId(finitName);
  fdecl->parentStatement = sfs;

  std::string argsName = "args";
  FunctionParam* param = alloc.make<FunctionParam>();
  param->name = makeId(argsName);
  param->varargs = false;
  param->parentStatement = fdecl;

  Block* body = alloc.make<Block>();
  body->parentStatement = fdecl;

  fdecl->functionBody = body;
  fdecl->arguments.push_back(param);
  fdecl->signature = createMainSignature(types);

  LocalVarSymbol* argsSym = alloc.make<LocalVarSymbol>(param->name->value, fdecl->signature->getArgumentType(0), POINTER_SIZE, 0, param);
  argsSym->addFlags(SYMFLAG_FUNC_ARG);
  lookup[param] = argsSym;

  Scope* funcScope = alloc.make<Scope>(SCOPE_FUNCTION, global);
  funcScope->pushSymbol(argsSym);

  LocalFunction* lf = alloc.make<LocalFunction>(finitName, fdecl, global);
  LocalFuncSymbol* lfs = alloc.make<LocalFuncSymbol>(*lf);

  ctx.pushLocalFunction(lf);
  global->pushSymbol(lfs);
  lookup[fdecl] = lfs;
  ctx.getScopeLookup()[lfs] = global;

  sfs->statements.push_back(fdecl);

  std::vector<LexicalDeclaration*>& globalVars = ctx.getGlobalVariables();
  for (LexicalDeclaration* gvar : globalVars) {
    if (!gvar->value) {
      continue;
    }

    Identifier* varId = makeId(gvar->variableName->value);

    BinaryExpr* assignExpr = alloc.make<BinaryExpr>();
    assignExpr->lhs = varId;
    assignExpr->rhs = gvar->value;
    assignExpr->op = BOP_ASSIGN;

    ExprStatement* exprStat = alloc.make<ExprStatement>();
    exprStat->expression = assignExpr;
    exprStat->parentStatement = body;

    body->statements.push_back(exprStat);
    lookup[varId] = lookup[gvar];
    gvar->value = nullptr;
  }

  std::vector<LocalFunction*>& mains = ctx.getMainFuncCandidates();
  LocalFunction* main;
  if (mains.empty()) {
    main = nullptr;
  } else {
    main = mains.at(0);
  }

  if (main) {
    FunctionSignature* sign = main->getSignature();

    Identifier* callId = makeId(main->getName());
    CallExpr* call = alloc.make<CallExpr>();
    call->target = callId;

    if (sign->getArgumentsLength() > 0 && sign->getArgumentType(0) == fdecl->signature->getArgumentType(0)) {
      Identifier* argsId = makeId(argsName);
      lookup[argsId] = argsSym;
      call->arguments.push_back(argsId);
    }

    if (sign->getReturnType() == ConstTypes::INT32()) {
      ReturnStatement* ret = alloc.make<ReturnStatement>();
      ret->value = call;
      ret->parentStatement = body;
      body->statements.push_back(ret);
      return;
    }

    ExprStatement* exprStat = alloc.make<ExprStatement>();
    exprStat->expression = call;
    exprStat->parentStatement = body;
    body->statements.push_back(exprStat);
  }

  ReturnStatement* ret = alloc.make<ReturnStatement>();
  IntLiteral* il = alloc.make<IntLiteral>();
  il->value = 0;
  il->smallestFittingType = PPT_INT32;
  il->resultType = ConstTypes::INT32();

  ret->value = il;
  ret->parentStatement = body;

  body->statements.push_back(ret);
}

static stringid getNestedFunctionName(const LocalFunction* lf, SemanticContext& ctx) {
  const stringid funcName = lf->getName();

  std::string name = "";
  name.append(funcName->data, funcName->len);

  const Scope* scope = lf->getScope();
  while (scope) {
    scope = scope->getParent();

    if (scope->getType() != SCOPE_FUNCTION) {
      continue;
    }

    const FunctionDeclStatement* decl = nullptr;
    for (const LocalFunction* func: ctx.getLocalFunctions()) {
      if (func->getScope() != scope) {
        continue;
      }
      decl = func->getDecl();
      break;
    }

    if (!decl) {
      // ????
      continue;
    }

    stringid parentName = decl->name->value;
    name.insert(0, 1, '#');
    name.insert(0, parentName->data, parentName->len);
  }

  name.insert(0, 1, '%');

  return ctx.getStrings().allocate(name);
}

void SemanticTransformer::flattenNestedFunctions() {
  std::vector<LocalFunction*>& functions = ctx.getLocalFunctions();

  for (auto it = functions.rbegin(); it != functions.rend(); ++it) {
    LocalFunction* lf = *it;
    if (!lf->isNested()) {
      continue;
    }

    // TODO:
    //   1. Add a parameter to the function "#closure: Stack*" that points to the
    //      stack frame of the nested function
    //   2. Replace every access of a variable from the nesting function with
    //      an access to the closure.
    //   3. Replace the function declaration with a LexDecl statement creating
    //      the closure with the current stack frame's pointer as its value.
    //   3. In calls to the function, provide the closure.
    //

    stringid newName = getNestedFunctionName(lf, ctx);

    FunctionDeclStatement* fdecl = lf->getDecl();
    fdecl->name->value = newName;
    lf->setNested(false);

    Block* parent = static_cast<Block*>(fdecl->parentStatement);
    std::vector<Statement*>& stats = parent->statements;
    for (auto statIt = stats.begin(); statIt != stats.end(); ++statIt) {
      if (*statIt == fdecl) {
        stats.erase(statIt);
        break;
      }
    }

    sfs->statements.push_back(fdecl);
    fdecl->parentStatement = sfs;
  }
}

SemanticTransformer::SemanticTransformer(SemanticContext& _ctx, ScriptFileStatement* _sfs)
  : ctx(_ctx), sfs(_sfs)
{

}

void SemanticTransformer::run() {
  runOptimizer();
  createStructConstructors();
  createFileInitMethod();
  flattenNestedFunctions();
}

void runSemanticTransformer(SemanticContext& ctx, ScriptFileStatement* sfs) {
  SemanticTransformer transformer = SemanticTransformer(ctx, sfs);
  return transformer.run();
}
