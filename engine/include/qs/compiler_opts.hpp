#ifndef QS_COMPILER_OPTS_H
#define QS_COMPILER_OPTS_H

#include <vector>
#include <string>

struct CompilationOptions {
  bool exprOptimizing = true;
  bool statOptimizing = true;
  bool includeAsserts = true;
  std::vector<std::string> implicitImports = {"std"};
};

#endif //QS_COMPILER_OPTS_H
