#ifndef QS_COMPILER_H
#define QS_COMPILER_H

#include "qs/analysis/SemanticContext.hpp"
#include "qs/bytecode/bytecode_file.hpp"

BytecodeFile& compile(SemanticContext& ctx);

#endif //QS_COMPILER_H
