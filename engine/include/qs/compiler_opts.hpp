#ifndef QS_COMPILER_OPTS_H
#define QS_COMPILER_OPTS_H

struct CompilationOptions {
  bool exprOptimizing = true;
  bool statOptimizing = true;
  bool includeAsserts = true;
  std::vector<std::string> implicitImports = {"std"};
};

#endif //QS_COMPILER_OPTS_H
