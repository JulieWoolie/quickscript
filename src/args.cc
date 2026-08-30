#include "args.h"

#include <cstring>

#define IS_LAST_ARG (i == argc - 1)

#define FLAG_INVALID 0
#define FLAG_PRINT_AST 1
#define FLAG_LOGLEVEL 2
#define FLAG_TEXT_COMPILE 3
#define FLAG_OMIT_OK_TESTS 4
#define FLAG_IGNORE_ASSERTS 5
#define FLAG_TEST_DUMP_DIR 6
#define FLAG_NO_STAT_INLINING 7
#define FLAG_NO_EXPR_INLINING 8
typedef uint8 FlagType;

struct FlagDef {
  FlagType type;
  conststring value;
};

static const FlagDef FLAG_DEFS[] = {
  {.type = FLAG_PRINT_AST, .value = "--print-ast"},
  {.type = FLAG_PRINT_AST, .value = "-p"},
  {.type = FLAG_LOGLEVEL, .value = "--loglevel"},
  {.type = FLAG_TEXT_COMPILE, .value = "--text-compile"},
  {.type = FLAG_TEXT_COMPILE, .value = "-tc"},
  {.type = FLAG_OMIT_OK_TESTS, .value = "--omit-ok-tests"},
  {.type = FLAG_OMIT_OK_TESTS, .value = "-oot"},
  {.type = FLAG_IGNORE_ASSERTS, .value = "--ignore-asserts"},
  {.type = FLAG_IGNORE_ASSERTS, .value = "-ia"},
  {.type = FLAG_TEST_DUMP_DIR, .value = "--test-dump-dir"},
  {.type = FLAG_NO_STAT_INLINING, .value = "--no-stat-inlining"},
  {.type = FLAG_NO_EXPR_INLINING, .value = "--no-expr-inlining"}
};

struct CommandDef {
  programcommand cmd;
  conststring value;
};

static const CommandDef COMMANDS[] = {
  {.cmd = CMD_COMPILE, .value = "compile"},
  {.cmd = CMD_HELP, .value = "help"},
  {.cmd = CMD_HELP, .value = "?"},
  {.cmd = CMD_RUN, .value = "run"},
  {.cmd = CMD_TESTS, .value = "test"}
};

void showHelpMessage() {
  printf("Quickscript Interpreter\n");
  printf("\n");
  printf("USAGE\n");
  printf("    quickscript [OPTIONS] [COMMAND]\n");
  printf("\n");
  printf("OPTIONS\n");
  printf("    --print-ast -p                Print the AST of parsed source files after semantic transformation.\n");
  printf("    --print-ast=<when> -p=<when>  Set a flag to print the AST after a certain compilation step\n");
  printf("                                  (One of: after-parse, after-analysis, after-transform.)\n");
  printf("    --loglevel=<level>            Set the logger level.\n");
  printf("                                  (One of: error, warn, info)\n");
  printf("    --text-compile -tc            Compile to a text based IR instead of binary.\n");
  printf("                                  (Only used for the 'compile' command)\n");
  printf("    --omit-ok-tests -oot          Do not log tests that passed.\n");
  printf("                                  (Only used for the 'test' command)\n");
  printf("    --test-dump-dir=<dir>         Directory to dump random test-related debug info to.\n");
  printf("                                  (Only used for the 'test' command)\n");
  printf("    --ignore-asserts -ia          Do not compile or assert statements.\n");
  printf("                                  (Ignored when using 'test' command)\n");
  printf("    --no-stat-inlining            Do not inline statements.\n");
  printf("                                  (Ignored when using 'test' command)\n");
  printf("    --no-expr-inlining            Do not inline expressions.\n");
  printf("                                  (Ignored when using 'test' command)\n");
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
  FlagType name;
  std::string_view nameView;
  std::string_view value;
};

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

static FlagType parseName(std::string_view nameView) {
  constexpr uint32 defs = sizeof(FLAG_DEFS) / sizeof(FlagDef);
  for (uint32 i = 0; i < defs; i++) {
    FlagDef def = FLAG_DEFS[i];
    if (stringEquals(def.value, nameView)) {
      return def.type;
    }
  }
  return FLAG_INVALID;
}

static void parseFlagPair(const conststring arg, const uint32 len, ArgPair& out) {
  const int32 idx = indexOf(arg, len, '=');

  if (idx == -1) {
    out.nameView = arg;
    out.name = parseName(arg);
    out.value = {};
  } else {
    out.value = std::string_view(arg + idx + 1, len - idx - 1);
    out.nameView = std::string_view(arg, idx);
  }

  out.name = parseName(out.nameView);
}

static bool parseFlag(const ArgPair& pair, ProgramSettings& out) {
  if (pair.value.empty()) {
    switch (pair.name) {
      case FLAG_PRINT_AST:
        out.printAst |= PRINTAST_AFTER_TRANSFORM;
        break;
      case FLAG_TEXT_COMPILE:
        out.compileToBinary = false;
        break;
      case FLAG_OMIT_OK_TESTS:
        out.omitPassedTests = true;
        break;
      case FLAG_IGNORE_ASSERTS:
        out.compilationOptions.includeAsserts = false;
        break;
      case FLAG_NO_EXPR_INLINING:
        out.compilationOptions.exprOptimizing = false;
        break;
      case FLAG_NO_STAT_INLINING:
        out.compilationOptions.statOptimizing = false;
        break;
      default:
        return false;
    }
    return true;
  }

  switch (pair.name) {
    case FLAG_PRINT_AST:
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
      break;
    case FLAG_TEST_DUMP_DIR:
      out.testDumpDirectory = pair.value;
      return true;
    case FLAG_LOGLEVEL:
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
      break;
    default:
      break;
  }

  return false;
}

static void collectFollowingArgsForProgram(ProgramSettings& out, const int32 argc, cstring argv[], const uint32 after) {
  uint64 totalLen = 0;
  const uint32 totalCount = argc - after - 1;
  uint32 lengthBuffer[totalCount];
  uint32 offsetBuffer[totalCount];

  uint32 runningOffset = 0;
  for (uint32 i = after + 1; i < argc; i++) {
    const uint32 len = strlen(argv[i]);
    totalLen += len;
    lengthBuffer[i - after - 1] = len;
    offsetBuffer[i - after - 1] = runningOffset;
    runningOffset += len;
  }

  const uint64 memSize = (totalCount * sizeof(uint32) * 2) + totalLen;
  uint8* memBlock = static_cast<uint8*>(malloc(memSize));

  uint32* offsets = reinterpret_cast<uint32*>(memBlock);
  uint32* lengths = reinterpret_cast<uint32*>(memBlock + (totalCount * sizeof(uint32)));
  int8* cdata = reinterpret_cast<int8*>(memBlock + (totalCount * sizeof(uint32) * 2));

  for (uint32 i = after + 1; i < argc; i++) {
    const uint32 argIdx = i - after - 1;

    const uint32 len = lengthBuffer[argIdx];
    const uint32 off = offsetBuffer[argIdx];
    const conststring characters = argv[i];

    offsets[argIdx] = off;
    lengths[argIdx] = len;

    memcpy(cdata + off, characters, len);
  }

  out.runArgs.count = totalCount;
  out.runArgs.lengths = lengths;
  out.runArgs.starts = offsets;
  out.runArgs.cdata = cdata;
}

static programcommand parseCommand(std::string_view view) {
  constexpr uint32 commands = sizeof(COMMANDS) / sizeof(CommandDef);
  for (uint32 i = 0; i < commands; i++) {
    CommandDef def = COMMANDS[i];
    if (stringEquals(def.value, view)) {
      return def.cmd;
    }
  }
  return CMD_NIL;
}

static void printUsage(const programcommand cmd) {
  fprintf(stderr, "USAGE:\n  quickscript ");

  if (cmd == CMD_HELP) {
    fprintf(stderr, "help");
  } else {
    fprintf(stderr, "[OPTIONS] ");

    switch (cmd) {
      case CMD_RUN:
        fprintf(stderr, "run <INPUT FILE> [PROGRAM ARGUMENTS]");
        break;
      case CMD_COMPILE:
        fprintf(stderr, "compile <INPUT FILE> <OUTPUT FILE>");
        break;
      case CMD_TESTS:
        fprintf(stderr, "test <TEST DIRECTORY>");
        break;
      default:
        break;
    }
  }

  fprintf(stderr, "\n\n");
}

ParseResult parseSettings(ProgramSettings& out, int32 argc, cstring argv[]) {
  bool commandSet = false;
  ArgPair pair;

  for (uint32 i = 1; i < argc; i++) {
    const conststring arg = argv[i];
    const uint32 len = strlen(arg);

    if (arg[0] == '-') {
      parseFlagPair(arg, len, pair);

      const int32 nameLen = pair.nameView.length();
      const conststring nameData = pair.nameView.data();

      if (pair.name == FLAG_INVALID) {
        fprintf(stderr, "Unknown flag '%.*s'\n", nameLen, nameData);
        return RES_FAILED;
      }

      if (parseFlag(pair, out)) {
        continue;
      }

      fprintf(stderr, "Invalid use of flag '%.*s'\n", nameLen, nameData);
      return RES_FAILED;
    }

    if (commandSet) {
      return RES_FAILED;
    }

    const std::string_view view = std::string_view(arg, len);
    const programcommand cmd = parseCommand(view);

    if (cmd == CMD_NIL) {
      fprintf(stderr, "Invalid command '%s'\n", arg);
      return RES_FAILED;
    }

    out.command = cmd;

    if (cmd == CMD_HELP) {
      return RES_OK;
    }
    if (IS_LAST_ARG) {
      fprintf(stderr, "Input file name not specified\n");
      printUsage(cmd);
      return RES_FAILED;
    }

    out.inputFile = argv[++i];
    commandSet = true;

    if (cmd == CMD_COMPILE) {
      if (IS_LAST_ARG) {
        fprintf(stderr, "Output file not specified\n");
        printUsage(CMD_COMPILE);
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
