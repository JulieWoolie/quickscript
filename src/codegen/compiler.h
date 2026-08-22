#ifndef QUICKSCRIPT_COMPILER_H
#define QUICKSCRIPT_COMPILER_H

#include "../analysis/SemanticContext.h"
#include "../interpreter/ir_file.h"

BytecodeFile& compile(SemanticContext& ctx);

#endif //QUICKSCRIPT_COMPILER_H
