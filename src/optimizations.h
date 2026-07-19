#ifndef QUICKSCRIPT_OPTIMIZATIONS_H
#define QUICKSCRIPT_OPTIMIZATIONS_H

#include "parse/syntaxtree.h"

Expr* optimizeBinaryOpIfPossible(BinaryExpr* e, NoFreeAllocator* alloc);

Expr* optimizeUnaryOpIfPossible(const UnaryExpr* u);

Expr* recursivelyOptimize(Expr* e, NoFreeAllocator* allocator);

#endif //QUICKSCRIPT_OPTIMIZATIONS_H
