#ifndef QUICKSCRIPT_ARGS_H
#define QUICKSCRIPT_ARGS_H

#include "errors.h"

//
// == CLI Usage ==
// quickscript <flags> <command> <command args or flags>
//
// == Flag arguments ==
//
// --print-ast or -p:
//     Prints a representation of the Abstract Syntax Tree,
//     Only applies when loading source files, and is
//     printed after analysis is completed.
//
// --loglevel=<level> (One of: info, warn, error)
//     Sets the logging level of the program, setting it to,
//     for example error (with --loglevel=error) will cause
//     all warning messages (For things like unused variables)
//     to be silenced.
//
// --text-compile or -tc:
//     If the flag is set for the compile command, then the
//     compiled IR is transformed into a text based form
//     before being printed to a file.
//
// == Program commands ==
//
// help
//     Prints the usage text along with some other info, if possible.
//
// run <input file> <program arguments>
//     Load a file (May be IR or source code) and execute it
//     In the case of source files, it is compiled, first,
//     obviously
//
// compile <input file> <output file> <flags>
//     Loads a source file and compiles to quickscript IR
//     and then writes the IR contents to the output file
//
// test <directory> <flags>
//     Treats each source file or IR file in the specified
//     directory as a unit test for the compiler to run and
//     executes it.
//

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
};

#define RES_OK 0
#define RES_FAILED 1
typedef uint8 ParseResult;

void showHelpMessage();

ParseResult parseSettings(ProgramSettings& out, int32 argc, cstring argv[]);

#endif //QUICKSCRIPT_ARGS_H
