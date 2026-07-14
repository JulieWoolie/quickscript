#ifndef QUICKSCRIPT_SYNTAXTREE_H
#define QUICKSCRIPT_SYNTAXTREE_H

#include <vector>

#include "common.h"
#include "stringtable.h"
#include "token.h"

struct Node {
  Location location;

  virtual ~Node() = default;
  virtual conststring nodeType() = 0;
};

#define AST_TYPE(name, supertype, body) struct name: supertype {\
  body\
  conststring nodeType() override {return #name;}\
};

// ========================
// ===== Expressions ======
// ========================

struct Expr: Node {

};

AST_TYPE(Identifier, Expr,
  stringid value = EMPTY_STRING;
)

AST_TYPE(CallExpr, Expr,
  Expr* target = nullptr;
  std::vector<Expr*> arguments;
)

AST_TYPE(PropertyAccessExpr, Expr,
  Identifier* property = nullptr;
  Expr* target = nullptr;
)

AST_TYPE(BooleanLiteral, Expr,
  bool value = false;
)

AST_TYPE(CharLiteral, Expr,
  uint16 value = 0;
)

AST_TYPE(StringLiteral, Expr,
  stringid value = EMPTY_STRING;
)

AST_TYPE(IntLiteral, Expr,
  int64 value = 0;
)

AST_TYPE(FloatLiteral, Expr,
  float64 value = 0.0;
)

// --- Binary Operations ---

#define BOP_ASSIGN_FLAG     0b100000
#define BOP_NIL             0b000000

#define BOP_GT              0b000001
#define BOP_LT              0b000010
#define BOP_GTE             0b000011
#define BOP_LTE             0b000100
#define BOP_EQ              0b000101
#define BOP_NEQ             0b000110
#define BOP_ADD             0b000111
#define BOP_SUB             0b001000
#define BOP_MUL             0b001001
#define BOP_DIV             0b001010
#define BOP_MOD             0b001011
#define BOP_EXP             0b001100
#define BOP_SHL             0b001101
#define BOP_SHR             0b001110
#define BOP_USHR            0b001111
#define BOP_XOR             0b010000
#define BOP_LOG_OR          0b010001
#define BOP_LOG_AND         0b010010
#define BOP_BIT_OR          0b010011
#define BOP_BIT_AND         0b010100
#define BOP_POW             0b010101

#define BOP_ASSIGN_ADD      (BOP_ADD | BOP_ASSIGN_FLAG)
#define BOP_ASSIGN_SUB      (BOP_SUB | BOP_ASSIGN_FLAG)
#define BOP_ASSIGN_MUL      (BOP_MUL | BOP_ASSIGN_FLAG)
#define BOP_ASSIGN_DIV      (BOP_DIV | BOP_ASSIGN_FLAG)
#define BOP_ASSIGN_MOD      (BOP_MOD | BOP_ASSIGN_FLAG)
#define BOP_ASSIGN_EXP      (BOP_EXP | BOP_ASSIGN_FLAG)
#define BOP_ASSIGN_SHL      (BOP_SHL | BOP_ASSIGN_FLAG)
#define BOP_ASSIGN_SHR      (BOP_SHR | BOP_ASSIGN_FLAG)
#define BOP_ASSIGN_USHR     (BOP_USHR | BOP_ASSIGN_FLAG)
#define BOP_ASSIGN_XOR      (BOP_XOR | BOP_ASSIGN_FLAG)
#define BOP_ASSIGN_LOG_OR   (BOP_LOG_OR | BOP_ASSIGN_FLAG)
#define BOP_ASSIGN_BIT_OR   (BOP_BIT_OR | BOP_ASSIGN_FLAG)
#define BOP_ASSIGN_LOG_AND  (BOP_LOG_AND | BOP_ASSIGN_FLAG)
#define BOP_ASSIGN_BIT_AND  (BOP_BIT_AND | BOP_ASSIGN_FLAG)

typedef uint8 binaryop;

// ---

AST_TYPE(BinaryExpr, Expr,
  Expr* lhs = nullptr;
  Expr* rhs = nullptr;
  binaryop op = BOP_NIL;
)

#define UOP_NIL       0
#define UOP_PREINC    1
#define UOP_POSTINC   2
#define UOP_PREDEC    3
#define UOP_POSTDEC   4
#define UOP_POS       5
#define UOP_NEG       6
#define UOP_BIT_NOT   7
#define UOP_LOG_NOT   8

typedef uint8 unaryop;

AST_TYPE(UnaryExpr, Expr,
  Expr* target = nullptr;
  unaryop op = UOP_NIL;
)

AST_TYPE(TernaryExpr, Expr,
  Expr* condition = nullptr;
  Expr* left = nullptr;
  Expr* right = nullptr;
)

// ========================
// ====== Statements ======
// ========================

struct Statement: Node {

};

AST_TYPE(IfStatement, Statement,
  Expr* condition = nullptr;
  Statement* body = nullptr;
  Statement* elseBody = nullptr;
)

#define CFT_CONTINUE 0
#define CFT_BREAK 1

typedef uint8 controlflowtype;

AST_TYPE(ControlFlowStatement, Statement,
  Identifier* label = nullptr;
  controlflowtype type = CFT_CONTINUE;
)

AST_TYPE(ReturnStatement, Statement,
  Expr* value = nullptr;
)

AST_TYPE(Block, Statement,
  std::vector<Statement*> statements;
)

AST_TYPE(ScriptFileStatement, Block,

)

AST_TYPE(FunctionDeclStatement, Statement,
  Identifier* name = nullptr;
  Block* functionBody = nullptr;
)

#endif //QUICKSCRIPT_SYNTAXTREE_H