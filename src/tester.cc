#include "tester.h"

#include <charconv>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <vector>

#include "allocator.h"
#include "errors.h"
#include "analysis/analyzer.h"
#include "analysis/transformer.h"
#include "codegen/compiler.h"
#include "interpreter/interpreter.h"
#include "interpreter/ir_file.h"
#include "parse/JsonPrinter.h"
#include "parse/lexer.h"
#include "parse/parser.h"
#include "parse/print-visitor.h"
#include "parse/token.h"

#define PRINT_FULL_ERRORS

#define TDIR_INVALID 0
#define TDIR_EXPECT_ERROR 1
#define TDIR_MODE 2
#define TDIR_EXPECT_AST 3
#define TDIR_DISABLE_STAT_INLINING 4
#define TDIR_DISABLE_EXPR_INLINING 5
#define TDIR_DROP_ASSERTS 6
#define TDIR_PROG_ARGUMENT 7
#define TDIR_BREAKPOINT 8
typedef uint8 TestDirective;

struct TestDirectiveDef {
  TestDirective directive;
  conststring value;
};

struct RunModeDef {
  testmode mode;
  conststring value;
};

static const TestDirectiveDef DIRECTIVES[] = {
  {.directive = TDIR_EXPECT_ERROR, .value = "ERROR"},
  {.directive = TDIR_MODE, .value = "MODE"},
  {.directive = TDIR_EXPECT_AST, .value = "EXPECT-AST"},
  {.directive = TDIR_DISABLE_STAT_INLINING, .value = "DISABLE-STAT-INLINING"},
  {.directive = TDIR_DISABLE_EXPR_INLINING, .value = "DISABLE-EXPR-INLINING"},
  {.directive = TDIR_DROP_ASSERTS, .value = "DROP-ASSERTS"},
  {.directive = TDIR_PROG_ARGUMENT, .value = "PROGRAM-ARG"},
  {.directive = TDIR_BREAKPOINT, .value = "BREAKPOINT"},
};

static const RunModeDef RUNMODES[] = {
  {.mode = TESTMODE_EXPR, .value = "expr"},
  {.mode = TESTMODE_RUN, .value = "run"},
  {.mode = TESTMODE_VALIDATE, .value = "validate"},
};

#define DIRECTIVE_COUNT (sizeof(DIRECTIVES) / sizeof(TestDirectiveDef))
#define RUNMODE_COUNT (sizeof(RUNMODES) / sizeof(RunModeDef))

struct KeyValuePair {
  std::string_view key;
  std::string_view value;
  uint32 end = 0;
};

static uint32 findTokenEnd(const std::string_view& view, const uint32 start) {
  if (view[start] == '"' || view[start] == '\'') {
    const int8 q = view[start];

    for (uint32 i = start + 1; i < view.length(); i++) {
      if (view[i] != q) {
        continue;
      }
      return i + 1;
    }

    return view.length();
  }

  for (uint32 i = start; i < view.length(); i++) {
    if (view[i] != ' ' && view[i] != '=') {
      continue;
    }
    return i;
  }

  return view.length();
}

static void skipWhitespace(const std::string_view& view, uint32& readIdx) {
  while (readIdx < view.length() && view[readIdx] == ' ') {
    readIdx++;
  }
}

static bool parsePair(const std::string_view& str, const uint32 off, KeyValuePair* out) {
  uint32 readIdx = off;
  skipWhitespace(str, readIdx);

  const uint32 keyStart = readIdx;
  const uint32 keyEnd = findTokenEnd(str, keyStart);

  if (keyStart == keyEnd) {
    return false;
  }

  readIdx = keyEnd;

  if (str[keyEnd] == '=') {
    readIdx++;
  } else {
    return false;
  }

  const uint32 valStart = readIdx;
  const uint32 valEnd = findTokenEnd(str, valStart);

  if (valStart == valEnd) {
    return false;
  }

  readIdx = valEnd;

  const uint32 keyLen = keyEnd - keyStart;
  const uint32 valLen = valEnd - valStart;

  if (str[valStart] == '"' || str[valStart] == '\'') {
    out->value = std::string_view(str.data() + valStart + 1, valLen - 2);
  } else {
    out->value = std::string_view(str.data() + valStart, valLen);
  }

  if (str[keyStart] == '"' || str[keyStart] == '\'') {
    out->key = std::string_view(str.data() + keyLen + 1, keyLen - 2);
  } else {
    out->key = std::string_view(str.data() + keyStart, keyLen);
  }

  out->end = readIdx;
  return true;
}

static bool startsWith(const std::string_view& view, const uint32 off, const conststring prefix) {
  const uint32 remLen = view.length() - off;
  const uint32 prefixLen = strlen(prefix);

  if (remLen < prefixLen) {
    return false;
  }

  const conststring start = view.data() + off;

  for (uint32 i = 0; i < prefixLen; i++) {
    if (start[i] == prefix[i]) {
      continue;
    }
    return false;
  }

  return true;
}

static int32 parseViewToInt(const std::string_view& sv) {
  const uint32 len = sv.length();

  char buf[len + 1];
  memcpy(buf, sv.data(), len);
  buf[len] = '\0';

  return atoi(buf);
}

static void parseExpectedAst(
  TestCase& out,
  const std::string_view& view,
  const uint32 start
) {
  const uint32 len = view.length();
  const conststring data = view.data();

  std::string& jsonString = out.expectedAst;
  uint32 readIdx = start;
  bool inString = false;

  while (readIdx < len) {
    const char ch = data[readIdx++];

    if (!inString && (ch == ' ' || ch == '\n' || ch == '\r')) {
      continue;
    }

    jsonString.push_back(ch);

    if (ch == '\"') {
      inString = !inString;
    }
  }
}

static testmode parseMode(const std::string_view& view, const uint32 readIdx) {
  for (uint32 i = 0; i < RUNMODE_COUNT; i++) {
    const RunModeDef def = RUNMODES[i];
    if (startsWith(view, readIdx, def.value)) {
      return def.mode;
    }
  }
  return TDIR_INVALID;
}

static TestDirective parseDirectivePrefix(const std::string_view& view, uint32& readIdx) {
  for (uint32 i = 0; i < DIRECTIVE_COUNT; i++) {
    TestDirectiveDef def = DIRECTIVES[i];
    if (startsWith(view, readIdx, def.value)) {
      readIdx += strlen(def.value);
      return def.directive;
    }
  }
  return TDIR_INVALID;
}

static void skipValueDelimiter(const std::string_view& view, uint32& cursor) {
  skipWhitespace(view, cursor);
  if (cursor < view.length() && (view[cursor] == ':' || view[cursor] == '=')) {
    cursor++;
    skipWhitespace(view, cursor);
  }
}

static bool parseBool(const std::string_view& view, uint32& cursor, const bool fallback) {
  skipValueDelimiter(view, cursor);

  if (startsWith(view, cursor, "true")) {
    return true;
  }
  if (startsWith(view, cursor, "false")) {
    return false;
  }

  return fallback;
}

static bool parseTestCommand(
  TestCase& out,
  const std::string_view& view,
  ExpectedError& err,
  const Token* t
) {
  uint32 readIdx = 0;
  skipWhitespace(view, readIdx);

  const TestDirective directive = parseDirectivePrefix(view, readIdx);
  if (directive == TDIR_INVALID) {
    return false;
  }

  switch (directive) {
    case TDIR_BREAKPOINT:
      out.breakpoint = true;
      return false;
    case TDIR_DISABLE_STAT_INLINING:
      out.compilerOpts.statOptimizing = !parseBool(view, readIdx, true);
      return false;
    case TDIR_DISABLE_EXPR_INLINING:
      out.compilerOpts.exprOptimizing = !parseBool(view, readIdx, true);
      return false;
    case TDIR_DROP_ASSERTS:
      out.compilerOpts.includeAsserts = !parseBool(view, readIdx, true);
      return false;
    case TDIR_MODE: {
      skipValueDelimiter(view, readIdx);
      const testmode mode = parseMode(view, readIdx);
      if (mode != TESTMODE_INVALID) {
        out.mode = mode;
      }
      return false;
    }
    case TDIR_EXPECT_AST: {
      skipValueDelimiter(view, readIdx);
      parseExpectedAst(out, view, readIdx);
      return false;
    }
    default:
      break;
  }

  if (directive != TDIR_EXPECT_ERROR) {
    return false;
  }

  KeyValuePair p;
  while (parsePair(view, readIdx, &p)) {
    if (p.key == "name") {
      err.name = std::string(p.value);
    } else if (p.key == "message") {
      err.message = std::string(p.value);
    } else if (p.key == "line") {
      if (p.value == "next") {
        err.line = t->start.line + 1;
      } else {
        err.line = parseViewToInt(p.value);
      }
    }
    readIdx = p.end;
  }

  return true;
}

void parseTestCase(TestCase& out, TokenList& list, StringTable& table) {
  const uint32 size = list.size();

  for (uint32 idx = 0; idx < size; idx++) {
    const Token* t = list.get(idx);

    if (t->ttype != TT_LCOMMENT && t->ttype != TT_BCOMMENT) {
      continue;
    }

    std::string_view content = table.getview(t->valueId);
    if (content.empty()) {
      continue;
    }

    ExpectedError err;
    err.line = t->start.line + 1;

    bool errorParsed = parseTestCommand(out, content, err, t);
    if (!errorParsed) {
      continue;
    }

    out.expectedErrors.push_back(err);
  }
}

static bool checkError(const ReportedError* rep, const ExpectedError& expect) {
  if (rep->message != expect.message) {
    fprintf(stderr,
      "[CONDITION FAIL]\n  Expected error message: %s\n  Actual: %s\n",
      expect.message.c_str(),
      rep->message.c_str()
    );
    return false;
  }

  if (expect.line == -1 || expect.line == rep->location.line) {
    return true;
  }

  fprintf(stderr,
    "[CONDITION FAIL]\n  Expected error on line %i\n  Actual: %i\n",
    expect.line,
    rep->location.line
  );

  return false;
}

static bool checkErrors(TestCase& tcase, CompilerErrors& compilerErrors, Node* astNode) {
  const uint32 expected = tcase.expectedErrors.size();
  const uint32 actual = compilerErrors.getErrorCount();

  std::vector<ReportedError*> reported;
  bool comparisonsFailed = false;

  for (ReportedError& err : compilerErrors.getErrors()) {
    if (err.level != LOGL_FATAL && err.level != LOGL_ERROR) {
      continue;
    }
    reported.push_back(&err);
  }

  for (uint32 i = 0; i < std::min(expected, actual); i++) {
    const ExpectedError& err = tcase.expectedErrors.at(i);
    const ReportedError* r = reported.at(i);

    if (checkError(r, err)) {
      continue;
    }
    compilerErrors.printError(*r);
    comparisonsFailed = true;
  }

  if (expected > actual) {
    // some errors did not occur
    for (uint32 i = actual; i < expected; i++) {
      const ExpectedError& err = tcase.expectedErrors[i];
      fprintf(stderr,
        "[CONDITION FAIL] Expected error did not occur: %s\n",
        err.message.c_str()
      );
    }

    return false;
  }

  if (expected < actual) {
    // some unexpected errors occurred
    fprintf(stderr, "[CONDITION FAIL] Unexpected error(s) when running test!\n");
    for (uint32 i = expected; i < actual; i++) {
      const ReportedError* err = reported.at(i);
      compilerErrors.printError(*err);
    }

    return false;
  }

  if (!tcase.expectedAst.empty()) {
    if (!astNode) {
      fprintf(
        stderr,
        "Expected an AST to compare to, but parsing failed before AST could be created"
      );
      return false;
    }

    JsonPrinter printer = JsonPrinter();
    astNode->acceptVisit(&printer);

    const std::string& jsonAst = printer.getResult();
    const std::string& expectedAst = tcase.expectedAst;

    if (jsonAst != expectedAst) {
      fprintf(stderr, "Expected AST did not match\nEXPECTED:\n%s\nFOUND:\n%s\n",
        expectedAst.c_str(),
        jsonAst.c_str()
      );
      return false;
    }
  }

  // All good size wise
  return !comparisonsFailed;
}

static void breakpoint() {}

#define BOOL_STR(e) (e ? "true" : "false")

static void dumpTestCaseToJson(TestCase& tcase) {
  if (tcase.dumpDirectory.empty()) {
    return;
  }

  std::filesystem::create_directories(tcase.dumpDirectory);

  const std::filesystem::path caseJsonPath = tcase.dumpDirectory / "testcase.json";
  const std::string pathString = caseJsonPath.string();

  FILE* file = fopen(pathString.c_str(), "w");

  fprintf(file, "{");
  fprintf(file, "\n  \"mode\": \"");

  for (uint32 i = 0; i < RUNMODE_COUNT; i++) {
    RunModeDef def = RUNMODES[i];
    if (def.mode == tcase.mode) {
      fprintf(file, "%s", def.value);
      break;
    }
  }
  fprintf(file, "\",");

  fprintf(file, "\n  \"breakpoint\": %s,", BOOL_STR(tcase.breakpoint));
  fprintf(file, "\n  \"expected_errors\": [");

  if (tcase.expectedErrors.empty()) {
    fprintf(file, "],");
  } else {
    bool first = true;
    for (const ExpectedError& err: tcase.expectedErrors) {
      if (!first) {
        fprintf(file, ",");
      }

      fprintf(file, "\n    {");
      fprintf(file, "\n      \"line\": %d,", err.line);
      fprintf(file, "\n      \"message\": \"%s\",", err.message.c_str());
      fprintf(file, "\n      \"name\": \"%s\"", err.name.c_str());
      fprintf(file, "\n    }");

      first = false;
    }
    fprintf(file, "\n  ],");
  }

  if (!tcase.expectedAst.empty()) {
    fprintf(file, "\n  \"expected_ast\": %s,", tcase.expectedAst.c_str());
  }

  fprintf(file, "\n  \"compiler_options\": {");
  fprintf(file, "\n    \"stat_inlining\": %s,", BOOL_STR(tcase.compilerOpts.statOptimizing));
  fprintf(file, "\n    \"expr_inlining\": %s,", BOOL_STR(tcase.compilerOpts.exprOptimizing));
  fprintf(file, "\n    \"include_asserts\": %s", BOOL_STR(tcase.compilerOpts.includeAsserts));
  fprintf(file, "\n  }");
  fprintf(file, "\n}");

  fclose(file);
}

bool runTestCase(
  TestCase& tcase,
  const std::filesystem::path& filePath,
  const ProgramSettings& settings
) {
  std::ifstream instream(filePath);

  if (!instream.is_open()) {
    fprintf(stderr, "Failed to open %ls\n", filePath.c_str());
    return false;
  }

  std::string file_contents { std::istreambuf_iterator<char>(instream), std::istreambuf_iterator<char>() };

  TokenList tlist;
  StringTable table;

  const std::string pathString = filePath.string();

  conststring fileName = pathString.c_str();
  CompilerErrors errors = CompilerErrors(&file_contents, fileName);
  errors.setSilent(true);

  Lexer l = Lexer(file_contents, &tlist, &table, &errors);
  l.setCommentsIgnored(false);

  bool stepFailed = false;

  try {
    l.lex();
    parseTestCase(tcase, tlist, table);
  } catch (std::runtime_error& err) {
    parseTestCase(tcase, tlist, table);
    stepFailed = true;
  }

  dumpTestCaseToJson(tcase);

  if (tcase.breakpoint) {
    breakpoint();
  }

  if (stepFailed) {
    return checkErrors(tcase, errors, nullptr);
  }

  NoFreeAllocator allocator;
  Parser p = Parser(&tlist, &allocator, &errors, &table);
  Node* result = nullptr;

  if (tcase.mode == TESTMODE_RUN || tcase.mode == TESTMODE_VALIDATE) {
    try {
      result = p.parse();
    } catch (std::runtime_error& e) {
      stepFailed = true;
    }
  } else {
    try {
      result = p.expr();
    } catch (std::runtime_error& e) {
      stepFailed = true;
    }
  }

  if (stepFailed) {
    return checkErrors(tcase, errors, result);
  }

  if (!tcase.dumpDirectory.empty()) {
    std::filesystem::create_directories(tcase.dumpDirectory);

    // Write JSON AST
    JsonPrinter printer;
    result->acceptVisit(&printer);

    const std::string& jsonString = printer.getResult();
    std::ofstream jsonStream(tcase.dumpDirectory / "ast.json");

    jsonStream << jsonString;
  }

  TypeTable lookup = TypeTable();

  if (result->nodeKind() == AST_ScriptFileStatement) {
    SemanticContext ctx = SemanticContext(lookup, table, errors, allocator, tcase.compilerOpts);
    runSemanticAnalysis(static_cast<ScriptFileStatement*>(result), ctx);

    if (!checkErrors(tcase, errors, result)) {
      return false;
    }

    if (tcase.mode != TESTMODE_RUN) {
      return true;
    }

    runSemanticTransformer(ctx, static_cast<ScriptFileStatement*>(result));
    BytecodeFile& bfile = compile(ctx);

    if (!tcase.dumpDirectory.empty()) {
      std::filesystem::path binaryIrFile = tcase.dumpDirectory / "bytecode.qscr-bin";
      std::filesystem::path textIrFile = tcase.dumpDirectory / "bytecode.qscr-ir";

      std::string textIrPath = textIrFile.string();
      std::string binIrPath = binaryIrFile.string();

      FILE* fileHandle = nullptr;

      fileHandle = fopen(binIrPath.c_str(), "w");
      uint64 binSize = 0;
      uint8* binData = serializeBytecodeFile(bfile, &binSize);
      fwrite(binData, binSize, 1, fileHandle);
      fclose(fileHandle);
      free(binData);

      fileHandle = fopen(textIrPath.c_str(), "w");
      printBytecodeFile(bfile, fileHandle);
      fclose(fileHandle);
    }

    VirtualMachine vm = VirtualMachine();

    uint32 entryPoint = vm.addBytecodeFile(bfile);
    BytecodeFile::destroy(bfile);

    try {
      vm.beginExecution(entryPoint, settings.runArgs);
    } catch (std::runtime_error& exc) {
      fprintf(stderr, "Failed with exception: ");
      fprintf(stderr, "%s\n", exc.what());
      return false;
    }

    return true;
  }

  if (settings.printAst & PRINTAST_AFTER_TRANSFORM) {
    PrintingVisitor pv = PrintingVisitor(&table, fileName);
    result->acceptVisit(&pv);
  }

  return checkErrors(tcase, errors, result);
}

void runTests(const ProgramSettings& settings) {
  const std::filesystem::path dirpath = settings.inputFile;

  uint32 total = 0;
  uint32 failed = 0;

  try {
    for (const auto& entry: std::filesystem::recursive_directory_iterator(dirpath)) {
      if (!entry.is_regular_file()) {
        continue;
      }

      const std::filesystem::path& path = entry.path();
      TestCase testCase;

      if (!settings.testDumpDirectory.empty()) {
        std::filesystem::path dumpDir = settings.testDumpDirectory;
        dumpDir /= std::filesystem::relative(path, dirpath);
        testCase.dumpDirectory = dumpDir;
      }

      bool success = runTestCase(testCase, path, settings);

      total++;

      if (success) {
        if (!settings.omitPassedTests) {
          fprintf(stdout, "[OK] Test '%ls' passed\n", path.c_str());
        }
        continue;
      }

      failed++;
      fprintf(stderr, "[TEST FAIL] Test '%ls' failed\n\n", path.c_str());
    }
  } catch (const std::filesystem::filesystem_error& e) {
    // ignore
  }

  fprintf(stdout, "[TESTING] Ran %i tests, %i failed\n", total, failed);
}
