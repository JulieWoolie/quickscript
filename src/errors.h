#ifndef GIT_QUICKSCRIPT_ERRORS_H
#define GIT_QUICKSCRIPT_ERRORS_H
#include "parse/lexer.h"

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

    void fatal(Location& loc, conststring msg, ...) __attribute__((format(printf, 3, 4)));
    void fatal(conststring msg, ...) __attribute__((format(printf, 2, 3)));

    void error(Location& loc, conststring msg, ...) __attribute__((format(printf, 3, 4)));
    void error(conststring msg, ...) __attribute__((format(printf, 2, 3)));

    void warn(Location& loc, conststring msg, ...) __attribute__((format(printf, 3, 4)));
    void warn(conststring msg, ...) __attribute__((format(printf, 2, 3)));

    void info(Location& loc, conststring msg, ...) __attribute__((format(printf, 3, 4)));
    void info(conststring msg, ...) __attribute__((format(printf, 2, 3)));

    void log(loglevel level, Location* l, conststring msg, va_list list);
};

#endif //GIT_QUICKSCRIPT_ERRORS_H
