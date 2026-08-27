#include "qs_stdlib.h"

#include "../strings/utf8.h"

#define QS_STR_SPEC "%.*s"
#define PRINT_QS_STR(str) str.length, reinterpret_cast<conststring>(str.data)

static void qs_println(NativeCall& call) {
  const QsArray str = call.getArrayArgument(0);
  fprintf(stdout, QS_STR_SPEC, PRINT_QS_STR(str));
}

void addStandardLibrary(BindingsObject* obj) {
  obj->addFunctionBinding("println", "(string)=>void", qs_println);
}
