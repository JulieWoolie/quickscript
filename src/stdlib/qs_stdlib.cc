#include "qs_stdlib.h"

#include "../strings/utf8.h"

static void qs_println(NativeCall& call) {
  const QsArray str = call.getArrayArgument(0);

  const uint32 len = str.length;
  const uint32 bufLen = str.length + 1;

  int8 buf[bufLen];

  memcpy(buf, str.data, len);
  buf[len] = '\0';

  fprintf(stdout, "%s", buf);
}

void addStandardLibrary(BindingsObject* obj) {
  obj->addFunctionBinding("println", "(string)=>void", qs_println);
}
