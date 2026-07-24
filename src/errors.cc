
#include "errors.h"

#include <stdarg.h>

#define CREATE_LOG_METHOD(name, level) void CompilerErrors::name(Location& loc, conststring msg, ...) {va_list l; va_start(l, msg); log(level, &loc, msg, l); va_end(l);}
#define CREATE_LOG_METHOD_NL(name, level) void CompilerErrors::name(conststring msg, ...) {va_list l; va_start(l, msg); log(level, nullptr, msg, l); va_end(l);}

CompilerErrors::CompilerErrors(std::string *fileContent, conststring fName) {
  m_fileContent = fileContent;
  m_fileName = fName;
}

void CompilerErrors::setSilent(const bool silent) {
  m_silent = silent;
}

uint32 CompilerErrors::getErrorCount() const {
  uint32 count = 0;

  for (const ReportedError& err : m_errors) {
    if (err.level == LOGL_INFO || err.level == LOGL_WARN) {
      continue;
    }
    count++;
  }

  return count;
}

CREATE_LOG_METHOD(fatal, LOGL_FATAL)
CREATE_LOG_METHOD_NL(fatal, LOGL_FATAL)

CREATE_LOG_METHOD(error, LOGL_ERROR)
CREATE_LOG_METHOD_NL(error, LOGL_ERROR)

CREATE_LOG_METHOD(warn, LOGL_WARN)
CREATE_LOG_METHOD_NL(warn, LOGL_WARN)

CREATE_LOG_METHOD(info, LOGL_INFO)
CREATE_LOG_METHOD_NL(info, LOGL_INFO)

static uint32 findLineBoundary(std::string* bufstr, int32 pos, int32 direction) {
  int32 p = pos;
  int32 len = bufstr->length();

  while (true) {
    if (p >= len) {
      return len - 1;
    }
    if (p < 0) {
      return 0;
    }

    const char ch = bufstr->at(p);

    if (ch == '\n' || ch == '\r') {
      if (direction == -1) {
        return p + 1;
      }
      return p;
    }

    p += direction;
  }
}

void repeat(const char c, const uint32 times, char* out) {
  for (uint32 i = 0; i < times; i++) {
    out[i] = c;
  }
  out[times] = '\0';
}

conststring loglevel_name(loglevel l) {
  switch (l) {
    case LOGL_INFO:  return " INFO";
    case LOGL_WARN:  return " WARN";
    case LOGL_ERROR: return "ERROR";
    case LOGL_FATAL: return "FATAL";
    default: return "?????";
  }
}

void CompilerErrors::log(loglevel level, Location *l, conststring msg, va_list list) {
  char content[1024];
  int32 result = vsnprintf_s(content, 1024, 1024, msg, list);

  ReportedError reported;
  reported.level = level;
  reported.message = std::string(content, result);
  if (l) {
    reported.location = *l;
  }

  m_errors.push_back(reported);

  if (!m_silent) {
    printError(reported);
  }

  if (level == LOGL_FATAL) {
    throw std::runtime_error(content);
  }
}

void CompilerErrors::printError(const ReportedError& err) const {
  conststring content = err.message.c_str();
  FILE* out = stderr;

  if (err.level == LOGL_INFO || err.level == LOGL_WARN) {
    out = stdout;
  }

  if (err.location.index == -1) {
    fprintf_s(out, "[%s] %s\n", loglevel_name(err.level), err.message.c_str());
    return;
  }

  int32 index = err.location.index;

  const uint32 linestart = findLineBoundary(m_fileContent, index, -1);
  const uint32 lineend = findLineBoundary(m_fileContent, index, 1);
  const uint32 linelen = lineend - linestart;

  std::string line = m_fileContent->substr(linestart, linelen);

  const uint32 lineno = err.location.line;
  const uint32 col = err.location.column;

  std::string linenoStr = std::to_string(lineno);
  uint32 len = linenoStr.length() + 1;

  char lpad[len];
  repeat(' ', len - 1, lpad);
  lpad[len - 1] = '\0';

  uint32 cpadlen = col < 0 ? 0 : col;
  char cpad[cpadlen + 1];
  repeat(' ', cpadlen, cpad);
  cpad[cpadlen] = '\0';

  fprintf(out,
    "[%s] %s\n%s--> %s:%s:%i\n%s |\n%s | %s\n%s |%s^ %s\n%s |\n",
    loglevel_name(err.level),
    content,
    lpad, m_fileName, linenoStr.c_str(), col,
    lpad,
    lpad, line.c_str(),
    lpad, cpad, content,
    lpad
  );
}
