#ifndef QUICKSCRIPT_TRANSFORMER_H
#define QUICKSCRIPT_TRANSFORMER_H
#include "SemanticContext.h"

class SemanticTransformer {
  SemanticContext& ctx;
  ScriptFileStatement* sfs;

  Expr* optimizeExpr(Expr* expr) const;

  Expr* optimizeStringConcat(StringLiteral* lhs, Expr* rhs) const;

  Expr* optimizeStringRepeat(StringLiteral* lhs, Expr* rhs) const;

  Expr* optimizeBinary(BinaryExpr* e) const;

  Expr* optimizeUnary(UnaryExpr* u) const;

  Expr* optimizeTernary(TernaryExpr* t) const;

  Statement* optimizeStatement(Statement* stat, bool emptyBlocksAsNull);

  Identifier* makeId(std::string& string);

  Identifier* makeId(stringid id);

  void runOptimizer();

  void createStructConstructors();

  void createFileInitMethod();
  
  void flattenNestedFunctions();

  public:
    SemanticTransformer(SemanticContext& ctx, ScriptFileStatement* sfs);

    void run();
};

void runSemanticTransformer(SemanticContext& ctx, ScriptFileStatement* sfs);

#endif //QUICKSCRIPT_TRANSFORMER_H
