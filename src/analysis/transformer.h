#ifndef QUICKSCRIPT_TRANSFORMER_H
#define QUICKSCRIPT_TRANSFORMER_H
#include "SemanticContext.h"

class SemanticTransformer {
  SemanticContext& ctx;
  ScriptFileStatement* sfs;

  Expr* transformExpr(Expr* expr) const;

  Expr* optimizeStringConcat(StringLiteral* lhs, Expr* rhs) const;

  Expr* optimizeStringRepeat(StringLiteral* lhs, Expr* rhs) const;

  Expr* transformBinary(BinaryExpr* e) const;

  Expr* transformUnary(UnaryExpr* u) const;

  Expr* transformTernary(TernaryExpr* t) const;

  Expr* transformCallExpr(CallExpr* expr) const;

  Statement* transformStatement(Statement* stat, bool emptyBlocksAsNull);

  Identifier* makeId(const std::string& string) const;

  Identifier* makeId(conststring string) const;

  Identifier* makeId(stringid id) const;

  void removeZeroValues();

  //
  // 1. Drop useless statements
  // 2. Inline as many expressions and function
  //    calls and constant accesses as possible
  //
  void firstPassTransformations();

  void createPropertyAssignStatements(StructDecl* decl, LocalVarSymbol* thisSym, Block* funcBlock) const;

  //
  // Create default constructors for each struct
  // type.
  //
  void createStructConstructors();

  //
  // Create a function that sets the value of
  // every global variable and calls the
  // script's main method.
  //
  void createFileInitMethod();

  //
  // Move every nested function to global space
  // and handle scope and symbol changes.
  //
  void flattenNestedFunctions();

  //
  // Flatten scopes
  //
  void processScopes() const;

  public:
    SemanticTransformer(SemanticContext& ctx, ScriptFileStatement* sfs);

    void run();
};

void runSemanticTransformer(SemanticContext& ctx, ScriptFileStatement* sfs);

#endif //QUICKSCRIPT_TRANSFORMER_H
