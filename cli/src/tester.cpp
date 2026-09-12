#include "tester.hpp"

#include <filesystem>
#include <fstream>
#include <vector>
#include <chrono>

#include "qs/allocator.hpp"
#include "qs/errors.hpp"
#include "qs/analysis/analyzer.hpp"
#include "qs/analysis/transformer.hpp"
#include "qs/codegen/compiler.hpp"
#include "qs/interpreter/interpreter.hpp"
#include "qs/bytecode/bytecode_file.hpp"
#include "qs/interpreter/script_error.hpp"
#include "qs/parse/JsonPrinter.hpp"
#include "qs/parse/lexer.hpp"
#include "qs/parse/parser.hpp"
#include "qs/parse/print-visitor.hpp"
#include "qs/parse/token.hpp"
#include "qs/strings/stringreader.hpp"
#include "qs/strings/strings.hpp"

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
#define TDIR_EXPECTED_RUNTIME_ERROR 9
typedef uint8 TestDirective;

#define TESTMODE_INVALID 0
#define TESTMODE_RUN 1
#define TESTMODE_EXPR 2
#define TESTMODE_VALIDATE 3
typedef uint8 testmode;

struct ExpectedError {
  int32 line = -1;
  std::string message;
  std::string name;
};

struct TestCase {
  testmode mode = TESTMODE_RUN;
  bool breakpoint = false;
  std::vector<ExpectedError> expectedErrors;
  std::string expectedAst;
  std::string expectedRuntimeError;
  std::filesystem::path dumpDirectory;
  CompilationOptions compilerOpts;
};


struct TestTiming {
  uint64 totalTime = 0;
  uint32 count = 0;

  void account(const uint64 start, const uint64 end) {
    if (end <= start) {
      return;
    }

    count++;
    totalTime += end - start;
  }

  void printTiming(const conststring name) const {
    const float64 avgNs = static_cast<float64>(totalTime) / static_cast<float64>(count);
    const float64 avgMs = avgNs / 1.0E+6;

    printf("  %s timings sampled %d times: average=%.2f ns (%.2f ms)\n", name, count, avgNs, avgMs);
  }
};

struct TestTimings {
  TestTiming lexTimings = TestTiming();
  TestTiming parseTimings = TestTiming();
  TestTiming analysisTimings = TestTiming();
  TestTiming transformTimings = TestTiming();
  TestTiming execTimings = TestTiming();

  void printTimings() {
    printf("Test Timings:\n");
    lexTimings.printTiming("Lex");
    parseTimings.printTiming("Parse");
    analysisTimings.printTiming("Semantic Analysis");
    transformTimings.printTiming("Semantic Transformation");
    execTimings.printTiming("Script Execution");
  }
};

struct TestContext {
  TestTimings timings;
  const ProgramSettings& settings;
  const BindingsObject* bindings;
};

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
  {.directive = TDIR_EXPECTED_RUNTIME_ERROR, .value = "EXPECT-RUNTIME-ERROR"},
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

static void readUntilTokenEnd(StringReader& reader, std::string_view* out) {
  if (reader.peek() == '"' || reader.peek() == '\'') {
    const utf32char q = reader.next();
    const uint32 start = reader.cursor();

    while (reader.hasNext()) {
      if (reader.peek() != q) {
        reader.next();
        continue;
      }

      *out = reader.substring(start, reader.cursor());
      reader.next();

      return;
    }

    *out = reader.substring(start, reader.cursor());
    return;
  }

  const uint32 start = reader.cursor();

  while (reader.hasNext()) {
    const utf32char p = reader.peek();

    if (isWhitespace(p) || p == '=') {
      break;
    }

    reader.next();
  }

  *out = reader.substring(start, reader.cursor());
}

static bool parsePair(StringReader& reader, KeyValuePair* out) {
  reader.skipWhitespace();

  readUntilTokenEnd(reader, &out->key);
  reader.skipWhitespace();

  if (out->key.length() == 0) {
    return false;
  }

  if (reader.peek() == '=') {
    reader.next();
    reader.skipWhitespace();
  } else {
    return false;
  }

  readUntilTokenEnd(reader, &out->value);
  return out->value.length() != 0;
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
  StringReader& reader
) {
  std::string& jsonString = out.expectedAst;
  bool inString = false;

  while (reader.hasNext()) {
    const char ch = reader.next();

    if (!inString && (ch == ' ' || ch == '\n' || ch == '\r')) {
      continue;
    }

    jsonString.push_back(ch);

    if (ch == '\"') {
      inString = !inString;
    }
  }
}

static testmode parseMode(StringReader& reader) {
  for (uint32 i = 0; i < RUNMODE_COUNT; i++) {
    const RunModeDef def = RUNMODES[i];

    if (reader.consumeIfMatches(def.value)) {
      return def.mode;
    }
  }

  return TDIR_INVALID;
}

static TestDirective parseDirectivePrefix(StringReader& reader) {
  for (uint32 i = 0; i < DIRECTIVE_COUNT; i++) {
    TestDirectiveDef def = DIRECTIVES[i];

    if (!reader.consumeIfMatches(def.value)) {
      continue;
    }

    return def.directive;
  }
  return TDIR_INVALID;
}

static void skipValueDelimiter(StringReader& reader) {
  reader.skipWhitespace();
  if (reader.peek() == ':' || reader.peek() == '=') {
    reader.next();
    reader.skipWhitespace();
  }
}

static void parseTestCommand(TestCase& out, StringReader& reader, const Token* t) {
  reader.skipWhitespace();

  const TestDirective directive = parseDirectivePrefix(reader);
  if (directive == TDIR_INVALID) {
    return;
  }

  switch (directive) {
    case TDIR_BREAKPOINT:
      out.breakpoint = true;
      return;
    case TDIR_DISABLE_STAT_INLINING:
      skipValueDelimiter(reader);
      out.compilerOpts.statOptimizing = !reader.parseBool(true);
      return;
    case TDIR_DISABLE_EXPR_INLINING:
      skipValueDelimiter(reader);
      out.compilerOpts.exprOptimizing = !reader.parseBool(true);
      return;
    case TDIR_DROP_ASSERTS:
      skipValueDelimiter(reader);
      out.compilerOpts.includeAsserts = !reader.parseBool(true);
      return;
    case TDIR_MODE: {
      skipValueDelimiter(reader);
      const testmode mode = parseMode(reader);
      if (mode != TESTMODE_INVALID) {
        out.mode = mode;
      }
      return;
    }
    case TDIR_EXPECTED_RUNTIME_ERROR: {
      skipValueDelimiter(reader);
      out.expectedRuntimeError = reader.remainingView();
      return;
    }
    case TDIR_EXPECT_AST: {
      skipValueDelimiter(reader);
      parseExpectedAst(out, reader);
      return;
    }
    default:
      break;
  }

  if (directive != TDIR_EXPECT_ERROR) {
    return;
  }

  ExpectedError err;
  err.line = t->start.line + 1;

  KeyValuePair p;
  while (parsePair(reader, &p)) {
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
  }

  out.expectedErrors.push_back(err);
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

    StringReader reader = StringReader(content);
    parseTestCommand(out, reader, t);
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

  if (!tcase.expectedRuntimeError.empty()) {
    fprintf(file, "\n  \"expected_runtime_error\": \"%s\",", tcase.expectedRuntimeError.c_str());
  }

  fprintf(file, "\n  \"compiler_options\": {");
  fprintf(file, "\n    \"stat_inlining\": %s,", BOOL_STR(tcase.compilerOpts.statOptimizing));
  fprintf(file, "\n    \"expr_inlining\": %s,", BOOL_STR(tcase.compilerOpts.exprOptimizing));
  fprintf(file, "\n    \"include_asserts\": %s", BOOL_STR(tcase.compilerOpts.includeAsserts));
  fprintf(file, "\n  }");
  fprintf(file, "\n}");

  fclose(file);
}

static void dumpAst(const conststring fileName, const TestCase& tcase, Node* result) {
  if (tcase.dumpDirectory.empty()) {
    return;
  }

  std::filesystem::create_directories(tcase.dumpDirectory);

  // Write JSON AST
  JsonPrinter printer;
  result->acceptVisit(&printer);

  const std::string& jsonString = printer.getResult();
  std::ofstream jsonStream(tcase.dumpDirectory / fileName);

  jsonStream << jsonString;
}

static int64 getTime() {
  return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}

bool runTestCase(TestCase& tcase, const std::filesystem::path& filePath, TestContext& tctx) {
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

  int64 lexStart = getTime();
  int64 lexEnd = -1;

  try {
    l.next();
    l.lex();

    lexEnd = getTime();

    parseTestCase(tcase, tlist, table);
  } catch (std::runtime_error& err) {
    lexEnd = getTime();
    parseTestCase(tcase, tlist, table);
    stepFailed = true;
  }

  tctx.timings.lexTimings.account(lexStart, lexEnd);

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

  int64 parseStart = 0;
  int64 parseEnd = 0;

  if (tcase.mode == TESTMODE_RUN || tcase.mode == TESTMODE_VALIDATE) {
    try {
      parseStart = getTime();
      result = p.parse();
    } catch (std::runtime_error& e) {
      stepFailed = true;
    }
  } else {
    try {
      parseStart = getTime();
      result = p.expr();
    } catch (std::runtime_error& e) {
      stepFailed = true;
    }
  }

  parseEnd = getTime();
  tctx.timings.parseTimings.account(parseStart, parseEnd);

  if (stepFailed) {
    return checkErrors(tcase, errors, result);
  }

  dumpAst("ast.json", tcase, result);

  TypeTable lookup = TypeTable();

  if (result->nodeKind() == AST_ScriptFileStatement) {
    int64 analysisStart = getTime();

    SemanticContext ctx = SemanticContext(lookup, table, errors, allocator, tcase.compilerOpts, tctx.bindings);
    runSemanticAnalysis(static_cast<ScriptFileStatement*>(result), ctx);

    int64 analysisEnd = getTime();
    tctx.timings.analysisTimings.account(analysisStart, analysisEnd);

    if (!checkErrors(tcase, errors, result)) {
      return false;
    }

    if (tcase.mode != TESTMODE_RUN) {
      return true;
    }

    int64 transformStart = getTime();

    runSemanticTransformer(ctx, static_cast<ScriptFileStatement*>(result));
    BytecodeFile& bfile = compile(ctx);

    int64 transformEnd = getTime();
    tctx.timings.transformTimings.account(transformStart, transformEnd);

    dumpAst("post-transform-ast.json", tcase, result);

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
    vm.addBindings(tctx.bindings);

    uint32 entryPoint = vm.addBytecodeFile(bfile, pathString);

    int64 execStart = getTime();
    int64 execEnd = execStart;

    try {
      vm.beginExecution(entryPoint, tctx.settings.runArgs);
      execEnd = getTime();
      tctx.timings.execTimings.account(execStart, execEnd);
    } catch (ScriptError& exc) {
      execEnd = getTime();
      tctx.timings.execTimings.account(execStart, execEnd);

      if (!tcase.expectedRuntimeError.empty()) {
        if (tcase.expectedRuntimeError == exc.getMessage()) {
          return true;
        }

        fprintf(stderr, "Expected runtime error didn't match!\n  Expected: %s\n  Found: ", tcase.expectedRuntimeError.c_str());
      } else {
        fprintf(stderr, "Failed with exception: ");
      }

      fprintf(stderr, "%s", exc.getMessage().c_str());

      if (!exc.getCallStack().empty()) {
        fprintf(stderr, "\nCall stack:\n  %s\n", exc.getCallStack().c_str());
      }

      BytecodeFile::destroy(bfile);
      return false;
    }

    BytecodeFile::destroy(bfile);
    return true;
  }

  if (tctx.settings.printAst & PRINTAST_AFTER_TRANSFORM) {
    PrintingVisitor pv = PrintingVisitor(&table, fileName);
    result->acceptVisit(&pv);
  }

  return checkErrors(tcase, errors, result);
}

static void collectTests(
  const std::filesystem::path& path,
  std::vector<std::filesystem::path>& testFiles
) {
  if (std::filesystem::is_regular_file(path)) {
    testFiles.push_back(path);
    return;
  }

  for (const auto& entry: std::filesystem::recursive_directory_iterator(path)) {
    if (!entry.is_regular_file()) {
      continue;
    }
    testFiles.push_back(entry.path());
  }
}

void runTests(const ProgramSettings& settings, const BindingsObject* bindings) {
  const std::filesystem::path dirpath = settings.inputFile;
  std::vector<std::filesystem::path> testFiles;

  collectTests(dirpath, testFiles);

  uint32 total = 0;
  uint32 failed = 0;

  TestContext ctx = {
    .timings = TestTimings(),
    .settings = settings,
    .bindings = bindings
  };

  try {
    for (const std::filesystem::path& path: testFiles) {
      TestCase testCase;

      if (!settings.testDumpDirectory.empty()) {
        std::filesystem::path dumpDir = settings.testDumpDirectory;
        dumpDir /= std::filesystem::relative(path, dirpath);
        testCase.dumpDirectory = dumpDir;
      }

      bool success = runTestCase(testCase, path, ctx);

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

  if (settings.printTestTimings) {
    ctx.timings.printTimings();
  }
}
