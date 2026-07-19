
#include "parser.h"
#include "../optimizations.h"

#define FATAL(...) m_errors->fatal(__VA_ARGS__)
#define ERROR(...) m_errors->error(__VA_ARGS__)
#define WARN(...) m_errors->warn(__VA_ARGS__)
#define INFO(...) m_errors->info(__VA_ARGS__)

#define EMPLACE(v) m_pool->emplace(v)

#define RECURSIVE_FUNC(name, cond, calls) \
Expr* Parser::name() {\
  Location l = peek()->start;\
  Expr* expr = calls();\
\
  Token* p = peek();\
  tokentype ttype = p->ttype;\
\
  while (cond) {\
    skip();\
\
    BinaryExpr bin;\
    bin.location = l;\
    bin.op = mapTokenToOp(ttype);\
    bin.lhs = expr;\
    bin.rhs = calls();\
\
    Expr* optimized = optimizeBinaryOpIfPossible(&bin, m_pool);\
    if (optimized) {\
      expr = optimized;\
    } else {\
      expr = m_pool->emplace(bin);\
    }\
\
    p = peek();\
    ttype = p->ttype;\
  }\
  return expr;\
}

#define SAVECURSOR uint32 c = m_tokenCursor;
#define RESTORECURSOR m_tokenCursor = c;

Parser::Parser(TokenList *tokens, NoFreeAllocator *pool, CompilerErrors *errors, StringTable* table) {
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

ScriptFileStatement * Parser::parse() {
  ScriptFileStatement sfs;
  sfs.location = {
    .index = 0,
    .line = 1,
    .column = 1
  };

  while (!is(TT_EOF)) {
    Statement* s = statement();
    sfs.statements.push_back(s);
  }

  return EMPLACE(sfs);
}

uint8 Parser::isLexOrFuncDecl() {
  SAVECURSOR

  tokentype tt = peek()->ttype;
  bool canBeLabelled = true;

  switch (tt) {
    case TT_KEYW_BOOL:
    case TT_KEYW_UINT8:
    case TT_KEYW_INT8:
    case TT_KEYW_UINT16:
    case TT_KEYW_INT16:
    case TT_KEYW_UINT32:
    case TT_KEYW_INT32:
    case TT_KEYW_UINT64:
    case TT_KEYW_INT64:
    case TT_KEYW_FLOAT32:
    case TT_KEYW_FLOAT64:
    case TT_KEYW_STRING:
      canBeLabelled = false;
    case TT_ID:
      break;
    default:
      return LFDL_NONE;
  }

  if (is(TT_COLON) && canBeLabelled) {
    RESTORECURSOR;
    return LFDL_LABELLED_LOOP;
  }

  next();
  while (is(TT_LSQUARE)) {
    next();

    if (!is(TT_RSQUARE)) {
      RESTORECURSOR
      return LFDL_NONE;
    }

    next();
  }

  if (!is(TT_ID)) {
    RESTORECURSOR
    return LFDL_NONE;
  }

  next();

  if (is(TT_ASSIGN)) {
    RESTORECURSOR
    return LFDL_LEX;
  }
  if (!is(TT_LBRACKET)) {
    RESTORECURSOR
    return LFDL_NONE;
  }

  RESTORECURSOR
  return LFDL_FUNC;
}

Statement* Parser::statement() {
  Token* t = peek();

  switch (t->ttype) {
    case TT_KEYW_IF:
      return ifStatement();
    case TT_KEYW_DO:
      return doWhileStatement(nullptr);
    case TT_KEYW_WHILE:
      return whileStatement(nullptr);
    case TT_KEYW_FOR:
      return forStatement(nullptr);
    case TT_KEYW_CONTINUE:
    case TT_KEYW_BREAK:
      return controlFlow();
    case TT_KEYW_RETURN:
      return returnStatement();
    case TT_LCURL:
      return block();
    case TT_KEYW_STRUCT:
      return structDecl();
    case TT_KEYW_CONST:
      return lexDecl();

    default:
      uint8 lfdl = isLexOrFuncDecl();
      if (lfdl == LFDL_LEX) {
        return lexDecl();
      }
      if (lfdl == LFDL_FUNC) {
        return funcDecl();
      }
      if (lfdl == LFDL_LABELLED_LOOP) {
        return labelledStatement();
      }

      Expr* e = expr();
      ExprStatement stat;
      stat.location = e->location;
      stat.expression = e;
      return EMPLACE(stat);
  }
}

FunctionParam * Parser::funcParam() {
  TypeExpr* ptype = typeExpr();
  Identifier* pname = id();

  FunctionParam param;
  param.location = ptype->location;
  param.name = pname;
  param.paramType = ptype;
  
  if (is(TT_THREE_DOTS)) {
    next();
    param.varargs = true;
  } else {
    param.varargs = false;
  }

  return EMPLACE(param);
}

FunctionDeclStatement * Parser::funcDecl() {
  TypeExpr* retType = typeExpr();
  Identifier* funcName = id();

  FunctionDeclStatement decl;
  decl.location = retType->location;
  decl.name = funcName;
  decl.returnType = retType;

  expect(TT_LBRACKET);

  while (!is(TT_RBRACKET)) {
    FunctionParam* param = funcParam();
    decl.arguments.push_back(param);

    if (is(TT_COMMA)) {
      next();
      if (is(TT_RBRACKET)) {
        ERROR(peek()->start, "Illegal trailing comma in function arguments declaration");
      }
    }
  }

  expect(TT_RBRACKET);

  decl.functionBody = block();

  return EMPLACE(decl);
}

Block* Parser::block() {
  Token* t = expect(TT_LCURL);

  Block b;
  b.location = t->start;

  while (!is(TT_RCURL)) {
    Statement* s = statement();
    b.statements.push_back(s);
  }

  expect(TT_RCURL);

  return EMPLACE(b);
}

ControlFlowStatement * Parser::controlFlow() {
  Token* p = next();
  tokentype tt = p->ttype;
  controlflowtype cft = CFT_NIL;

  if (tt == TT_KEYW_CONTINUE) {
    cft = CFT_CONTINUE;
  } else if (tt == TT_KEYW_BREAK) {
    cft = CFT_BREAK;
  } else {
    // Should never happen as long as this is called from statement()
    return nullptr;
  }

  ControlFlowStatement stat;
  stat.location = p->start;
  stat.type = cft;

  p = peek();
  if (p->ttype == TT_ID && p->start.line == stat.location.line) {
    Identifier* label = id();
    stat.label = label;
  }

  return EMPLACE(stat);
}

IfStatement* Parser::ifStatement() {
  Token* t = expect(TT_KEYW_IF);

  IfStatement s;
  s.location = t->start;

  Expr* cond = expr();
  Statement* body = statement();

  s.condition = cond;
  s.body = body;

  if (is(TT_KEYW_ELSE)) {
    next();
    s.elseBody = statement();
  }

  return EMPLACE(s);
}

Statement* Parser::labelledStatement() {
  Identifier* label = id();
  expect(TT_COLON);

  Token* t = peek();

  switch (t->ttype) {
    case TT_KEYW_FOR:
      return forStatement(label);
    case TT_KEYW_WHILE:
      return whileStatement(label);
    case TT_KEYW_DO:
      return doWhileStatement(label);
    default:
      FATAL(t->start, "Invalid statement for labelled statement");
      return nullptr;
  }
}

ForStatement * Parser::forStatement(Identifier* label) {
  const Token* t = expect(TT_KEYW_FOR);

  ForStatement f;
  f.location = t->start;
  f.label = label;

  bool hasParentheses = false;

  if (is(TT_LBRACKET)) {
    hasParentheses = true;
    next();
  }

  f.first = lexDecl();
  expect(TT_SEMICOLON);
  f.second = expr();
  expect(TT_SEMICOLON);
  f.third = expr();

  if (hasParentheses) {
    expect(TT_RBRACKET);
  }

  f.loopBody = block();

  return EMPLACE(f);
}

DoWhileStatement* Parser::doWhileStatement(Identifier* label) {
  const Token* t = expect(TT_KEYW_DO);

  DoWhileStatement dow;
  dow.location = t->start;
  dow.label = label;
  dow.body = block();

  expect(TT_KEYW_WHILE);
  dow.condition = expr();

  return EMPLACE(dow);
}

WhileStatement* Parser::whileStatement(Identifier* label) {
  const Token* t = expect(TT_KEYW_WHILE);

  WhileStatement dow;
  dow.location = t->start;
  dow.label = label;
  dow.condition = expr();
  dow.body = block();

  return EMPLACE(dow);
}

LexicalDeclaration * Parser::lexDecl() {
  bool isConst = is(TT_KEYW_CONST);
  Location loc;

  if (isConst) {
    loc = next()->start;
  } else {
    loc = peek()->start;
  }

  TypeExpr* te = typeExpr();
  Identifier* name = id();
  Expr* val = nullptr;

  if (is(TT_ASSIGN)) {
    next();
    val = expr();
  }

  LexicalDeclaration lex;
  lex.location = loc;
  lex.typeExpr = te;
  lex.variableName = name;
  lex.value = val;
  lex.isConstDeclaration = isConst;

  return EMPLACE(lex);
}

ReturnStatement* Parser::returnStatement() {
  Token* t = expect(TT_KEYW_RETURN);

  ReturnStatement r;
  r.location = t->start;

  if (is(TT_RCURL) || is(TT_EOF)) {
    return EMPLACE(r);
  }
  if (peek()->start.line != r.location.line) {
    return EMPLACE(r);
  }

  r.value = expr();

  return EMPLACE(r);
}

StructDecl* Parser::structDecl() {
  Token* t = expect(TT_KEYW_STRUCT);
  StructDecl decl;
  decl.location = t->start;
  decl.name = id();

  expect(TT_LCURL);

  while (!is(TT_RCURL)) {
    StructPropertyDecl* prop = structProperty();
    decl.properties.push_back(prop);
  }

  expect(TT_RCURL);

  StructDecl* result = EMPLACE(decl);
  for (StructPropertyDecl* prop : result->properties) {
    prop->structDeclStatement = result;
  }
  return result;
}

StructPropertyDecl* Parser::structProperty() {
  TypeExpr* t = typeExpr();

  StructPropertyDecl prop;
  prop.location = t->location;
  prop.propertyType = t;
  prop.name = id();

  if (is(TT_ASSIGN)) {
    next();
    prop.value = expr();
  }

  return EMPLACE(prop);
}

TypeExpr* Parser::typeExpr() {
  return arrayType();
}

TypeExpr* Parser::arrayType() {
  TypeExpr* typeExpr = primaryTypeExpr();

  while (is(TT_LSQUARE)) {
    next();
    expect(TT_RSQUARE);

    ArrayTypeExpr arr;
    arr.location = typeExpr->location;
    arr.componentType = typeExpr;

    typeExpr = EMPLACE(arr);
  }

  return typeExpr;
}

TypeExpr * Parser::primaryTypeExpr() {
  Token* t = peek();
  tokentype tt = t->ttype;

  switch (tt) {
    case TT_KEYW_BOOL:
    case TT_KEYW_UINT8:
    case TT_KEYW_INT8:
    case TT_KEYW_UINT16:
    case TT_KEYW_INT16:
    case TT_KEYW_UINT32:
    case TT_KEYW_INT32:
    case TT_KEYW_UINT64:
    case TT_KEYW_INT64:
    case TT_KEYW_FLOAT32:
    case TT_KEYW_FLOAT64:
    case TT_KEYW_STRING:
    case TT_KEYW_CONST:
      return primitiveType();
    case TT_ID:
      return typeName();
    default:
      FATAL(t->start, "Don't know how to parse %s into a type name", tokentype_name(tt));
      return nullptr;
  }
}

TypeExpr* Parser::typeName() {
  Token* token = expect(TT_ID);
  TypeNameExpr e;
  e.location = token->start;
  e.typeName = token->valueId;
  return EMPLACE(e);
}

PrimitiveTypeExpr* Parser::primitiveType() {
  parsedprimitivetype pt = PPT_NIL;

  switch (peek()->ttype) {
    case TT_KEYW_BOOL:
      pt = PPT_BOOL;
      break;
    case TT_KEYW_UINT8:
      pt = PPT_UINT8;
      break;
    case TT_KEYW_INT8:
      pt = PPT_INT8;
      break;
    case TT_KEYW_UINT16:
      pt = PPT_UINT16;
      break;
    case TT_KEYW_INT16:
      pt = PPT_INT16;
      break;
    case TT_KEYW_UINT32:
      pt = PPT_UINT32;
      break;
    case TT_KEYW_INT32:
      pt = PPT_INT32;
      break;
    case TT_KEYW_UINT64:
      pt = PPT_UINT64;
      break;
    case TT_KEYW_INT64:
      pt = PPT_INT64;
      break;
    case TT_KEYW_FLOAT32:
      pt = PPT_FLOAT32;
      break;
    case TT_KEYW_FLOAT64:
      pt = PPT_FLOAT64;
      break;
    case TT_KEYW_STRING:
      pt = PPT_STRING;
      break;
    case TT_KEYW_CONST:
      pt = PPT_VOID;
      break;
    default:
      break;
  }

  if (pt == PPT_NIL) {
    FATAL("Expected primitive keyword here");
  }

  Token* t = next();

  PrimitiveTypeExpr pte;
  pte.location = t->start;
  pte.primType = pt;
  return EMPLACE(pte);
}

Expr* Parser::expr() {
  return assignExpr();
}

Expr* Parser::ternaryExpr() {
  Location l = peek()->start;
  Expr* expr = logicalOrExpr();

  if (!is(TT_QUESTION)) {
    return expr;
  }

  TernaryExpr tern;
  tern.location = l;
  tern.condition = expr;

  Expr* left = assignExpr();
  expect(TT_COLON);
  Expr* right = assignExpr();

  tern.left = left;
  tern.right = right;

  return EMPLACE(tern);
}

binaryop mapTokenToOp(tokentype tt) {
  switch (tt) {
    case TT_BIT_AND_ASSIGN: return BOP_ASSIGN_BIT_AND;
    case TT_LOGICAL_AND_ASSIGN: return BOP_ASSIGN_LOG_AND;
    case TT_WALL_ASSIGN: return BOP_ASSIGN_BIT_OR;
    case TT_STAR_ASSIGN: return BOP_ASSIGN_MUL;
    case TT_SLASH_ASSIGN: return BOP_ASSIGN_DIV;
    case TT_PLUS_ASSIGN: return BOP_ASSIGN_ADD;
    case TT_MINUS_ASSIGN: return BOP_ASSIGN_SUB;
    case TT_XOR_ASSIGN: return BOP_ASSIGN_XOR;
    case TT_PERCENT_ASSIGN: return BOP_ASSIGN_MOD;
    case TT_SHL_ASSIGN: return BOP_ASSIGN_SHL;
    case TT_SHR_ASSIGN: return BOP_ASSIGN_SHR;
    case TT_USHR_ASSIGN: return BOP_ASSIGN_USHR;
    case TT_POW_ASSIGN: return BOP_ASSIGN_POW;
    case TT_DWALL_ASSIGN: return BOP_ASSIGN_LOG_OR;

    case TT_GT: return BOP_GT;
    case TT_LT: return BOP_LT;
    case TT_GTE: return BOP_GTE;
    case TT_LTE: return BOP_LTE;
    case TT_EQ: return BOP_EQ;
    case TT_NEQ: return BOP_NEQ;
    case TT_PLUS: return BOP_ADD;
    case TT_MINUS: return BOP_SUB;
    case TT_STAR: return BOP_MUL;
    case TT_SLASH: return BOP_DIV;
    case TT_PERCENT: return BOP_MOD;
    case TT_POW: return BOP_POW;
    case TT_SHL: return BOP_SHL;
    case TT_SHR: return BOP_SHR;
    case TT_USHR: return BOP_USHR;
    case TT_XOR: return BOP_XOR;
    case TT_DWALL: return BOP_LOG_OR;
    case TT_LOGICAL_AND: return BOP_LOG_AND;
    case TT_WALL: return BOP_BIT_OR;
    case TT_BIT_AND: return BOP_BIT_AND;

    default: return BOP_NIL;
  }
}

Expr* Parser::assignExpr() {
  Location l = peek()->start;
  Expr* expr = ternaryExpr();

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

RECURSIVE_FUNC(
  logicalOrExpr,
  ttype == TT_DWALL,
  logicalAndExpr
)
RECURSIVE_FUNC(
  logicalAndExpr,
  ttype == TT_LOGICAL_AND,
  bitwiseOrExpr
)
RECURSIVE_FUNC(
  bitwiseOrExpr,
  ttype == TT_WALL,
  bitwiseAndExpr
)
RECURSIVE_FUNC(
  bitwiseAndExpr,
  ttype == TT_BIT_AND,
  bitwiseXorExpr
)
RECURSIVE_FUNC(
  bitwiseXorExpr,
  ttype == TT_XOR,
  equalityExpr
)

RECURSIVE_FUNC(
  equalityExpr,
  ttype == TT_EQ || ttype == TT_NEQ,
  relationalExpr
)
RECURSIVE_FUNC(
  relationalExpr,
  ttype == TT_LT || ttype == TT_LTE || ttype == TT_GT || ttype == TT_GTE,
  shiftExpr
)
RECURSIVE_FUNC(
  shiftExpr,
  ttype == TT_SHL || ttype == TT_SHR || ttype == TT_USHR,
  additiveExpr
)
RECURSIVE_FUNC(
  additiveExpr,
  ttype == TT_PLUS || ttype == TT_MINUS,
  multiplicativeExpr
)
RECURSIVE_FUNC(
  multiplicativeExpr,
  ttype == TT_SLASH || ttype == TT_STAR || ttype == TT_PERCENT,
  exponentialExpr
)

RECURSIVE_FUNC(
  exponentialExpr,
  ttype == TT_POW,
  unaryExpr
)

Expr* Parser::unaryExpr() {
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
      Expr* target = memberExpr(true);

      if (!is(TT_INC) || is(TT_DEC)) {
        return target;
      }

      tokentype tt = next()->ttype;
      u.op = tt == TT_DEC ? UOP_POSTDEC : UOP_PREDEC;
      u.target = target;

      return EMPLACE(u);
  }

  next();
  u.target = unaryExpr();

  Expr* opt = optimizeUnaryOpIfPossible(&u);
  if (opt) {
    return opt;
  }

  return EMPLACE(u);
}

Expr* Parser::memberExpr(const bool allowCall) {
  Expr* expr = primaryExpr();
  return memberExprTail(allowCall, expr);
}

Expr* Parser::memberExprTail(const bool allowCall, Expr* target) {
  Expr* expr = target;

  while (true) {
    if (is(TT_DOT)) {
      expr = propertyAccess(expr);
      continue;
    }
    if (is(TT_LSQUARE)) {
      expr = indexAccess(expr);
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

Expr* Parser::propertyAccess(Expr* target) {
  const Token* t = expect(TT_DOT);

  PropertyAccessExpr expr;
  expr.location = t->start;
  expr.target = target;

  Identifier* propId = id();
  expr.property = propId;

  return EMPLACE(expr);
}

Expr* Parser::indexAccess(Expr *target) {
  const Token* t = expect(TT_LSQUARE);

  IndexAccessExpr access;
  access.location = t->start;
  access.target = target;

  Expr* propId = expr();
  access.index = propId;

  expect(TT_RSQUARE);

  return EMPLACE(access);
}

Expr* Parser::callExpr(Expr* target) {
  const Token* start = expect(TT_LBRACKET);

  CallExpr call;
  call.location = start->start;
  call.target = target;

  while (!is(TT_RBRACKET) && !is(TT_EOF)) {
    Expr* arg = expr();
    call.arguments.push_back(arg);

    Token* p = peek();

    if (p->ttype == TT_COMMA) {
      skip();
      p = peek();

      if (p->ttype == TT_RBRACKET || p->ttype == TT_EOF) {
        FATAL("Illegal trailing comma in function call arguments");
      }

      continue;
    }

    if (p->ttype == TT_EOF) {
      FATAL("Call illegal cutoff");
    }
  }

  expect(TT_RBRACKET);

  return EMPLACE(call);
}

Expr* Parser::primaryExpr() {
  Token* p = peek();

  switch (p->ttype) {
    case TT_ID:
      return id();
    case TT_STRING_LITERAL:
      return stringLiteral();
    case TT_CHAR_LITERAL:
      return charLiteral();

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
      return nullptr;
  }
}

Expr* Parser::parenthesizedExpr() {
  expect(TT_LBRACKET);
  Expr* res = expr();
  expect(TT_RBRACKET);
  return res;
}

CharLiteral * Parser::charLiteral() {
  Token* t = expect(TT_CHAR_LITERAL);
  CharLiteral l;
  l.location = t->start;
  l.value = t->valueId;
  return EMPLACE(l);
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

IntLiteral* Parser::intLiteral() {
  Token* p = next();

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
      return nullptr;
  }
}

BooleanLiteral* Parser::boolLiteral() {
  Token* t = next();

  if (t->ttype != TT_KEYW_TRUE && t->ttype != TT_KEYW_FALSE) {
    FATAL(t->start, "Expected either 'true' or 'false', found '%s", tokentype_name(t->ttype));
    return nullptr;
  }

  BooleanLiteral lit;
  lit.value = t->ttype == TT_KEYW_TRUE;
  lit.location = t->start;

  return EMPLACE(lit);
}

FloatLiteral* Parser::floatLiteral() {
  const Token* t = expect(TT_FLOAT_LITERAL);

  char content[256];
  m_nameTable->getchars(t->valueId, content, 256);

  const float64 v = strtod(content, nullptr);

  FloatLiteral lit;
  lit.location = t->start;
  lit.value = v;

  return EMPLACE(lit);
}

StringLiteral* Parser::stringLiteral() {
  const Token* t = expect(TT_STRING_LITERAL);
  StringLiteral* id = m_pool->make<StringLiteral>();
  id->location = t->start;
  id->value = t->valueId;
  return id;
}

Identifier* Parser::id() {
  const Token* t = expect(TT_ID);
  Identifier* id = m_pool->make<Identifier>();
  id->location = t->start;
  id->value = t->valueId;
  return id;
}


