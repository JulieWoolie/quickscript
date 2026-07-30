#include "ast_optimizer.h"

// Target optimizations
//
// == 1. Inline constants ==
// The following code:
//   const uint32 x = 0
//   const uint32 y = x
// Becomes:
//   const uint32 y = 0
//
// == 2. Inline constant functions ==
// The following code:
//   uint32 f() { return 1 }
//   const uint32 x = f()
// Becomes:
//   const uint32 x = 1
//
// The following code:
//   uint32 f(uint32 i) { return i * 2; }
//   const uint32 x = f(2)
// Becomes:
//   const uint32 x = 4
//
//
//
//

void optimize(ScriptFileStatement* sfs) {

}
