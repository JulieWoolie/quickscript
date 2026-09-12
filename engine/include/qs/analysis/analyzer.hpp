#ifndef QS_TYPERESOLVER_H
#define QS_TYPERESOLVER_H

#include "qs/parse/syntaxtree.hpp"
#include "qs/analysis/SemanticContext.hpp"

void runSemanticAnalysis(ScriptFileStatement* sfs, SemanticContext& ctx);

#endif //QS_TYPERESOLVER_H
