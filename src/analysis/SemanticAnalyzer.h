#ifndef QUICKSCRIPT_TYPERESOLVER_H
#define QUICKSCRIPT_TYPERESOLVER_H

#include "../parse/syntaxtree.h"
#include "semantictree.h"
#include "SemanticContext.h"

SemanticFile* runSemanticAnalysis(ScriptFileStatement* sfs, SemanticContext& ctx);

#endif //QUICKSCRIPT_TYPERESOLVER_H
