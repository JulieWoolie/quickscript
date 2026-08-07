#ifndef QUICKSCRIPT_TESTER_H
#define QUICKSCRIPT_TESTER_H

#include <filesystem>
#include <vector>

#include "args.h"
#include "common.h"
#include "parse/token.h"

#define TESTMODE_RUN 0
#define TESTMODE_EXPR 1
#define TESTMODE_VALIDATE 2
typedef uint8 testmode;

struct ExpectedError {
  int32 line = -1;
  std::string message;
  std::string name;
};

struct TestCase {
  testmode mode = TESTMODE_RUN;
  std::vector<ExpectedError> expectedErrors;
};

void parseTestCase(TestCase& out, TokenList& list, StringTable& table);

bool runTestCase(const std::filesystem::path& filePath, const ProgramSettings& settings);

void runTests(const ProgramSettings& settings);

#endif //QUICKSCRIPT_TESTER_H
