#ifndef QS_ARGS_H
#define QS_ARGS_H

#include "qs/compiler_opts.hpp"
#include "qs/errors.hpp"

#define CMD_NIL 0
#define CMD_HELP 1
#define CMD_RUN 2
#define CMD_COMPILE 3
#define CMD_TESTS 4
typedef uint8 programcommand;

#define PRINTAST_NONE             0
#define PRINTAST_AFTER_PARSE      (1 << 0)
#define PRINTAST_AFTER_ANALYSIS   (1 << 1)
#define PRINTAST_AFTER_TRANSFORM  (1 << 2)

struct ProgramSettings {
  uint8 printAst = PRINTAST_NONE;
  loglevel loggerLevel = LOGL_INFO;
  programcommand command = CMD_HELP;

  // The input file to run for run command
  //    or the file to compile for compile command
  //    or the directory to read tests from for the tests command
  std::string_view inputFile;

  // Directory random json, ir and txt files are dumped
  // to for more debug info on test cases
  std::string_view testDumpDirectory;

  // Used for 'compile' command, it's the file the compiled IR is written
  std::string_view outputFile;

  bool compileToBinary = true;
  bool omitPassedTests = false;
  bool printTestTimings = false;

  std::vector<std::string_view> runArgs;
  CompilationOptions compilationOptions;
};

#define RES_OK 0
#define RES_FAILED 1
typedef uint8 ParseResult;

void showHelpMessage();

ParseResult parseSettings(ProgramSettings& out, int32 argc, cstring argv[]);

#endif //QS_ARGS_H
