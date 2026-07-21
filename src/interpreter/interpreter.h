#ifndef QUICKSCRIPT_INTERPRETER_H
#define QUICKSCRIPT_INTERPRETER_H

#include "nativeinterface.h"
#include "../common.h"

#define REGISTERS_COUNT 8
#define MAX_CALL_DEPTH 128

struct CallFrame {
  conststring name = nullptr;
  uint32 lineno = 0;
};

struct InterpreterState {
  uint64 registers[REGISTERS_COUNT]{};

  CallFrame frames[MAX_CALL_DEPTH]{};
  uint32 framesDepth = 0;

  uint32 getLine() const {
    if (framesDepth <= 0) {
      return 0;
    }
    return frames[framesDepth-1].lineno;
  }
};

struct BytecodeFunction {
  conststring name = nullptr;
  uint8* instructions = nullptr;
};

struct BytecodeFile {
  uint8* stringConstPool = nullptr;
  NativeFunction* nativeFunctions = nullptr;
  BytecodeFunction* functions = nullptr;
};

#endif //QUICKSCRIPT_INTERPRETER_H
