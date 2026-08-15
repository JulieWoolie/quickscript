#ifndef QUICKSCRIPT_ARGS_H
#define QUICKSCRIPT_ARGS_H

#include "errors.h"

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

struct ProgramArgs {
  int8* cdata = nullptr;
  int32* lengths = nullptr;
  uint32 count = 0;
};

struct ProgramSettings {
  uint8 printAst = PRINTAST_NONE;
  loglevel loggerLevel = LOGL_INFO;
  programcommand command = CMD_HELP;

  // The input file to run for run command
  //    or the file to compile for compile command
  //    or the directory to read tests from for the tests command
  std::string_view inputFile;

  // Used for 'compile' command, it's the file the compiled IR is written
  std::string_view outputFile;

  bool compileToBinary = true;

  ProgramArgs runArgs;

  bool omitPassedTests = false;
  bool ignoreAsserts = true;
};

#define RES_OK 0
#define RES_FAILED 1
typedef uint8 ParseResult;

void showHelpMessage();

ParseResult parseSettings(ProgramSettings& out, int32 argc, cstring argv[]);

#endif //QUICKSCRIPT_ARGS_H
