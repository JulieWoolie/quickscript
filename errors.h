#ifndef GIT_QUICKSCRIPT_ERRORS_H
#define GIT_QUICKSCRIPT_ERRORS_H
#include "lexer.h"

typedef uint8 loglevel;

#define LOGL_INFO  0
#define LOGL_WARN  1
#define LOGL_ERROR 2
#define LOGL_FATAL 3

class CompilerErrors {
  std::string* m_fileContent;
  conststring m_fileName;
  bool m_silent = false;

  public:
    CompilerErrors(std::string* fileContent, conststring fName);

    void setSilent(bool silent);

    void fatal(Location& loc, conststring msg, ...);
    void fatal(conststring msg, ...);

    void error(Location& loc, conststring msg, ...);
    void error(conststring msg, ...);

    void warn(Location& loc, conststring msg, ...);
    void warn(conststring msg, ...);

    void info(Location& loc, conststring msg, ...);
    void info(conststring msg, ...);

    void log(loglevel level, Location* l, conststring msg, va_list list);
};

#endif //GIT_QUICKSCRIPT_ERRORS_H
