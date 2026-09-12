#ifndef QS_COMPILER_OPTS_H
#define QS_COMPILER_OPTS_H

struct CompilationOptions {
  bool exprOptimizing = true;
  bool statOptimizing = true;
  bool includeAsserts = true;
};

#endif //QS_COMPILER_OPTS_H
