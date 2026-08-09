#include "args.h"

#include <cstring>

#define IS_LAST_ARG (i == argc - 1)

void showHelpMessage() {
  printf("Quickscript Interpreter\n");
  printf("\n");
  printf("USAGE\n");
  printf("    quickscript [OPTIONS] [COMMAND]\n");
  printf("\n");
  printf("OPTIONS\n");
  printf("    --print-ast -p                Print the AST of parsed source files after semantic transformation.\n");
  printf("    --print-ast=<when> -p=<when>  Set a flag to print the AST after a certain compilation step (One of: after-parse, after-analysis, after-transform.)\n");
  printf("    --loglevel=<level>            Set the logger level. (One of: error, warn, info)\n");
  printf("    --text-compile -tc            Compile to a text based IR instead of binary.\n");
  printf("    --omit-ok-tests -oot          Do not log tests that passed.\n");
  printf("\n");
  printf("COMMANDS\n");
  printf("    help                                Display this help message.\n");
  printf("    run <file> [program arguments]      Run an IR or source file.\n");
  printf("    test <directory>                    Run script files in a directory and treat them as test cases.\n");
  printf("    compile <input file> <output file>  Compile a source file to IR.\n");
  printf("\n");
}

static int32 indexOf(conststring str, uint32 len, int8 ch) {
  for (int32 i = 0; i < len; i++) {
    if (str[i] == ch) {
      return i;
    }
  }
  return -1;
}

struct ArgPair {
  std::string_view name;
  std::string_view value;
};

static void parseFlagPair(const conststring arg, const uint32 len, ArgPair& out) {
  int32 idx = indexOf(arg, len, '=');

  if (idx == -1) {
    out.name = arg;
    out.value = {};
    return;
  }

  out.name = std::string_view(arg, idx);
  out.value = std::string_view(arg + idx + 1, len - idx - 1);
}

static bool stringEquals(const conststring a, const std::string_view& b) {
  const uint32 aLen = strlen(a);
  const uint32 bLen = b.length();

  if (bLen != aLen) {
    return false;
  }

  for (uint32 i = 0; i < aLen; i++) {
    if (b[i] == a[i]) {
      continue;
    }
    return false;
  }

  return true;
}

static bool parseFlag(ProgramSettings& out, const conststring arg, const uint32 len) {
  ArgPair pair;
  parseFlagPair(arg, len, pair);

  if (pair.value.empty()) {
    if (stringEquals("--print-ast", pair.name)) {
      out.printAst |= PRINTAST_AFTER_TRANSFORM;
      return true;
    }
    if (stringEquals("-p", pair.name)) {
      out.printAst = true;
      return true;
    }

    if (stringEquals("--text-compile", pair.name)) {
      out.compileToBinary = false;
      return true;
    }
    if (stringEquals("-tc", pair.name)) {
      out.compileToBinary = false;
      return true;
    }
    if (stringEquals("--omit-ok-tests", pair.name)) {
      out.omitPassedTests = true;
      return true;
    }
    if (stringEquals("-oot", pair.name)) {
      out.omitPassedTests = true;
      return true;
    }

    return false;
  }

  if (stringEquals("--print-ast", pair.name) || stringEquals("-p", pair.name)) {
    if (stringEquals("after-transform", pair.value)) {
      out.printAst |= PRINTAST_AFTER_TRANSFORM;
      return true;
    }
    if (stringEquals("after-parse", pair.value)) {
      out.printAst |= PRINTAST_AFTER_PARSE;
      return true;
    }
    if (stringEquals("after-analysis", pair.value)) {
      out.printAst |= PRINTAST_AFTER_ANALYSIS;
      return true;
    }
  }

  if (stringEquals("--loglevel", pair.name)) {
    if (stringEquals("error", pair.value)) {
      out.loggerLevel = LOGL_ERROR;
      return true;
    }
    if (stringEquals("warn", pair.value)) {
      out.loggerLevel = LOGL_WARN;
      return true;
    }
    if (stringEquals("info", pair.value)) {
      out.loggerLevel = LOGL_INFO;
      return true;
    }
    return false;
  }

  return false;
}

static void collectFollowingArgsForProgram(ProgramSettings& out, const int32 argc, cstring argv[], const uint32 after) {
  uint64 totalLen = 0;
  const uint32 totalCount = argc - after - 1;
  uint32 lenbuf[totalCount];

  for (uint32 i = after + 1; i < argc; i++) {
    const uint32 len = strlen(argv[i]);
    totalLen += len;
    lenbuf[i - after - 1] = len;
  }

  uint64 memSize = totalLen + (totalCount * sizeof(uint32));
  uint32* block = static_cast<uint32*>(malloc(memSize));

  memcpy(block, lenbuf, totalCount);

  int8* cdata = reinterpret_cast<int8*>(block + totalCount);
  uint64 cdataOff = 0;

  for (uint32 i = after + 1; i < argc; i++) {
    const conststring arg = argv[i];
    const uint32 len = lenbuf[i - after - 1];

    memcpy(cdata + cdataOff, arg, len);
    cdataOff += len;
  }
}

static programcommand parseCommand(std::string_view view) {
  switch (view.length()) {
    case 3:
      if (view[0] == 'r'
       && view[1] == 'u'
       && view[2] == 'n'
      ) {
        return CMD_RUN;
      }
      return CMD_NIL;
    case 4:
      if (view[0] == 'h'
       && view[1] == 'e'
       && view[2] == 'l'
       && view[3] == 'p'
      ) {
        return CMD_HELP;
      }
      if (view[0] == 't'
       && view[1] == 'e'
       && view[2] == 's'
       && view[3] == 't'
      ) {
        return CMD_TESTS;
      }
      return CMD_NIL;
    case 7:
      if (view[0] == 'c'
       && view[1] == 'o'
       && view[2] == 'm'
       && view[3] == 'p'
       && view[4] == 'i'
       && view[5] == 'l'
       && view[6] == 'e'
      ) {
        return CMD_COMPILE;
      }
      return CMD_NIL;
    default:
      return CMD_NIL;
  }
}

ParseResult parseSettings(ProgramSettings& out, int32 argc, cstring argv[]) {
  bool commandSet = false;

  for (uint32 i = 1; i < argc; i++) {
    const conststring arg = argv[i];
    const uint32 len = strlen(arg);

    if (arg[0] == '-') {
      if (parseFlag(out, arg, len)) {
        continue;
      }
      return RES_FAILED;
    }

    if (commandSet) {
      return RES_FAILED;
    }

    const std::string_view view = std::string_view(arg, len);
    const programcommand cmd = parseCommand(view);

    if (!cmd) {
      return RES_FAILED;
    }

    out.command = cmd;

    if (cmd == CMD_HELP) {
      return RES_OK;
    }
    if (IS_LAST_ARG) {
      return RES_FAILED;
    }

    out.inputFile = argv[++i];
    commandSet = true;

    if (cmd == CMD_COMPILE) {
      if (IS_LAST_ARG) {
        return RES_FAILED;
      }
      out.outputFile = argv[++i];
    }
    if (cmd == CMD_RUN) {
      if (!IS_LAST_ARG) {
        collectFollowingArgsForProgram(out, argc, argv, i);
      }
      return RES_OK;
    }
  }

  return RES_OK;
}
