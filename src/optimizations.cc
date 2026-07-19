#include "optimizations.h"

#include <complex>

#define IS_NUM_LITERAL(v) (v == AST_IntLiteral || v == AST_FloatLiteral)

Expr* optimizeBinaryOpIfPossible(BinaryExpr* e, NoFreeAllocator* alloc) {
  const astnodetype lkind = e->lhs->nodeKind();
  const astnodetype rkind = e->rhs->nodeKind();

  const binaryop op = e->op;

  Expr* lhs = e->lhs;
  Expr* rhs = e->rhs;

  if (lkind == AST_IntLiteral && rkind == AST_IntLiteral) {
    IntLiteral* lLit = (IntLiteral*) lhs;
    IntLiteral* rLit = (IntLiteral*) rhs;

    const int64 l = lLit->value;
    const int64 r = rLit->value;

    BooleanLiteral bl;
    bl.location = lLit->location;

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
        return alloc->emplace(bl);
      case BOP_GTE:
        bl.value = l >= r;
        return alloc->emplace(bl);
      case BOP_LT:
        bl.value = l < r;
        return alloc->emplace(bl);
      case BOP_LTE:
        bl.value = l <= r;
        return alloc->emplace(bl);
      case BOP_EQ:
        bl.value = l == r;
        return alloc->emplace(bl);
      case BOP_NEQ:
        bl.value = l != r;
        return alloc->emplace(bl);
      default:
        return nullptr;
    }
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
        return nullptr;
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

    switch (op) {
      case BOP_GT:
        bl.value = l > r;
        return alloc->emplace(bl);
      case BOP_GTE:
        bl.value = l >= r;
        return alloc->emplace(bl);
      case BOP_LT:
        bl.value = l < r;
        return alloc->emplace(bl);
      case BOP_LTE:
        bl.value = l <= r;
        return alloc->emplace(bl);
      case BOP_EQ:
        bl.value = l == r;
        return alloc->emplace(bl);
      case BOP_NEQ:
        bl.value = l != r;
        return alloc->emplace(bl);

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
        return nullptr;
    }

    fr->location = e->location;
    return fr;
  }

  return nullptr;
}

Expr* optimizeUnaryOpIfPossible(const UnaryExpr* u) {
  unaryop op = u->op;
  astnodetype kind = u->target->nodeKind();

  // Optimize away the unary expression if we're evaluating  a literal value
  if (op == UOP_NEG) {
    if (kind == AST_FloatLiteral) {
      FloatLiteral* ftarget = (FloatLiteral*) u->target;
      ftarget->value = -ftarget->value;
      ftarget->location = u->location;
      return ftarget;
    }
    if (kind == AST_IntLiteral) {
      IntLiteral* ftarget = (IntLiteral*) u->target;
      ftarget->value = -ftarget->value;
      ftarget->location = u->location;
      return ftarget;
    }
  } else if (op == UOP_POS) {
    // What the fuck is the point of the unary plus operator
    // im crine
    if (kind == AST_FloatLiteral || kind == AST_IntLiteral) {
      return u->target;
    }
  }

  return nullptr;
}

Expr* recursivelyOptimize(Expr* e, NoFreeAllocator* allocator) {
  astnodetype kind = e->nodeKind();

  switch (kind) {
    case AST_UnaryExpr: {
      UnaryExpr* unary = (UnaryExpr*) e;
      unary->target = recursivelyOptimize(unary->target, allocator);

      Expr* r = optimizeUnaryOpIfPossible(unary);
      if (!r) {
        return e;
      }

      return r;
    }
    case AST_BinaryExpr: {
      BinaryExpr* bin = (BinaryExpr*) e;
      bin->lhs = recursivelyOptimize(bin->lhs, allocator);
      bin->rhs = recursivelyOptimize(bin->rhs, allocator);

      Expr* r = optimizeBinaryOpIfPossible(bin, allocator);
      if (r) {
        return r;
      }

      return e;
    }
    case AST_CallExpr: {
      CallExpr* call = (CallExpr*) e;
      for (uint32 i = 0; i < call->arguments.size(); i++) {
        call->arguments[i] = recursivelyOptimize(call->arguments[i], allocator);
      }
      return call;
    }
    case AST_IndexAccessExpr: {
      IndexAccessExpr* iae = (IndexAccessExpr*) e;
      iae->index = recursivelyOptimize(iae->index, allocator);
      return iae;
    }
    default:
      return e;
  }
}
