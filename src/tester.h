#ifndef QUICKSCRIPT_TESTER_H
#define QUICKSCRIPT_TESTER_H

#include "common.h"

struct TesterSettings {
  conststring directory = nullptr;
  bool printAsts = false;
};

void runTests(TesterSettings settings);

#endif //QUICKSCRIPT_TESTER_H
