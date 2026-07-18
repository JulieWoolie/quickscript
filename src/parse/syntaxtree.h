#ifndef QUICKSCRIPT_SYNTAXTREE_H
#define QUICKSCRIPT_SYNTAXTREE_H

#include <vector>

#include "../common.h"
#include "../stringtable.h"
#include "token.h"
#include "../types.h"

// ========================
// ======== Visitor =======
// ========================

struct TypeNameExpr;
struct ArrayTypeExpr;
struct PrimitiveTypeExpr;
struct Identifier;
struct CallExpr;
struct PropertyAccessExpr;
struct IndexAccessExpr;
struct BooleanLiteral;
struct CharLiteral;
struct StringLiteral;
struct IntLiteral;
struct FloatLiteral;
struct BinaryExpr;
struct UnaryExpr;
struct TernaryExpr;
struct Block;
struct IfStatement;
struct ForStatement;
struct LexicalDeclaration;
struct DoWhileStatement;
struct WhileStatement;
struct ControlFlowStatement;
struct ReturnStatement;
struct ScriptFileStatement;
struct FunctionParam;
struct FunctionDeclStatement;
struct ExprStatement;
struct StructPropertyDecl;
struct StructDecl;

struct Visitor {
  virtual ~Visitor() = default;

  virtual void acceptTypeNameExpr(TypeNameExpr* v) = 0;
  virtual void acceptArrayTypeExpr(ArrayTypeExpr* v) = 0;
  virtual void acceptPrimitiveTypeExpr(PrimitiveTypeExpr* v) = 0;

  virtual void acceptIdentifier(Identifier* v) = 0;
  virtual void acceptCallExpr(CallExpr* v) = 0;
  virtual void acceptPropertyAccessExpr(PropertyAccessExpr* v) = 0;
  virtual void acceptIndexAccessExpr(IndexAccessExpr* v) = 0;
  virtual void acceptBooleanLiteral(BooleanLiteral* v) = 0;
  virtual void acceptCharLiteral(CharLiteral* v) = 0;
  virtual void acceptStringLiteral(StringLiteral* v) = 0;
  virtual void acceptIntLiteral(IntLiteral* v) = 0;
  virtual void acceptFloatLiteral(FloatLiteral* v) = 0;
  virtual void acceptBinaryExpr(BinaryExpr* v) = 0;
  virtual void acceptUnaryExpr(UnaryExpr* v) = 0;
  virtual void acceptTernaryExpr(TernaryExpr* v) = 0;

  virtual void acceptBlock(Block* v) = 0;
  virtual void acceptIfStatement(IfStatement* v) = 0;
  virtual void acceptForStatement(ForStatement* v) = 0;
  virtual void acceptLexicalDeclaration(LexicalDeclaration* v) = 0;
  virtual void acceptDoWhileStatement(DoWhileStatement* v) = 0;
  virtual void acceptWhileStatement(WhileStatement* v) = 0;
  virtual void acceptControlFlowStatement(ControlFlowStatement* v) = 0;
  virtual void acceptReturnStatement(ReturnStatement* v) = 0;
  virtual void acceptScriptFileStatement(ScriptFileStatement* v) = 0;
  virtual void acceptFunctionParam(FunctionParam* v) = 0;
  virtual void acceptFunctionDeclStatement(FunctionDeclStatement* v) = 0;
  virtual void acceptExprStatement(ExprStatement* v) = 0;
  virtual void acceptStructPropertyDecl(StructPropertyDecl* v) = 0;
  virtual void acceptStructDecl(StructDecl* v) = 0;
};

// ================================
// ======== Node super type =======
// ================================

struct Node {
  Location location;

  virtual ~Node() = default;

  virtual conststring nodeType() = 0;
  virtual void acceptVisit(Visitor* v) = 0;
};

#define AST_TYPE(name, supertype, body) struct name: supertype {\
  body\
  conststring nodeType() override {return #name;}\
  void acceptVisit(Visitor* v) override { v->accept##name(this); }\
};

#define AST_EXPR_TYPE(name, supertype, body) struct name: supertype {\
  body\
  ScriptType* resultType = nullptr;\
  ScriptType* getResultingType() override { return resultType; }\
  void setResultingType(ScriptType* t) override { resultType = t; }\
  conststring nodeType() override {return #name;}\
  void acceptVisit(Visitor* v) override { v->accept##name(this); }\
};

// ==============================
// ====== Type Expressions ======
// ==============================

struct TypeExpr: Node {
  virtual ScriptType* getReferencedType() = 0;
};

#define PPT_NIL      0
#define PPT_BOOL     1
#define PPT_UINT8    2
#define PPT_INT8     3
#define PPT_UINT16   4
#define PPT_INT16    5
#define PPT_UINT32   6
#define PPT_INT32    7
#define PPT_UINT64   8
#define PPT_INT64    9
#define PPT_FLOAT32  10
#define PPT_FLOAT64  11
#define PPT_STRING   12
#define PPT_VOID     13

typedef uint8 parsedprimitivetype;

conststring parsedprimitivetype_name(parsedprimitivetype pt);

AST_TYPE(PrimitiveTypeExpr, TypeExpr,
  parsedprimitivetype primType = PPT_NIL;
  ScriptType* referencedType = nullptr;

  ScriptType* getReferencedType() override {
    return referencedType;
  }
)

AST_TYPE(TypeNameExpr, TypeExpr,
  stringid typeName = EMPTY_STRING;
  ScriptType* referencedType = nullptr;

  ScriptType* getReferencedType() override {
    return referencedType;
  }
)

AST_TYPE(ArrayTypeExpr, TypeExpr,
  TypeExpr* componentType = nullptr;
  ScriptType* referencedType = nullptr;

  ScriptType* getReferencedType() override {
    return referencedType;
  }
)

// ========================
// ===== Expressions ======
// ========================

struct Expr: Node {
  virtual ScriptType* getResultingType() = 0;
  virtual void setResultingType(ScriptType* t) = 0;
};

AST_EXPR_TYPE(Identifier, Expr,
  stringid value = EMPTY_STRING;
)

AST_EXPR_TYPE(CallExpr, Expr,
  Expr* target = nullptr;
  std::vector<Expr*> arguments;
)

AST_EXPR_TYPE(PropertyAccessExpr, Expr,
  Identifier* property = nullptr;
  Expr* target = nullptr;
)

AST_EXPR_TYPE(IndexAccessExpr, Expr,
  Expr* index = nullptr;
  Expr* target = nullptr;
)

AST_EXPR_TYPE(BooleanLiteral, Expr,
  bool value = false;
)

AST_EXPR_TYPE(CharLiteral, Expr,
  stringid value = EMPTY_STRING;
)

AST_EXPR_TYPE(StringLiteral, Expr,
  stringid value = EMPTY_STRING;
)

AST_EXPR_TYPE(IntLiteral, Expr,
  int64 value = 0;
)

AST_EXPR_TYPE(FloatLiteral, Expr,
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

conststring binaryop_name(binaryop op);

// ---

AST_EXPR_TYPE(BinaryExpr, Expr,
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

conststring unaryop_name(unaryop op);

AST_EXPR_TYPE(UnaryExpr, Expr,
  Expr* target = nullptr;
  unaryop op = UOP_NIL;
)

AST_EXPR_TYPE(TernaryExpr, Expr,
  Expr* condition = nullptr;
  Expr* left = nullptr;
  Expr* right = nullptr;
)

// ========================
// ====== Statements ======
// ========================

struct Statement: Node {

};

AST_TYPE(Block, Statement,
  std::vector<Statement*> statements;
)

AST_TYPE(IfStatement, Statement,
  Expr* condition = nullptr;
  Statement* body = nullptr;
  Statement* elseBody = nullptr;
)

AST_TYPE(ForStatement, Statement,
  LexicalDeclaration* first = nullptr;
  Expr* second = nullptr;
  Expr* third = nullptr;
  Statement* loopBody = nullptr;
  Identifier* label = nullptr;
)

AST_TYPE(LexicalDeclaration, Statement,
  TypeExpr* typeExpr = nullptr;
  Identifier* variableName = nullptr;
  Expr* value = nullptr;
  bool isConstDeclaration = false;
)

AST_TYPE(DoWhileStatement, Statement,
  Block* body = nullptr;
  Expr* condition = nullptr;
  Identifier* label = nullptr;
)

AST_TYPE(WhileStatement, Statement,
  Block* body = nullptr;
  Expr* condition = nullptr;
  Identifier* label = nullptr;
)

#define CFT_NIL 0
#define CFT_CONTINUE 1
#define CFT_BREAK 2

typedef uint8 controlflowtype;

conststring controlflowtype_name(controlflowtype cft);

AST_TYPE(ControlFlowStatement, Statement,
  Identifier* label = nullptr;
  controlflowtype type = CFT_CONTINUE;
)

AST_TYPE(ReturnStatement, Statement,
  Expr* value = nullptr;
)

AST_TYPE(ScriptFileStatement, Block,

)

AST_TYPE(FunctionParam, Statement,
  TypeExpr* paramType = nullptr;
  Identifier* name = nullptr;
)

AST_TYPE(FunctionDeclStatement, Statement,
  Identifier* name = nullptr;
  std::vector<FunctionParam*> arguments;
  Block* functionBody = nullptr;
  TypeExpr* returnType = nullptr;
)

AST_TYPE(StructPropertyDecl, Statement,
  TypeExpr* propertyType = nullptr;
  Identifier* name = nullptr;
  Expr* value = nullptr;
  StructDecl* structDeclStatement = nullptr;
)

AST_TYPE(StructDecl, Statement,
  Identifier* name = nullptr;
  std::vector<StructPropertyDecl*> properties;
  ScriptStructType* type = nullptr;
)

AST_TYPE(ExprStatement, Statement,
  Expr* expression = nullptr;
)

#endif //QUICKSCRIPT_SYNTAXTREE_H