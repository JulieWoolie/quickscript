#include "tester.h"

#include <charconv>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <vector>

#include "allocator.h"
#include "errors.h"
#include "analysis/LexicalAnalyzer.h"
#include "analysis/TypeResolver.h"
#include "parse/lexer.h"
#include "parse/parser.h"
#include "parse/print-visitor.h"
#include "parse/token.h"

struct ExpectedError {
  int32 line = -1;
  std::string message;
  std::string name;
};

struct KeyValuePair {
  std::string_view key;
  std::string_view value;
  uint32 end = 0;
};

uint32 findTokenEnd(conststring str, uint32 start, uint32 len) {
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

bool parsePair(conststring str, uint32 len, uint32 off, KeyValuePair* out) {
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

bool startsWith(conststring str, uint32 off, uint32 len, conststring prefix) {
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

int32 parseViewToInt(std::string_view sv) {
  uint32 len = sv.length();

  char buf[len + 1];
  memcpy(buf, sv.data(), len);
  buf[len] = '\0';

  return atoi(buf);
}

void findExpectedErrors(std::vector<ExpectedError>* out, TokenList* list, StringTable* table) {
  uint32 size = list->size();

  for (uint32 idx = 0; idx < size; idx++) {
    Token* t = list->get(idx);
    if (t->ttype != TT_LCOMMENT) {
      continue;
    }

    std::string_view content = table->getview(t->valueId);
    if (content.empty()) {
      continue;
    }

    conststring data = content.data();
    uint32 clen = content.length();
    ExpectedError err;

    bool skip = false;

    uint32 readidx = 0;
    while (readidx < clen) {
      if (data[readidx] == ' ') {
        readidx++;
        continue;
      }

      if (!startsWith(data, readidx, clen, "ERROR ")) {
        skip = true;
        break;
      }

      readidx += 5;

      KeyValuePair p;
      while (parsePair(data, clen, readidx, &p)) {
        if (p.key == "name") {
          err.name = std::string(p.value);
        } else if (p.key == "message") {
          err.message = std::string(p.value);
        } else if (p.key == "line") {
          err.line = parseViewToInt(p.value);
        }
        readidx = p.end;
      }
    }

    if (skip) {
      continue;
    }

    out->push_back(err);
  }
}

bool checkError(const ReportedError& rep, const ExpectedError& expect) {
  if (rep.message != expect.message) {
    fprintf(stderr,
      "[CONDITION FAIL]\n  Expected error message: %s\n  Actual: %s\n",
      expect.message.c_str(),
      rep.message.c_str()
    );
    return false;
  }

  if (expect.line == -1 || expect.line == rep.location.line) {
    return true;
  }

  fprintf(stderr,
    "[CONDITION FAIL]\n  Expected error on line %i\n  Actual: %i\n",
    expect.line,
    rep.location.line
  );

  return false;
}

#define COMPARE_REPORTED_ERRORS(len) \
  for (uint32 i = 0; i < len; i++) {\
    const ExpectedError& err = errors->at(i);\
    const ReportedError& r = reported.at(i);\
    \
    if (checkError(r, err)) {\
      continue;\
    }\
    \
    comparisonsFailed = true;\
  }

bool checkErrors(std::vector<ExpectedError>* errors, CompilerErrors* compilerErrors) {
  int32 expected = errors->size();
  int32 actual = compilerErrors->getErrorCount();

  const std::vector<ReportedError>& reported = compilerErrors->getErrors();
  bool comparisonsFailed = false;

  if (expected > actual) {
    // some errors did not occur
    COMPARE_REPORTED_ERRORS(actual)

    for (uint32 i = actual; i < expected; i++) {
      const ExpectedError& err = errors->at(i);
      fprintf(stderr,
        "[CONDITION FAIL] Expected error did not ocurr!\n  Expected message: %s\n",
        err.message.c_str()
      );
    }

    return false;
  }

  if (expected < actual) {
    // some unexpected errors occurred
    COMPARE_REPORTED_ERRORS(expected)

    fprintf(stderr, "[CONDITION FAIL] Unexpected error(s) when running test!\n");
    for (uint32 i = expected; i < actual; i++) {
      compilerErrors->printError(reported.at(i));
    }

    return false;
  }

  // All good size wise
  COMPARE_REPORTED_ERRORS(expected)
  return !comparisonsFailed;
}

#define RUN_ERROR_CHECKS return checkErrors(&expectedErrors, &errors);

bool runTestFile(const std::filesystem::path& fpath, TesterSettings* settings) {
  std::ifstream instream(fpath);

  if (!instream.is_open()) {
    fprintf(stderr, "Failed to open %ls\n", fpath.c_str());
    return false;
  }

  std::string file_contents { std::istreambuf_iterator<char>(instream), std::istreambuf_iterator<char>() };

  TokenList tlist;
  StringTable table;

  conststring fname = reinterpret_cast<conststring>(fpath.c_str());

  CompilerErrors errors = CompilerErrors(&file_contents, fname);
  errors.setSilent(true);

  Lexer l = Lexer(file_contents, &tlist, &table, &errors);
  l.setCommentsIgnored(false);

  std::vector<ExpectedError> expectedErrors;

  bool stepFailed = false;
  try {
    l.lex();
    findExpectedErrors(&expectedErrors, &tlist, &table);
  } catch (std::runtime_error& e) {
    findExpectedErrors(&expectedErrors, &tlist, &table);
    stepFailed = true;
  }

  if (stepFailed) {
    RUN_ERROR_CHECKS
  }

  NoFreeAllocator allocator;
  Parser p = Parser(&tlist, &allocator, &errors, &table);
  ScriptFileStatement* sfs = nullptr;

  stepFailed = false;
  try {
    sfs = p.parse();
  } catch (std::runtime_error& e) {
    stepFailed = true;
  }

  if (stepFailed) {
    RUN_ERROR_CHECKS
  }

  TypeLookup lookup = TypeLookup(&allocator);
  Bindings bindings;

  TypeResolver typeResolver = TypeResolver(&lookup, &table, &errors, &bindings);
  LexicalAnalyzer analyzer = LexicalAnalyzer(&table, &errors, &bindings);

  typeResolver.acceptScriptFileStatement(sfs);
  analyzer.acceptScriptFileStatement(sfs);

  if (settings->printAsts) {
    PrintingVisitor pv = PrintingVisitor(&table, fname);
    pv.acceptScriptFileStatement(sfs);
  }

  RUN_ERROR_CHECKS
}

void runTests(TesterSettings settings) {
  std::filesystem::path dirpath = settings.directory;

  uint32 total = 0;
  uint32 failed = 0;

  try {
    for (const auto& entry: std::filesystem::recursive_directory_iterator(dirpath)) {
      if (!entry.is_regular_file()) {
        continue;
      }

      const std::filesystem::path& path = entry.path();
      bool success = runTestFile(path, &settings);

      total++;

      if (success) {
        fprintf(stdout, "[OK] Test '%ls' passed\n", path.c_str());
        continue;
      }

      failed++;
      fprintf(stderr, "[TEST FAIL] Test '%ls' failed\n", path.c_str());
    }
  } catch (const std::filesystem::filesystem_error& e) {
    // ignore
  }

  fprintf(stdout, "[TESTING] Ran %i tests, %i failed\n", total, failed);
}
