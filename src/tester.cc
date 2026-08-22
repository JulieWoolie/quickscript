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

struct KeyValuePair {
  std::string_view key;
  std::string_view value;
  uint32 end = 0;
};

static uint32 findTokenEnd(conststring str, uint32 start, uint32 len) {
  if (str[start] == '"' || str[start] == '\'') {
    const int8 q = str[start];

    for (uint32 i = start + 1; i < len; i++) {
      if (str[i] != q) {
        continue;
      }
      return i + 1;
    }

    return len;
  }

  for (uint32 i = start; i < len; i++) {
    if (str[i] != ' ' && str[i] != '=') {
      continue;
    }
    return i;
  }

  return len;
}

static bool parsePair(conststring str, uint32 len, uint32 off, KeyValuePair* out) {
  uint32 readidx = off;
  while (str[readidx] == ' ' && readidx < len) {
    readidx++;
  }

  uint32 keystart = readidx;
  uint32 keyend = findTokenEnd(str, keystart, len);

  if (keystart == keyend) {
    return false;
  }

  readidx = keyend;

  if (str[keyend] == '=') {
    readidx++;
  } else {
    return false;
  }

  uint32 valstart = readidx;
  uint32 valend = findTokenEnd(str, valstart, len);

  if (valstart == valend) {
    return false;
  }

  readidx = valend;

  uint32 keylen = keyend - keystart;
  uint32 vallen = valend - valstart;

  if (str[valstart] == '"' || str[valstart] == '\'') {
    out->value = std::string_view(str + valstart + 1, vallen - 2);
  } else {
    out->value = std::string_view(str + valstart, vallen);
  }

  if (str[keystart] == '"' || str[keystart] == '\'') {
    out->key = std::string_view(str + keylen + 1, keylen - 2);
  } else {
    out->key = std::string_view(str + keystart, keylen);
  }

  out->end = readidx;
  return true;
}

static bool startsWith(conststring str, uint32 off, uint32 len, conststring prefix) {
  uint32 remlen = len - off;
  uint32 prefixlen = strlen(prefix);

  if (remlen < prefixlen) {
    return false;
  }

  conststring start = str + off;

  for (uint32 i = 0; i < prefixlen; i++) {
    if (start[i] == prefix[i]) {
      continue;
    }
    return false;
  }

  return true;
}

static int32 parseViewToInt(std::string_view sv) {
  uint32 len = sv.length();

  char buf[len + 1];
  memcpy(buf, sv.data(), len);
  buf[len] = '\0';

  return atoi(buf);
}

static void parseExpectedAst(TestCase& out, const stringid comment) {
  uint32 readIdx = 0;

  const uint32 len = comment->len;
  conststring data = comment->data;

  while (data[readIdx] == ' ') {
    readIdx++;
  }

  conststring prefix = "EXPECT-AST:";

  if (!startsWith(data, readIdx, len, prefix)) {
    return;
  }

  readIdx += strlen(prefix);
  std::string& jsonString = out.expectedAst;

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

void parseTestCase(TestCase& out, TokenList& list, StringTable& table) {
  const uint32 size = list.size();

  for (uint32 idx = 0; idx < size; idx++) {
    const Token* t = list.get(idx);

    if (t->ttype == TT_BCOMMENT) {
      parseExpectedAst(out, t->valueId);
      continue;
    }

    if (t->ttype != TT_LCOMMENT) {
      continue;
    }

    std::string_view content = table.getview(t->valueId);
    if (content.empty()) {
      continue;
    }

    const conststring data = content.data();
    const uint32 clen = content.length();
    ExpectedError err;
    err.line = t->start.line + 1;

    bool skip = false;
    uint32 readIdx = 0;

    while (readIdx < clen) {
      if (data[readIdx] == ' ') {
        readIdx++;
        continue;
      }

      if (startsWith(data, readIdx, clen, "BREAKPOINT")) {
        out.breakpoint = true;
        skip = true;
        break;
      }

      if (startsWith(data, readIdx, clen, "MODE ")) {
        readIdx += 5;

        if (startsWith(data, readIdx, clen, "run")) {
          out.mode = TESTMODE_RUN;
        } else if (startsWith(data, readIdx, clen, "expr")) {
          out.mode = TESTMODE_EXPR;
        } else {
          out.mode = TESTMODE_VALIDATE;
        }

        skip = true;
        break;
      }

      if (!startsWith(data, readIdx, clen, "ERROR ")) {
        skip = true;
        break;
      }

      readIdx += 5;

      KeyValuePair p;
      while (parsePair(data, clen, readIdx, &p)) {
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
    }

    if (skip) {
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
      fprintf(stderr, "Expected AST did not match, printing parsed AST:\n%s\n", jsonAst.c_str());
      return false;
    }
  }

  // All good size wise
  return !comparisonsFailed;
}

#define RUN_ERROR_CHECKS return checkErrors(&expectedErrors, &errors);

static void breakpoint() {}

bool runTestCase(const std::filesystem::path& filePath, const ProgramSettings& settings) {
  std::ifstream instream(filePath);

  if (!instream.is_open()) {
    fprintf(stderr, "Failed to open %ls\n", filePath.c_str());
    return false;
  }

  std::string file_contents { std::istreambuf_iterator<char>(instream), std::istreambuf_iterator<char>() };

  TokenList tlist;
  StringTable table;

  conststring fileName = reinterpret_cast<conststring>(filePath.c_str());
  CompilerErrors errors = CompilerErrors(&file_contents, fileName);
  errors.setSilent(true);

  Lexer l = Lexer(file_contents, &tlist, &table, &errors);
  l.setCommentsIgnored(false);

  TestCase tcase;
  bool stepFailed = false;

  try {
    l.lex();
    parseTestCase(tcase, tlist, table);
  } catch (std::runtime_error& err) {
    parseTestCase(tcase, tlist, table);
    stepFailed = true;
  }

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

  TypeTable lookup = TypeTable();

  if (result->nodeKind() == AST_ScriptFileStatement) {
    SemanticContext ctx = SemanticContext(lookup, table, errors, allocator);
    runSemanticAnalysis(static_cast<ScriptFileStatement*>(result), ctx);

    if (!checkErrors(tcase, errors, result)) {
      return false;
    }

    if (tcase.mode != TESTMODE_RUN) {
      return true;
    }

    runSemanticTransformer(ctx, static_cast<ScriptFileStatement*>(result));
    BytecodeFile bfile = compile(ctx);

    VirtualMachine vm = VirtualMachine();

    uint32 entryPoint = vm.addBytecodeFile(bfile);
    vm.beginExecution(entryPoint, settings.runArgs);

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
      bool success = runTestCase(path, settings);

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
