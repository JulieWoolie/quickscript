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
  Expr* right = t->right = optimizeExpr(t->right);

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
  if (!ctx.getOptions().exprOptimizing) {
    return expr;
  }

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
  const CompilationOptions& opts = ctx.getOptions();

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

      if (stats.size() == 1) {
        return stats[0];
      }

      if (emptyBlocksAsNull && stats.empty() && opts.statOptimizing) {
        return nullptr;
      }
      return b;
    }
    case AST_IfStatement: {
      IfStatement* ifStatement = static_cast<IfStatement*>(stat);
      Expr* cond = ifStatement->condition = optimizeExpr(ifStatement->condition);

      ifStatement->body = optimizeStatement(ifStatement->body, true);
      if (ifStatement->elseBody) {
        ifStatement->elseBody = optimizeStatement(ifStatement->elseBody, true);
      }

      if (!isBooleanAssignableLiteral(cond) || !opts.statOptimizing) {
        return ifStatement;
      }

      bool condResult = literalToBoolean(cond);
      if (condResult) {
        return ifStatement->body;
      }

      return ifStatement->elseBody;
    }
    case AST_ForStatement: {
      ForStatement* forStat = static_cast<ForStatement*>(stat);

      LexicalDeclaration* first = forStat->first;
      first->value = optimizeExpr(first->value);

      forStat->second = optimizeExpr(forStat->second);
      forStat->third = optimizeExpr(forStat->third);

      if (forStat->third->nodeKind() == AST_UnaryExpr) {
        UnaryExpr* un = static_cast<UnaryExpr*>(forStat->third);
        if (un->op == UOP_POSTDEC) {
          un->op = UOP_PREDEC;
        } else if (un->op == UOP_POSTINC) {
          un->op = UOP_PREINC;
        }
      }

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

      if (!opts.statOptimizing) {
        return exprStat;
      }

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

    case AST_AssertStatement: {
      AssertStatement* assert = static_cast<AssertStatement*>(stat);
      if (!opts.includeAsserts) {
        return nullptr;
      }

      assert->condition = optimizeExpr(assert->condition);

      if (assert->message) {
        assert->message = optimizeExpr(assert->message);
      }

      if (!isBooleanAssignableLiteral(assert->condition) || !opts.statOptimizing) {
        return stat;
      }

      const bool cond = literalToBoolean(assert->condition);
      if (!cond) {
        return nullptr;
      }

      return stat;
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

Identifier* SemanticTransformer::makeId(const std::string& string) const {
  stringid nameId = ctx.getStrings().allocate(string);
  Identifier* id = ctx.getAllocator().make<Identifier>();
  id->value = nameId;
  return id;
}

Identifier* SemanticTransformer::makeId(const conststring string) const {
  stringid nameId = ctx.getStrings().allocate(string);
  Identifier* id = ctx.getAllocator().make<Identifier>();
  id->value = nameId;
  return id;
}

Identifier* SemanticTransformer::makeId(const stringid id) const {
  Identifier* idExpr = ctx.getAllocator().make<Identifier>();
  idExpr->value = id;
  return idExpr;
}

static bool isZeroValue(Expr* expr) {
  switch (expr->nodeKind()) {
    case AST_FloatLiteral:
      return static_cast<FloatLiteral*>(expr)->value == 0.0;
    case AST_IntLiteral:
      return static_cast<IntLiteral*>(expr)->value == 0;
    case AST_BooleanLiteral:
      return !static_cast<BooleanLiteral*>(expr)->value;
    case AST_StringLiteral:
      return static_cast<StringLiteral*>(expr)->value->len == 0;
    case AST_ArrayLiteral:
      return static_cast<ArrayLiteral*>(expr)->values.empty();
    case AST_ObjectLiteral:
      return static_cast<ObjectLiteral*>(expr)->properties.empty();
    default:
      return false;
  }
}

void SemanticTransformer::removeZeroValues() {
  std::vector<LexicalDeclaration*>& decls = ctx.getAllVariables();
  for (LexicalDeclaration* decl : decls) {
    if (!decl->value) {
      continue;
    }
    if (!isZeroValue(decl->value)) {
      continue;
    }

    decl->value = nullptr;
  }

  for (StructDecl* decl : ctx.getDeclaredStructs()) {
    for (StructPropertyDecl* prop : decl->properties) {
      if (!prop->value) {
        continue;
      }
      if (!isZeroValue(prop->value)) {
        continue;
      }
      prop->value = nullptr;
    }
  }
}

void SemanticTransformer::runOptimizer() {
  const CompilationOptions& opts = ctx.getOptions();

  // Don't run optimizer if we've explicitly been told not to change anything
  if (!opts.statOptimizing && !opts.exprOptimizing && opts.includeAsserts) {
    return;
  }

  for (Statement* statement : sfs->statements) {
    optimizeStatement(statement, false);
  }
}

void SemanticTransformer::createPropertyAssignStatements(StructDecl* decl, LocalVarSymbol* thisSym, Block* funcBlock) const {
  NoFreeAllocator& alloc = ctx.getAllocator();
  stringid thisId = thisSym->getName();

  for (StructPropertyDecl* prop : decl->properties) {
    if (!prop->value) {
      continue;
    }

    Expr* propValue = prop->value;

    Identifier* thisExpr = makeId(thisId);
    thisExpr->resultType = decl->type;

    Identifier* propId = makeId(prop->name->value);
    LocalStructPropSymbol* propSym = static_cast<LocalStructPropSymbol*>(ctx.getSymbolLookup()[prop]);
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

    funcBlock->statements.push_back(stat);
    stat->parentStatement = funcBlock;

    prop->value = nullptr;
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

    Scope* bodyScope = alloc.make<Scope>(SCOPE_FUNCTION, global);

    LocalFunction* lf = alloc.make<LocalFunction>(ctorDecl->name->value, ctorDecl);
    LocalFuncSymbol* lfs = alloc.make<LocalFuncSymbol>(lf);

    lf->setScope(bodyScope);

    ctx.pushLocalFunction(lf);
    global->pushSymbol(lfs);
    lookup[ctorDecl] = lfs;
    ctx.getAstScopeLookup()[ctorDecl] = bodyScope;
    ctx.getScopeLookup()[lfs] = bodyScope;

    ObjectAllocExpr* allocExpr = alloc.make<ObjectAllocExpr>();
    allocExpr->resultType = decl->type;

    std::vector<Statement*>& stats = funcBlock->statements;

    uint32 valuedProperties = 0;
    for (StructPropertyDecl* property : decl->properties) {
      if (!property->value) {
        continue;
      }
      valuedProperties++;
    }

    sfs->statements.push_back(ctorDecl);
    ctx.getConstructors()[structType] = lfs;

    if (valuedProperties == 0) {
      ReturnStatement* retStat = alloc.make<ReturnStatement>();
      retStat->value = allocExpr;
      retStat->parentStatement = funcBlock;
      stats.push_back(retStat);
      continue;
    }

    LexicalDeclaration* lexDecl = alloc.make<LexicalDeclaration>();
    lexDecl->flags = DECLFLAG_CONST;
    lexDecl->variableName = makeId(thisId);
    lexDecl->value = allocExpr;
    lexDecl->parentStatement = funcBlock;

    LocalVarSymbol* thisSym = alloc.make<LocalVarSymbol>(thisId, structType, POINTER_SIZE, 0, lexDecl);

    stats.push_back(lexDecl);
    bodyScope->pushSymbol(thisSym);
    ctx.getScopeLookup()[thisSym] = bodyScope;
    ctx.getSymbolLookup()[lexDecl] = thisSym;

    createPropertyAssignStatements(decl, thisSym, funcBlock);

    ReturnStatement* retStat = alloc.make<ReturnStatement>();
    retStat->value = makeId(thisId);
    retStat->value->resultType = structType;
    retStat->parentStatement = funcBlock;
    ctx.getSymbolLookup()[retStat->value] = thisSym;

    stats.push_back(retStat);
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
  ctx.getScopeLookup()[argsSym] = funcScope;

  LocalFunction* lf = alloc.make<LocalFunction>(finitName, fdecl);
  LocalFuncSymbol* lfs = alloc.make<LocalFuncSymbol>(lf);

  lf->setScope(funcScope);

  ctx.setEntryPoint(lfs);
  ctx.pushLocalFunction(lf);
  global->pushSymbol(lfs);
  lookup[fdecl] = lfs;
  ctx.getScopeLookup()[lfs] = global;
  ctx.getAstScopeLookup()[fdecl] = funcScope;

  sfs->statements.push_back(fdecl);

  const std::vector<LexicalDeclaration*>& globalVars = ctx.getGlobalVariables();
  for (LexicalDeclaration* gvar : globalVars) {
    if (!gvar->value) {
      continue;
    }

    Identifier* varId = makeId(gvar->variableName->value);

    BinaryExpr* assignExpr = alloc.make<BinaryExpr>();
    assignExpr->lhs = varId;
    assignExpr->rhs = gvar->value;
    assignExpr->op = BOP_ASSIGN;
    assignExpr->resultType = gvar->value->resultType;
    assignExpr->location = gvar->value->location;

    ExprStatement* exprStat = alloc.make<ExprStatement>();
    exprStat->expression = assignExpr;
    exprStat->parentStatement = body;
    exprStat->location = gvar->value->location;

    body->statements.push_back(exprStat);
    lookup[varId] = lookup[gvar];
    gvar->value = nullptr;
  }

  std::vector<LocalFunction*>& mains = ctx.getMainFuncCandidates();
  LocalFuncSymbol* mainSym;
  LocalFunction* main;

  if (mains.empty()) {
    mainSym = nullptr;
    main = nullptr;
  } else {
    main = mains[0];
    mainSym = static_cast<LocalFuncSymbol*>(ctx.getSymbolLookup()[main->getDecl()]);
  }

  if (mainSym) {
    FunctionSignature* sign = main->getSignature();

    Identifier* callId = makeId(main->getName());
    lookup[callId] = mainSym;
    callId->resultType = sign;

    CallExpr* call = alloc.make<CallExpr>();
    call->target = callId;
    call->resultType = main->getSignature()->getReturnType();

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

  for (const Scope* scope = lf->getScope(); scope; scope = scope->getParent()) {
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

static bool equals(stringid symName, std::string name) {
  if (symName->len != name.length()) {
    return false;
  }

  uint32 len = symName->len;
  for (uint32 i = 0; i < len; i++) {
    if (symName->data[i] == name[i]) {
      continue;
    }
    return false;
  }

  return true;
}

static stringid createClosureParamName(Scope* scope, StringTable& strings) {
  std::string prefix = "#closure";
  uint32 num = 1;

  while (num < 999) {
    std::string name = "";
    name.append(prefix);
    if (num < 10) {
      name.append("00");
    } else if (num < 100) {
      name.append("0");
    }
    name.append(std::to_string(num));
    bool failed = false;

    Scope* s = scope;
    while (s) {
      for (Symbol* symbol : s->getSymbols()) {
        stringid nameId = symbol->getName();
        if (equals(nameId, name)) {
          continue;
        }
        failed = true;
        break;
      }
      if (failed) {
        break;
      }
      s = s->getParent();
    }

    if (failed) {
      num++;
      continue;
    }

    return strings.allocate(name);
  }

  return nullptr;
}

void SemanticTransformer::flattenNestedFunctions() {
  std::vector<LocalFunction*>& functions = ctx.getLocalFunctions();
  NoFreeAllocator& alloc = ctx.getAllocator();

  for (auto it = functions.begin(); it != functions.end(); ++it) {
    LocalFunction* lf = *it;
    if (!lf->isNested()) {
      continue;
    }

    // TODO:
    //   x 1. Add a parameter to the function "#closure: Stack*" that points to the
    //        stack frame of the nested function
    //   _ 2. Replace every access of a variable from the nesting function with
    //        an access to the closure.
    //   x 3. Replace the function declaration with a LexDecl statement creating
    //        the closure with the current stack frame's pointer as its value.
    //   _ 3. In calls to the function, provide the closure.
    //

    stringid newName = getNestedFunctionName(lf, ctx);

    FunctionDeclStatement* fdecl = lf->getDecl();
    fdecl->name->value = newName;
    lf->setNested(false);

    Block* parent = static_cast<Block*>(fdecl->parentStatement);
    std::vector<Statement*>& stats = parent->statements;

    stringid closureId = createClosureParamName(lf->getScope(), ctx.getStrings());

    for (auto statIt = stats.begin(); statIt != stats.end(); ++statIt) {
      if (*statIt != fdecl) {
        continue;
      }

      LexicalDeclaration* decl = alloc.make<LexicalDeclaration>();
      decl->flags = DECLFLAG_CONST;
      decl->parentStatement = parent;
      decl->value = alloc.make<GetStackPointer>();
      decl->variableName = makeId(closureId);
      decl->value->resultType = ConstTypes::CLOSURE();

      LocalVarSymbol* lvs = alloc.make<LocalVarSymbol>(closureId, ConstTypes::CLOSURE(), POINTER_SIZE, 0, decl);

      Scope* scope = lf->getScope()->getParent();
      scope->pushSymbol(scope->findSymbol(lf->getName(), SYM_LocalFunc), lvs);
      ctx.getSymbolLookup()[decl] = lvs;
      ctx.getScopeLookup()[lvs] = scope;

      *statIt = decl;

      break;
    }

    FunctionParam* closureParam = alloc.make<FunctionParam>();
    closureParam->parentStatement = fdecl;
    closureParam->name = makeId(closureId);
    closureParam->varargs = false;

    LocalVarSymbol* closureSym = alloc.make<LocalVarSymbol>(closureId, ConstTypes::CLOSURE(), POINTER_SIZE, 0, closureParam);
    closureSym->addFlags(SYMFLAG_FUNC_ARG);

    fdecl->arguments.insert(fdecl->arguments.cbegin(), closureParam);
    Scope* fscope = lf->getScope();
    fscope->getSymbols().insert(fscope->getSymbols().cbegin(), closureSym);
    ctx.getScopeLookup()[closureSym] = fscope;

    sfs->statements.push_back(fdecl);
    fdecl->parentStatement = sfs;
  }
}

static void processGlobalVarScopes(SemanticContext& ctx) {
  Scope* gscope = ctx.getGlobalScope();
  const std::vector<LexicalDeclaration*>& globals = ctx.getGlobalVariables();
  uint64 off = 0;

  for (LexicalDeclaration* decl : globals) {
    LocalVarSymbol* lvs = static_cast<LocalVarSymbol*>(ctx.getSymbolLookup()[decl]);
    lvs->setStackOffset(off);
    off += lvs->getStackSize();
  }

  gscope->setStackSize(off);
}

struct ScopeProcessContext {
  SemanticContext& semantics;
  Scope* scope;
  uint64 callArgsSpace = 0;
  uint64 extraSpace = 0;
  uint64 variableSpace = 0;
  uint64 offsetStart = 0;
};

#define LOOKUP_LOCALVAR(c, v) static_cast<LocalVarSymbol*>(c[v])
#define CASTED_VAR(name, type, value) type* name = static_cast<type*>(value);
#define AST_SWITCH(node) switch (node->nodeKind())
#define AST_CASE(type, vname, v, body) case AST_##type: {type* vname = static_cast<type*>(v); body}

static void processScope(Node* node, ScopeProcessContext& ctx, const bool rollbackBlock = true) {
  const uint64 start = ctx.offsetStart;
  const uint64 startSize = ctx.variableSpace;

  switch (node->nodeKind()) {
    case AST_CallExpr: {
      CASTED_VAR(c, CallExpr, node)
      const FunctionSignature* sig = static_cast<FunctionSignature*>(c->target->resultType);

      uint64 callArgsSize = 0;
      const uint32 argsCount = sig->getArgumentsLength();

      for (uint32 i = 0; i < argsCount; i++) {
        callArgsSize += sig->getArgumentType(i)->stackSizeBytes();
      }

      for (Expr* arg : c->arguments) {
        processScope(arg, ctx);
      }

      if (callArgsSize > ctx.callArgsSpace) {
        ctx.callArgsSpace = callArgsSize;
      }
      return;
    }
    case AST_PropertyAccessExpr: {
      CASTED_VAR(p, PropertyAccessExpr, node)
      processScope(p->target, ctx);
      return;
    }
    case AST_IndexAccessExpr: {
      CASTED_VAR(i, IndexAccessExpr, node)
      processScope(i->target, ctx);
      return;
    }
    case AST_BinaryExpr: {
      CASTED_VAR(b, BinaryExpr, node)
      processScope(b->lhs, ctx);
      processScope(b->rhs, ctx);
      return;
    }
    case AST_UnaryExpr: {
      CASTED_VAR(u, UnaryExpr, node)
      processScope(u->target, ctx);
      return;
    }
    case AST_TernaryExpr: {
      CASTED_VAR(t, TernaryExpr, node)
      processScope(t->condition, ctx);
      processScope(t->left, ctx);
      processScope(t->right, ctx);
      return;
    }
    case AST_ObjectLiteral: {
      CASTED_VAR(l, ObjectLiteral, node)
      for (ObjectLiteralProperty* property : l->properties) {
        processScope(property->value, ctx);
      }
      return;
    }
    case AST_ArrayLiteral: {
      CASTED_VAR(a, ArrayLiteral, node)
      for (Expr* v : a->values) {
        processScope(v, ctx);
      }
      return;
    }

    case AST_Block: {
      CASTED_VAR(b, Block, node)

      for (Statement* statement : b->statements) {
        processScope(statement, ctx);
      }

      if (rollbackBlock) {
        break;
      } else {
        return;
      }
    }
    case AST_IfStatement: {
      CASTED_VAR(i, IfStatement, node)
      processScope(i->condition, ctx);
      processScope(i->body, ctx, false);
      if (i->elseBody) {
        processScope(i->elseBody, ctx, false);
      }
      break;
    }
    case AST_ForStatement: {
      CASTED_VAR(f, ForStatement, node)
      processScope(f->first, ctx);
      processScope(f->loopBody, ctx, false);
      break;
    }
    case AST_WhileStatement: {
      CASTED_VAR(w, WhileStatement, node)
      if (w->doWhile) {
        processScope(w->body, ctx, false);
        processScope(w->condition, ctx);
      } else {
        processScope(w->condition, ctx);
        processScope(w->body, ctx, false);
      }
      break;
    }
    case AST_ReturnStatement: {
      CASTED_VAR(r, ReturnStatement, node)
      if (!r->value) {
        return;
      }
      processScope(r->value, ctx);
      return;
    }
    case AST_LexicalDeclaration:
    case AST_FunctionParam: {
      LocalVarSymbol* lvs = LOOKUP_LOCALVAR(ctx.semantics.getSymbolLookup(), node);
      lvs->setStackOffset(start);
      ctx.variableSpace += lvs->getStackSize();
      ctx.offsetStart += lvs->getStackSize();

      bool inCurrenScope = false;
      for (Symbol* symbol : ctx.scope->getSymbols()) {
        if (symbol != lvs) {
          continue;
        }
        inCurrenScope = true;
        break;
      }

      if (!inCurrenScope) {
        Scope* containing = ctx.semantics.getScopeLookup()[lvs];
        if (containing) {
          containing->removeSymbol(lvs);
          ctx.scope->pushSymbol(lvs);
        }
      }
      return;
    }
    case AST_AssertStatement: {
      CASTED_VAR(a, AssertStatement, node)
      processScope(a->condition, ctx);
      if (a->message) {
        processScope(a->message, ctx);
      }
      return;
    }

    case AST_ExprStatement: {
      CASTED_VAR(e, ExprStatement, node)
      processScope(e->expression, ctx);
      return;
    }

    default:
      return;
  }

  const uint64 newSize = ctx.variableSpace;
  const uint64 dif = newSize - startSize;

  if (dif > ctx.extraSpace) {
    ctx.extraSpace = dif;
  }

  ctx.offsetStart = start;
  ctx.variableSpace = startSize;
}

static void processFunctionScopes(SemanticContext& ctx, const LocalFunction* lf) {
  Scope* scope = lf->getScope();
  FunctionDeclStatement* decl = lf->getDecl();
  Block* body = decl->functionBody;

  ScopeProcessContext procContext = {
    .semantics = ctx,
    .scope = scope,
    .callArgsSpace = 0,
    .extraSpace = 0,
    .variableSpace = 0,
    .offsetStart = 0
  };

  for (FunctionParam* arg : decl->arguments) {
    processScope(arg, procContext);
  }
  processScope(body, procContext, false);

  const uint64 scopeSize
      = procContext.variableSpace
      + procContext.extraSpace
      + procContext.callArgsSpace;

  scope->setStackSize(scopeSize);
}

void SemanticTransformer::processScopes() const {
  processGlobalVarScopes(ctx);
  for (const LocalFunction* lf : ctx.getLocalFunctions()) {
    processFunctionScopes(ctx, lf);
  }
}

SemanticTransformer::SemanticTransformer(SemanticContext& _ctx, ScriptFileStatement* _sfs)
  : ctx(_ctx), sfs(_sfs)
{

}

void SemanticTransformer::run() {
  runOptimizer();
  removeZeroValues();
  createStructConstructors();
  createFileInitMethod();
  flattenNestedFunctions();
  processScopes();
}

void runSemanticTransformer(SemanticContext& ctx, ScriptFileStatement* sfs) {
  SemanticTransformer transformer = SemanticTransformer(ctx, sfs);
  return transformer.run();
}
