#ifndef QUICKSCRIPT_COMPILER_OPTS_H
#define QUICKSCRIPT_COMPILER_OPTS_H

struct CompilationOptions {
  bool exprOptimizing = true;
  bool statOptimizing = true;
  bool includeAsserts = true;
};

#endif //QUICKSCRIPT_COMPILER_OPTS_H
