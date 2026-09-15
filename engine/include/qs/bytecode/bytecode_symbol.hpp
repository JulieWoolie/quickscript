#ifndef QS_BYTECODE_SYMBOL_HPP
#define QS_BYTECODE_SYMBOL_HPP

#include "qs/common.hpp"

#define BFSYM_FUNC 0
#define BFSYM_STRUCT 1
#define BFSYM_VARIABLE 2
typedef uint8 bfsymtype;

struct BytecodeSymbol {
  const bfsymtype type;

  explicit BytecodeSymbol(bfsymtype symtype);
};

struct BytecodeFuncSymbol: BytecodeSymbol {
  uint32 funcTableIndex = -1;

  BytecodeFuncSymbol();

  static BytecodeFuncSymbol* create();
  static void destroy(const BytecodeFuncSymbol* sym);
};

struct BytecodeTypeSymbol: BytecodeSymbol {
  uint32 funcTableIndex = -1;

  BytecodeTypeSymbol();

  static BytecodeTypeSymbol* create();
  static void destroy(const BytecodeTypeSymbol* sym);
};

struct BytecodeVarSymbol: BytecodeSymbol {
  uint64 memOffset = 0;
  uint64 nameOffset = 0;
  uint32 typeIndex = 0;

  BytecodeVarSymbol();

  static BytecodeVarSymbol* create();
  static void destroy(const BytecodeVarSymbol* sym);
};

#endif //QS_BYTECODE_SYMBOL_HPP
