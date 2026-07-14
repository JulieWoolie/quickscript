#ifndef QUICKSCRIPT_SYNTAXTREE_H
#define QUICKSCRIPT_SYNTAXTREE_H

#include <vector>

#include "common.h"
#include "stringtable.h"
#include "token.h"

template<typename T>
using NodeRef = uint64;

typedef uint64 nodeid;

#define UNSET_REF 0xffffffffU

struct Node {
  Location location;
};

#define AST_TYPE(name, supertype, body) struct name: supertype {body};

// ========================
// ===== Expressions ======
// ========================

struct Expr: Node {

};

AST_TYPE(Identifier, Expr,
  stringid value = EMPTY_STRING;
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

// --- Binary Operations ---

#define BOP_ASSIGN_FLAG  0b100000
#define BOP_NIL          0b000000

#define BOP_GT           0b000001
#define BOP_LT           0b000010
#define BOP_GTE          0b000011
#define BOP_LTE          0b000100
#define BOP_EQ           0b000101
#define BOP_NEQ          0b000110
#define BOP_ADD          0b000111
#define BOP_SUB          0b001000
#define BOP_MUL          0b001001
#define BOP_DIV          0b001010
#define BOP_MOD          0b001011
#define BOP_EXP          0b001100
#define BOP_SHL          0b001101
#define BOP_SHR          0b001110
#define BOP_USHR         0b001111
#define BOP_XOR          0b010000
#define BOP_OR           0b010001
#define BOP_AND          0b010010

#define BOP_ASSIGN_ADD   (BOP_ADD | BOP_ASSIGN_FLAG)
#define BOP_ASSIGN_SUB   (BOP_SUB | BOP_ASSIGN_FLAG)
#define BOP_ASSIGN_MUL   (BOP_MUL | BOP_ASSIGN_FLAG)
#define BOP_ASSIGN_DIV   (BOP_DIV | BOP_ASSIGN_FLAG)
#define BOP_ASSIGN_MOD   (BOP_MOD | BOP_ASSIGN_FLAG)
#define BOP_ASSIGN_EXP   (BOP_EXP | BOP_ASSIGN_FLAG)
#define BOP_ASSIGN_SHL   (BOP_SHL | BOP_ASSIGN_FLAG)
#define BOP_ASSIGN_SHR   (BOP_SHR | BOP_ASSIGN_FLAG)
#define BOP_ASSIGN_USHR  (BOP_USHR | BOP_ASSIGN_FLAG)
#define BOP_ASSIGN_XOR   (BOP_XOR | BOP_ASSIGN_FLAG)
#define BOP_ASSIGN_OR    (BOP_OR | BOP_ASSIGN_FLAG)
#define BOP_ASSIGN_AND   (BOP_AND | BOP_ASSIGN_FLAG)

typedef uint8 binaryop;

// ---

AST_TYPE(BinaryExpr, Expr,
  NodeRef<Expr> lhs = UNSET_REF;
  NodeRef<Expr> rhs = UNSET_REF;
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
  NodeRef<Expr> target = UNSET_REF;
  unaryop op = UOP_NIL;
)

// ========================
// ====== Statements ======
// ========================

struct Statement: Node {

};

AST_TYPE(IfStatement, Statement,
  NodeRef<Expr> condition = UNSET_REF;
  NodeRef<Statement> body = UNSET_REF;
  NodeRef<Statement> elseBody = UNSET_REF;
)

#define CFT_CONTINUE 0
#define CFT_BREAK 1

typedef uint8 controlflowtype;

AST_TYPE(ControlFlowStatement, Statement,
  NodeRef<Identifier> label = UNSET_REF;
  controlflowtype type = CFT_CONTINUE;
)

AST_TYPE(ReturnStatement, Statement,
  NodeRef<Expr> value = UNSET_REF;
)

AST_TYPE(Block, Statement,
  std::vector<nodeid> statements;
)

AST_TYPE(ScriptFileStatement, Block,

)

AST_TYPE(FunctionDeclStatement, Statement,
  NodeRef<Identifier> name = UNSET_REF;
  NodeRef<Block> functionBody = UNSET_REF;
)

#endif //QUICKSCRIPT_SYNTAXTREE_H

// === Pool ===

class NodePool {
  private:
    uint8* m_data = nullptr;
    uint64 capacity = 0;
    uint64 cursor = 0;

  public:
    template<typename T>
    NodeRef<T> emplace(T node);

    template<typename T>
    T* get(NodeRef<T> ref);

  private:
    uint64 allocspace(uint64 sz);
};

template<typename T>
NodeRef<T> NodePool::emplace(T node) {
  uint64 sz = sizeof(T);
  uint64 idx = allocspace(sz);

  uint8* dstart = m_data + idx;
  T* typedPtr = (T*) dstart;

  *typedPtr = node;

  return idx;
}

template<typename T>
T* NodePool::get(NodeRef<T> ref) {
  if (ref >= cursor) {
    return nullptr;
  }

  uint8* dstart = m_data + ref;
  return (T*) dstart;
}

uint64 NodePool::allocspace(const uint64 sz) {
  uint64 nsize = cursor + sz;

  if (nsize > capacity) {
    uint64 ncap = capacity + 1024;
    uint8* ndata = static_cast<uint8 *>(realloc(m_data, ncap));

    if (!ndata) {
      throw std::runtime_error("Error expanding node pool");
    }

    m_data = ndata;
    capacity = ncap;
  }

  uint64 ref = cursor;
  cursor += sz;

  return ref;
}
