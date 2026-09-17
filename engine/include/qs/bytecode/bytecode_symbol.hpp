#ifndef QS_BYTECODE_SYMBOL_HPP
#define QS_BYTECODE_SYMBOL_HPP

#include "qs/common.hpp"
#include "qs/types/types.hpp"

#define BFSYM_FUNC 0
#define BFSYM_STRUCT 1
#define BFSYM_VARIABLE 2
typedef uint8 bfsymtype;

struct BytecodeSymbol {
  const bfsymtype type;

  union {
    uint32 funcTableIndex;
    uint32 typeTableIndex;

    struct {
      uint64 memOffset;
      uint64 nameOffset;
      typeindex typeIndex;
    } variable;
  };
};

#endif //QS_BYTECODE_SYMBOL_HPP
