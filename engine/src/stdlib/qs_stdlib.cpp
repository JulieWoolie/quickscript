#include "qs/stdlib/qs_stdlib.hpp"

#include "qs/strings/utf8.hpp"
#include "qs/types/ConstTypes.hpp"

#define QS_STR_SPEC "%.*s"
#define PRINT_QS_STR(str) str.length, reinterpret_cast<conststring>(str.data)

static void qs_println(NativeCall& call) {
  const QsArray str = call.getArrayArgument(0);
  fprintf(stdout, QS_STR_SPEC "\n", PRINT_QS_STR(str));
}

void addStandardLibrary(BindingsObject* obj) {
  obj->addFunctionBinding(
    "println",
    FunctionSignature::make(nullptr, 1, ConstTypes::STRING()),
    qs_println
  );
}
