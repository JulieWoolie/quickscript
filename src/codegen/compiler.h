#ifndef QUICKSCRIPT_COMPILER_H
#define QUICKSCRIPT_COMPILER_H

#include "../interpreter/ir_file.h"
#include "../parse/syntaxtree.h"

BytecodeFile compile(ScriptFileStatement* sfs, StringTable* table, TypeLookup* types);

#endif //QUICKSCRIPT_COMPILER_H
