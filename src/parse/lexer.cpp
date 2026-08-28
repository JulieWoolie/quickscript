#include "lexer.h"

#include <utility>
#include "keyw_lookup.h"
#include "../strings/strings.h"

Lexer::Lexer(const std::string& input, TokenList* tokens, StringTable* table, CompilerErrors* errors) {
  m_input = input;
  m_tokens = tokens;
  m_table = table;
  m_errors = errors;

  idx = 0;
  line = 1;
  col = 0;

  peekedToken = nullptr;
  eofToken = nullptr;

  tokenStart.index = 0;
  tokenStart.column = 0;
  tokenStart.line = 0;
}

Lexer::~Lexer() {
  if (readbuf) {
    free(readbuf);
    readbuf = nullptr;
    readbufCap = 0;
  }
}

void Lexer::setCommentsIgnored(bool ignored) {
  ignoreComments = ignored;
}

void Lexer::lex() {
  Token* t = nextToken();
  while (t->ttype != TT_EOF) {
    t = nextToken();
  }

  Token* last = m_tokens->get(m_tokens->size() - 1);
  if (last->ttype == EOF) {
    return;
  }

  Token* eof = m_tokens->newToken();
  eof->start = last->end;
  eof->end = last->end;
  eof->ttype = TT_EOF;
  eof->valueId = EMPTY_STRING;
}

utf32char Lexer::peek(const int32 ahead) const {
  const uint32 index = idx + ahead;
  utf32char ch = 0;
  getchar(index, &ch);
  return ch;
}

utf32char Lexer::peek() const {
  return peek(0);
}

utf32char Lexer::next() {
  if (idx >= m_input.length()) {
    currentChar = EOF;
    idx = m_input.length();
    return EOF;
  }

  utf32char nch = 0;
  uint8 advanceBy = getchar(idx, &nch);

  if (advanceBy == 0) {
    Location l = recordLocation();
    m_errors->fatal(l, "Invalid unicode codepoint found");
  }

  if (nch == LF || nch == CR) {
    line++;
    col = 0;

    if (nch == CR) {
      utf32char next = 0;
      const uint8 nextLen = getchar(idx + advanceBy, &next);

      if (next == LF) {
        advanceBy += nextLen;
      }
    }

    nch = LF;
  } else {
    col++;
  }

  idx += advanceBy;
  currentChar = nch;

  return currentChar;
}

Token* Lexer::peekToken() {
  if (peekedToken != nullptr) {
    return peekedToken;
  }
  return peekedToken = readToken();
}

Token* Lexer::nextToken() {
  if (peekedToken != nullptr) {
    Token* t = peekedToken;
    peekedToken = nullptr;
    return t;
  }
  return readToken();
}

Location Lexer::recordLocation() const {
  Location loc;
  loc.index = idx;
  loc.column = col;
  loc.line = line;
  return loc;
}

int8 Lexer::getchar(const int32 readIdx, utf32char* out) const {
  if (readIdx < 0 || readIdx >= m_input.length()) {
    return EOF;
  }

  const utf8char* buf = reinterpret_cast<const utf8char*>(m_input.c_str());
  buf = buf + readIdx;

  return decodeUtf8(buf, out, m_input.length() - readIdx);
}

void Lexer::skipEmptyContent() {
  while (true) {
    if (currentChar == EOF) {
      break;
    }

    if (isWhitespace(currentChar)) {
      next();
      continue;
    }

    if (!ignoreComments) {
      break;
    }

    if (currentChar == '#' || (currentChar == COMMENT_CHAR && peek() == COMMENT_CHAR)) {
      skipLineComment();
      continue;
    }

    if (currentChar == COMMENT_CHAR && peek() == STAR_CHAR) {
      skipBlockComment();
      continue;
    }

    break;
  }
}

void Lexer::skipLineComment() {
  while (currentChar != LF && currentChar != EOF) {
    next();
  }
}

void Lexer::skipBlockComment() {
  while (currentChar != EOF) {
    if (currentChar == STAR_CHAR && peek() == COMMENT_CHAR) {
      next();
      next();
      break;
    }

    next();
  }
}

Token * Lexer::readToken() {
  skipEmptyContent();

  if (currentChar == EOF) {
    return eoftoken();
  }

  tokenStart.index = idx;
  tokenStart.line = line;
  tokenStart.column = col;

  const utf32char p = peek();

  if (!ignoreComments) {
    if (currentChar == '#' || (currentChar == COMMENT_CHAR && p == COMMENT_CHAR)) {
      return readLineComment();
    }
    if (currentChar == COMMENT_CHAR && p == STAR_CHAR) {
      return readBlockComment();
    }
  }

  switch (currentChar) {
    case '{':
      next();
      return token(TT_LCURL);
    case '}':
      next();
      return token(TT_RCURL);

    case '[':
      next();
      return token(TT_LSQUARE);
    case ']':
      next();
      return token(TT_RSQUARE);

    case '(':
      next();
      return token(TT_LBRACKET);
    case ')':
      next();
      return token(TT_RBRACKET);

    case ':':
      next();
      return token(TT_COLON);
    case ';':
      next();
      return token(TT_SEMICOLON);
    case ',':
      next();
      return token(TT_COMMA);
    case '?':
      next();
      return token(TT_QUESTION);

    case '!':
      next();
      if (currentChar == '=') {
        next();
        return token(TT_NEQ);
      }
      return token(TT_INVERT);
    case '~':
      next();
      return token(TT_BIT_INVERT);
    case '^':
      next();
      if (currentChar == '=') {
        next();
        return token(TT_XOR_ASSIGN);
      }
      return token(TT_XOR);
    case '%':
      next();
      if (currentChar == '=') {
        next();
        return token(TT_PERCENT_ASSIGN);
      }
      return token(TT_PERCENT);
    case '=':
      next();
      if (currentChar == '=') {
        next();
        return token(TT_EQ);
      }
      return token(TT_ASSIGN);
    case '&':
      next();
      if (currentChar == '=') {
        next();
        return token(TT_BIT_AND_ASSIGN);
      }
      if (currentChar == '&') {
        next();
        if (currentChar == '=') {
          next();
          return token(TT_LOGICAL_AND_ASSIGN);
        }
        return token(TT_LOGICAL_AND);
      }
      return token(TT_BIT_AND);
    case '|':
      next();
      if (currentChar == '=') {
        next();
        return token(TT_WALL_ASSIGN);
      }
      if (currentChar == '|') {
        next();
        if (currentChar == '=') {
          next();
          return token(TT_DWALL_ASSIGN);
        }
        return token(TT_DWALL);
      }
      return token(TT_WALL);
    case '*':
      next();
      if (currentChar == '=') {
        next();
        return token(TT_STAR_ASSIGN);
      }
      if (currentChar == '*') {
        next();
        if (currentChar == '=') {
          next();
          return token(TT_POW_ASSIGN);
        }
        return token(TT_POW);
      }
      return token(TT_STAR);
    case '/':
      next();
      if (currentChar == '=') {
        next();
        return token(TT_SLASH_ASSIGN);
      }
      return token(TT_SLASH);

    // Comparison
    case '<':
      next();
      if (currentChar == '<') {
        next();
        if (currentChar == '=') {
          next();
          return token(TT_SHL_ASSIGN);
        }
        return token(TT_SHL);
      }
      if (currentChar == '>') {
        next();
        return token(TT_LAMBDA_ARROW);
      }
      if (currentChar == '=') {
        next();
        return token(TT_LTE);
      }
      return token(TT_LT);
    case '>':
      next();
      if (currentChar == '>') {
        next();
        if (currentChar == '=') {
          next();
          return token(TT_SHR_ASSIGN);
        }
        if (currentChar == '>') {
          next();
          if (currentChar == '=') {
            next();
            return token(TT_USHR_ASSIGN);
          }
          return token(TT_USHR);
        }
        return token(TT_SHR);
      }
      if (currentChar == '=') {
        next();
        return token(TT_GTE);
      }
      return token(TT_GT);

    // inc/dec operators
    case '-':
      next();
      if (currentChar == '-') {
        next();
        return token(TT_DEC);
      }
      if (currentChar == '=') {
        next();
        return token(TT_MINUS_ASSIGN);
      }
      return token(TT_MINUS);
    case '+':
      next();
      if (currentChar == '+') {
        next();
        return token(TT_INC);
      }
      if (currentChar == '=') {
        next();
        return token(TT_PLUS_ASSIGN);
      }
      return token(TT_PLUS);

    case '\'':
    case '"':
      return readQuotedString();

    case '0':
      if (p == 'x' || p == 'X') {
        return readHexLiteral();
      }
      if (p == 'o' || p == 'O') {
        return readOctoLiteral();
      }
      if (p == 'b' || p == 'B') {
        return readBinaryLiteral();
      }
    case '.':
      if (currentChar == '.' && !isNumeric(p)) {
        if (p == '.' && peek(1) == '.') {
          next();
          next();
          next();
          return token(TT_THREE_DOTS);
        }
        next();
        return token(TT_DOT);
      }
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      return readNumberLiteral();

    default:
      if (isIdentifierStart(currentChar)) {
        return readIdOrKeyword();
      }

      if (currentChar == EOF) {
        return eoftoken();
      }

      next();
      return token(TT_UNKNOWN);

  }
}

Token* Lexer::readBlockComment() {
  next();
  next();

  clearReadBuf();
  while (true) {
    if (currentChar == EOF) {
      break;
    }
    if (currentChar == STAR_CHAR && peek() == COMMENT_CHAR) {
      next();
      next();
      break;
    }

    appendToReadBuf();
    next();
  }

  return valueToken(TT_BCOMMENT);
}

Token* Lexer::readLineComment() {
  if (currentChar == COMMENT_CHAR) {
    next();
    next();
  } else {
    next();
  }

  clearReadBuf();
  while (true) {
    if (currentChar == EOF) {
      break;
    }

    if (currentChar == LF) {
      next();
      break;
    }

    appendToReadBuf();
    next();
  }

  return valueToken(TT_LCOMMENT);
}

Token* Lexer::eoftoken() {
  if (eofToken == nullptr) {
    eofToken = new Token();
    eofToken->ttype = TT_EOF;
    eofToken->start = recordLocation();
    eofToken->end = eofToken->start;
  }
  return eofToken;
}

Token* Lexer::token(const tokentype ttype) const {
  Token* t = m_tokens->newToken();

  t->start = tokenStart;
  t->end = recordLocation();

  t->ttype = ttype;

  t->valueId = EMPTY_STRING;

  return t;
}

Token* Lexer::valueToken(const tokentype ttype) const {
  Token* t = token(ttype);
  t->valueId = m_table->allocate(reinterpret_cast<conststring>(readbuf), readbufLen);
  return t;
}

Token* Lexer::readIdOrKeyword() {
  int32 start = idx;

  clearReadBuf();

  while (isIdentifierPart(currentChar)) {
    appendToReadBuf();
    next();
  }

  int32 end = idx;
  if (start == end) {
    m_errors->fatal(tokenStart, "Invalid Identifier/keyword");
  }

  const tokentype keyword = tokenTypeFromString(reinterpret_cast<conststring>(readbuf), readbufLen);

  if (keyword != TT_UNKNOWN) {
    return token(keyword);
  }

  return valueToken(TT_ID);
}

#define TEN 10u

static uint32 charHexValue(const utf8char ch) {
  if (ch >= '0' && ch <= '9') {
    return ch - '0';
  }
  if (ch >= 'a' && ch <= 'f') {
    return TEN + (ch - 'a');
  }
  return TEN + (ch - 'A');
}

void Lexer::readHexEscape() {
  utf32char result = 0;
  Location location = recordLocation();

  while (isHexChar(currentChar)) {
    result = (result << 4) | charHexValue(currentChar);
    next();
  }

  if (result > MAX_4BYTE) {
    m_errors->fatal(location, "Invalid unicode escape sequence");
  }

  appendToReadBuf(result);
}

void Lexer::clearReadBuf() {
  readbufLen = 0;
}

void Lexer::appendToReadBuf() {
  appendToReadBuf(currentChar);
}

void Lexer::appendToReadBuf(const utf32char ch) {
  const uint8 len = getUtf8ByteLength(ch);
  
  ensureReadBufWriteable(len);
  encodeToUtf8(ch, readbuf + readbufLen);
  
  readbufLen += len;
  readbuf[readbufLen] = '\0';
}

void Lexer::ensureReadBufWriteable(const uint32 characters) {
  const uint32 newLen = readbufLen + characters + 1;

  if (newLen <= readbufCap) {
    return;
  }

  const uint32 newCap = readbufCap + 128;
  utf8char* newBuffer = static_cast<utf8char*>(realloc(readbuf, sizeof(utf8char) * newCap));

  if (!newBuffer) {
    throw std::runtime_error("Failed to allocate bigger readbuf");
  }

  readbuf = newBuffer;
  readbufCap = newCap;
}

Token* Lexer::readQuotedString() {
  const utf32char quote = currentChar;
  next();

  bool escaped = false;
  const int32 start = idx;

  clearReadBuf();

  while (true) {
    if (currentChar == EOF) {
      m_errors->fatal(tokenStart, "Unclosed string");
    }
    if (currentChar == LF || currentChar == CR) {
      m_errors->fatal(tokenStart, "Line break inside string");
    }

    if (currentChar == quote) {
      if (escaped) {
        escaped = false;
        appendToReadBuf();
        next();
        continue;
      }

      break;
    }

    if (currentChar == '\\') {
      next();

      if (escaped) {
        escaped = false;
        appendToReadBuf('\\');
      } else {
        escaped = true;
      }

      continue;
    }

    if (escaped) {
      const utf32char ch = currentChar;
      next();
      escaped = false;

      switch (ch) {
        case 't':
        case 'T':
          appendToReadBuf('\t');
        case 'r':
        case 'R':
          appendToReadBuf('\r');
        case 'n':
        case 'N':
          appendToReadBuf('\n');
          break;

        case 'u':
        case 'U':
          readHexEscape();
          break;

        default:
          Location l = recordLocation();
          m_errors->fatal(l, "Invalid escape sequence");
      }

      continue;
    }

    appendToReadBuf();
    next();
  }

  if (currentChar == quote) {
    next();
  } else {
    m_errors->fatal(tokenStart, "Unclosed string");
  }

  tokentype ttype = TT_STRING_LITERAL;

  if (quote == '\'') {
    ttype = TT_CHAR_LITERAL;
    if (readbufLen != 1) {
      m_errors->fatal(tokenStart, "Char literal too long");
    }
  }

  return valueToken(ttype);
}

Token* Lexer::readNumberLiteral() {
  tokentype ttype = TT_INT_LITERAL;

  int32 start = idx;
  int32 end = start;

  clearReadBuf();

  while (isNumeric(currentChar)) {
    appendToReadBuf();
    next();
  }

  if (currentChar == '.' && isNumeric(peek())) {
    next();
    appendToReadBuf('.');
    ttype = TT_FLOAT_LITERAL;

    while (isNumeric(currentChar)) {
      appendToReadBuf();
      next();
    }
  }

  if (currentChar == 'e' || currentChar == 'E') {
    utf32char n = peek();
    if (n == '+' || n == '-') {
      n = peek(1);
    }

    if (isNumeric(n)) {
      appendToReadBuf('e');
      next();

      if (currentChar == '+' || currentChar == '-') {
        appendToReadBuf();
        next();
      }

      while (isNumeric(currentChar)) {
        appendToReadBuf();
        next();
      }
    }
  }

  end = idx;

  if (start == end) {
    m_errors->fatal(tokenStart, "Invalid number");
  }

  return valueToken(ttype);
}

#define SPECIAL_NUMBER_READER_METHOD(name, testMethod, errormsg, tt) Token* Lexer::name() { \
  next();\
  next();\
  clearReadBuf();\
  while(testMethod(currentChar)) {\
    appendToReadBuf();\
    next();\
  }\
  if (readbufLen == 0) {\
    m_errors->fatal(tokenStart, errormsg);\
  }\
  return valueToken(tt);\
  }

static bool isOctoChar(const int8 ch) {
  return ch >= '0' && ch <= '7';
}

static bool isBinaryChar(const int8 ch) {
  return ch == '0' || ch == '1';
}

SPECIAL_NUMBER_READER_METHOD(readHexLiteral, isHexChar, "Invalid hex sequence", TT_HEX_LITERAL)
SPECIAL_NUMBER_READER_METHOD(readOctoLiteral, isOctoChar, "Invalid oct sequence", TT_OCT_LITERAL)
SPECIAL_NUMBER_READER_METHOD(readBinaryLiteral, isBinaryChar, "Invalid binary sequence", TT_BIN_LITERAL)