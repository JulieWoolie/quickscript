
#include "parser.h"

#define FATAL(...) m_errors->fatal(__VA_ARGS__)
#define ERROR(...) m_errors->error(__VA_ARGS__)
#define WARN(...) m_errors->warn(__VA_ARGS__)
#define INFO(...) m_errors->info(__VA_ARGS__)

#define EMPLACE(v) m_pool->emplace(v)

Parser::Parser(TokenList *tokens, NodePool *pool, CompilerErrors *errors, StringTable* table) {
  m_tokens = tokens;
  m_pool = pool;
  m_errors = errors;

  m_tokenCursor = 0;

  m_nameTable = table;
}

Token* Parser::gettoken(uint32 idx) const {
  if (idx >= m_tokens->size()) {
    return m_tokens->get(m_tokens->size() - 1);
  }
  return m_tokens->get(idx);
}

Token* Parser::next() {
  Token* t = gettoken(m_tokenCursor);
  if (t->ttype != TT_EOF) {
    m_tokenCursor++;
  }
  return t;
}

Token* Parser::peek() const {
  return peek(0);
}

Token* Parser::peek(uint32 ahead) const {
  return gettoken(m_tokenCursor + ahead);
}

void Parser::skip() {
  if (m_tokenCursor >= m_tokens->size()) {
    return;
  }
  m_tokenCursor++;
}

bool Parser::hasNext() const {
  return m_tokenCursor < m_tokens->size();
}

bool Parser::is(const tokentype tt) const {
  return peek()->ttype == tt;
}

bool Parser::is(const uint32 ahead, const tokentype tt) const {
  return peek(ahead)->ttype == tt;
}

Token* Parser::expect(const tokentype tt) {
  Token* t = next();
  if (t->ttype == tt) {
    return t;
  }
  FATAL(t->start, "Expected %s, found %s", tokentype_name(tt), tokentype_name(t->ttype));
  return t;
}

#define RECURSIVE_FUNC(name, binop, ttype, calls) \
  NodeRef<Expr> Parser::name() {\
    Location l = peek()->start;\
    NodeRef<Expr> expr = calls();\
    while (is(ttype)) {\
      skip();\
      \
      BinaryExpr bin;\
      bin.location = l;\
      bin.op = binop;\
      bin.lhs = expr;\
      bin.rhs = calls();\
      \
      expr = m_pool->emplace(bin);\
    }\
    return expr;\
  }

#define BINOP_CASE(ttype, bop) \
  case ttype:\
    bin.op = bop;\
    break;

NodeRef<Expr> Parser::expr() {
  return assignExpr();
}

NodeRef<Expr> Parser::ternaryExpr() {
  Location l = peek()->start;
  NodeRef<Expr> expr = logicalOrExpr();

  if (!is(TT_QUESTION)) {
    return expr;
  }

  TernaryExpr tern;
  tern.location = l;
  tern.condition = expr;

  NodeRef<Expr> left = assignExpr();
  expect(TT_COLON);
  NodeRef<Expr> right = assignExpr();

  tern.left = left;
  tern.right = right;

  return EMPLACE(tern);
}

binaryop mapTokenToOp(tokentype tt) {
  switch (tt) {
    case TT_BIT_AND_ASSIGN:
      return BOP_ASSIGN_BIT_AND;
    case TT_LOGICAL_AND_ASSIGN:
      return BOP_ASSIGN_LOG_AND;
    case TT_WALL_ASSIGN:
      return BOP_ASSIGN_BIT_OR;
    case TT_STAR_ASSIGN:
      return BOP_ASSIGN_MUL;
    case TT_SLASH_ASSIGN:
      return BOP_ASSIGN_DIV;
    case TT_PLUS_ASSIGN:
      return BOP_ASSIGN_ADD;
    case TT_MINUS_ASSIGN:
      return BOP_ASSIGN_SUB;
    case TT_XOR_ASSIGN:
      return BOP_ASSIGN_XOR;
    case TT_PERCENT_ASSIGN:
      return BOP_ASSIGN_MOD;
    case TT_SHL_ASSIGN:
      return BOP_ASSIGN_SHL;
    case TT_SHR_ASSIGN:
      return BOP_ASSIGN_SHR;
    case TT_USHR_ASSIGN:
      return BOP_ASSIGN_USHR;
    case TT_POW_ASSIGN:
      return BOP_ASSIGN_EXP;
    case TT_DWALL_ASSIGN:
      return BOP_ASSIGN_LOG_AND;
    default:
      return BOP_NIL;
  }
}

NodeRef<Expr> Parser::assignExpr() {
  Location l = peek()->start;
  NodeRef<Expr> expr = ternaryExpr();

  Token* t = peek();
  binaryop op = mapTokenToOp(t->ttype);

  if (!(op & BOP_ASSIGN_FLAG)) {
    return expr;
  }

  next();

  BinaryExpr assign;
  assign.lhs = expr;
  assign.rhs = assignExpr();
  assign.op = op;
  assign.location = l;

  return EMPLACE(assign);
}

RECURSIVE_FUNC(logicalOrExpr, BOP_LOG_OR, TT_DWALL, bitwiseOrExpr)
RECURSIVE_FUNC(logicalAndExpr, BOP_LOG_AND, TT_LOGICAL_AND, bitwiseOrExpr)
RECURSIVE_FUNC(bitwiseOrExpr, BOP_BIT_OR, TT_WALL, bitwiseAndExpr)
RECURSIVE_FUNC(bitwiseAndExpr, BOP_BIT_AND, TT_BIT_AND, bitwiseXorExpr)
RECURSIVE_FUNC(bitwiseXorExpr, BOP_XOR, TT_XOR, equalityExpr)

NodeRef<Expr> Parser::equalityExpr() {
  Location l = peek()->start;
  NodeRef<Expr> prev = relationalExpr();

  Token* p = peek();
  tokentype ttype = p->ttype;

  while (ttype == TT_EQ || ttype == TT_NEQ) {
    skip();

    BinaryExpr bin;
    bin.location = l;
    bin.lhs = prev;

    switch (ttype) {
      BINOP_CASE(TT_EQ, BOP_EQ)
      BINOP_CASE(TT_NEQ, BOP_NEQ)
    }

    bin.rhs = relationalExpr();
    prev = EMPLACE(bin);

    p = peek();
    ttype = p->ttype;
  }

  return prev;
}

NodeRef<Expr> Parser::relationalExpr() {
  Location l = peek()->start;
  NodeRef<Expr> prev = shiftExpr();

  Token* p = peek();
  tokentype ttype = p->ttype;

  while (ttype == TT_LT || ttype == TT_LTE || ttype == TT_GT || ttype == TT_GTE) {
    skip();

    BinaryExpr bin;
    bin.location = l;
    bin.lhs = prev;

    switch (ttype) {
      BINOP_CASE(TT_LT, BOP_LT)
      BINOP_CASE(TT_LTE, BOP_LTE)
      BINOP_CASE(TT_GT, BOP_GT)
      BINOP_CASE(TT_GTE, BOP_GTE)
    }

    bin.rhs = shiftExpr();
    prev = EMPLACE(bin);

    p = peek();
    ttype = p->ttype;
  }

  return prev;
}

NodeRef<Expr> Parser::shiftExpr() {
  Location l = peek()->start;
  NodeRef<Expr> prev = additiveExpr();

  Token* p = peek();
  tokentype ttype = p->ttype;

  while (ttype == TT_SHL || ttype == TT_SHR || ttype == TT_USHR) {
    skip();

    BinaryExpr bin;
    bin.location = l;
    bin.lhs = prev;

    switch (ttype) {
      case TT_SHL:
        bin.op = BOP_SHL;
        break;
      case TT_SHR:
        bin.op = BOP_SHR;
        break;
      case TT_USHR:
        bin.op = BOP_USHR;
        break;
    }

    bin.rhs = additiveExpr();
    prev = EMPLACE(bin);

    p = peek();
    ttype = p->ttype;
  }

  return prev;
}

NodeRef<Expr> Parser::additiveExpr() {
  Location l = peek()->start;
  NodeRef<Expr> prev = multiplicativeExpr();

  Token* p = peek();
  tokentype ttype = p->ttype;

  while (ttype == TT_PLUS || ttype == TT_MINUS) {
    skip();

    BinaryExpr bin;
    bin.location = l;
    bin.lhs = prev;

    switch (ttype) {
      case TT_PLUS:
        bin.op = BOP_ADD;
        break;
      case TT_MINUS:
        bin.op = BOP_SUB;
        break;
    }

    bin.rhs = multiplicativeExpr();
    prev = EMPLACE(bin);

    p = peek();
    ttype = p->ttype;
  }

  return prev;
}

NodeRef<Expr> Parser::multiplicativeExpr() {
  Location l = peek()->start;
  NodeRef<Expr> prev = exponentialExpr();

  Token* p = peek();
  tokentype ttype = p->ttype;

  while (ttype == TT_SLASH || ttype == TT_STAR || ttype == TT_PERCENT) {
    skip();

    BinaryExpr bin;
    bin.location = l;
    bin.lhs = prev;

    switch (ttype) {
      case TT_SLASH:
        bin.op = BOP_DIV;
        break;
      case TT_STAR:
        bin.op = BOP_MUL;
        break;
      case TT_PERCENT:
        bin.op = BOP_MOD;
        break;
    }

    bin.rhs = exponentialExpr();
    prev = EMPLACE(bin);

    p = peek();
    ttype = p->ttype;
  }

  return prev;
}

RECURSIVE_FUNC(exponentialExpr, BOP_POW, TT_POW, unaryExpr)

NodeRef<Expr> Parser::unaryExpr() {
  Token* p = peek();

  UnaryExpr u;
  u.location = p->start;

  switch (p->ttype) {
    case TT_BIT_INVERT:
      u.op = UOP_BIT_NOT;
      break;
    case TT_INVERT:
      u.op = UOP_LOG_NOT;
      break;
    case TT_INC:
      u.op = UOP_PREINC;
      break;
    case TT_DEC:
      u.op = UOP_PREDEC;
      break;
    case TT_PLUS:
      u.op = UOP_POS;
      break;
    case TT_MINUS:
      u.op = UOP_NEG;
      break;

    default:
      u.target = memberExpr(true);

      if (!is(TT_INC) || is(TT_DEC)) {
        return EMPLACE(u);
      }

      tokentype tt = next()->ttype;
      u.op = tt == TT_DEC ? UOP_POSTDEC : UOP_PREDEC;

      return EMPLACE(u);
  }

  next();
  u.target = unaryExpr();

  return EMPLACE(u);
}

NodeRef<Expr> Parser::memberExpr(const bool allowCall) {
  NodeRef<Expr> expr = primaryExpr();
  return memberExprTail(allowCall, expr);
}

NodeRef<Expr> Parser::memberExprTail(const bool allowCall, const NodeRef<Expr> target) {
  NodeRef<Expr> expr = target;

  while (true) {
    if (is(TT_DOT)) {
      expr = propertyAccess(target);
      continue;
    }
    if (is(TT_LBRACKET)) {
      if (!allowCall) {
        break;
      }
      expr = callExpr(expr);
      continue;
    }
    break;
  }

  return expr;
}

NodeRef<Expr> Parser::propertyAccess(NodeRef<Expr> target) {
  Token* t = expect(TT_DOT);

  PropertyAccessExpr expr;
  expr.location = t->start;
  expr.target = target;

  NodeRef<Identifier> propId = id();
  expr.property = propId;

  return EMPLACE(expr);
}

NodeRef<Expr> Parser::callExpr(NodeRef<Expr> target) {
  Token* start = expect(TT_LBRACKET);

  CallExpr call;
  call.location = start->start;
  call.target = target;

  Token* p = peek();
  while (p->ttype != TT_RBRACKET && p->ttype != TT_EOF) {
    NodeRef<Expr> arg = expr();
    call.arguments.push_back(arg);

    if (is(TT_COMMA)) {
      skip();
      p = peek();
      if (p->ttype == TT_RBRACKET || p->ttype == TT_EOF) {
        ERROR(p->start, "Illegal trailing comma in function call arguments");
      }
    } else if (!is(TT_RBRACKET)) {
      ERROR("Unexpected token in function call arguments");
    }
  }

  expect(TT_RBRACKET);

  return EMPLACE(call);
}

NodeRef<Expr> Parser::primaryExpr() {
  Token* p = peek();

  switch (p->ttype) {
    case TT_ID:
      return id();
    case TT_STRING_LITERAL:
      return stringLiteral();

    case TT_INT_LITERAL:
    case TT_HEX_LITERAL:
    case TT_OCT_LITERAL:
    case TT_BIN_LITERAL:
      return intLiteral();

    case TT_FLOAT_LITERAL:
      return floatLiteral();

    case TT_KEYW_TRUE:
    case TT_KEYW_FALSE:
      return boolLiteral();

    case TT_LBRACKET:
      return parenthesizedExpr();

    default:
      FATAL(p->start, "This token was not expected here");
      return UNSET_REF;
  }
}

NodeRef<Expr> Parser::parenthesizedExpr() {
  expect(TT_LBRACKET);
  NodeRef<Expr> res = expr();
  expect(TT_RBRACKET);
  return res;
}

int64 stringTo64(cstring str, uint32 len) {
  int64 res = 0;
  for (uint32 i = 0; i < len; i++) {
    res = res * 10 + (str[i] - '0');
  }
  return res;
}
int64 binTo64(cstring str, uint32 len) {
  int64 res = 0;
  for (uint32 i = 0; i < len; i++) {
    res = (res << 1) + (str[i] - '0');
  }
  return res;
}
int64 octTo64(cstring str, uint32 len) {
  int64 res = 0;
  for (uint32 i = 0; i < len; i++) {
    res = (res << 3) + (str[i] - '0');
  }
  return res;
}
int64 hexTo64(cstring str, uint32 len) {
  int64 res = 0;
  for (uint32 i = 0; i < len; i++) {
    int64 digit;
    int8 ch = str[i];

    if (ch <= '9')
      digit = ch - '0';
    else if (ch <= 'F')
      digit = ch - 'A' + 10;
    else
      digit = ch - 'a' + 10;

    res = (res << 4) + digit;
  }
  return res;
}

NodeRef<IntLiteral> Parser::intLiteral() const {
  Token* p = peek();

  uint32 buflen = 128;
  char numtxt[buflen];

  if (p->valueId != EMPTY_STRING) {
    buflen = m_nameTable->getchars(p->valueId, numtxt, buflen);
  } else {
    buflen = 0;
  }

  IntLiteral lit;
  lit.location = p->start;

  switch (p->ttype) {
    case TT_INT_LITERAL:
      lit.value = stringTo64(numtxt, buflen);
      return m_pool->emplace(lit);
    case TT_BIN_LITERAL:
      lit.value = binTo64(numtxt, buflen);
      return m_pool->emplace(lit);
    case TT_OCT_LITERAL:
      lit.value = octTo64(numtxt, buflen);
      return m_pool->emplace(lit);
    case TT_HEX_LITERAL:
      lit.value = hexTo64(numtxt, buflen);
      return m_pool->emplace(lit);
    default:
      FATAL(p->start, "Expected integer literal, found %s", tokentype_name(p->ttype));
      return UNSET_REF;
  }
}

NodeRef<BooleanLiteral> Parser::boolLiteral() const {
  Token* t = peek();

  if (t->ttype != TT_KEYW_TRUE && t->ttype != TT_KEYW_FALSE) {
    FATAL(t->start, "Expected either 'true' or 'false', found '%s", tokentype_name(t->ttype));
    return UNSET_REF;
  }

  BooleanLiteral lit;
  lit.value = t->ttype == TT_KEYW_TRUE;
  lit.location = t->start;

  return EMPLACE(lit);
}

NodeRef<FloatLiteral> Parser::floatLiteral() {
  const Token* t = expect(TT_FLOAT_LITERAL);

  char content[256];
  m_nameTable->getchars(t->valueId, content, 256);

  float64 v = strtod(content, nullptr);

  FloatLiteral lit;
  lit.location = t->start;
  lit.value = v;

  return EMPLACE(lit);
}

NodeRef<StringLiteral> Parser::stringLiteral() {
  const Token* t = expect(TT_STRING_LITERAL);
  auto [id, ref] = m_pool->make<StringLiteral>();
  id->location = t->start;
  id->value = t->valueId;
  return ref;
}

NodeRef<Identifier> Parser::id() {
  const Token* t = expect(TT_ID);
  auto [id, ref] = m_pool->make<Identifier>();
  id->location = t->start;
  id->value = t->valueId;
  return ref;
}


