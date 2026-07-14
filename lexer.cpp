#include "lexer.h"

#include <utility>

Lexer::Lexer(const std::string& input, TokenList* tokens, StringTable* table) {
  m_input = input;
  m_tokens = tokens;
  m_table = table;

  idx = -1;
  line = 0;
  col = 0;

  advanceLineTracker();
  next();

  peekedToken = nullptr;
  eofToken = nullptr;

  tokenStart.index = 0;
  tokenStart.column = 0;
  tokenStart.line = 0;
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

void Lexer::advanceLineTracker() {
  line++;
  col = 0;
}

int8 Lexer::peek(const int32 ahead) {
  return getchar(idx + 1 + ahead);
}

int8 Lexer::peek() {
  return peek(0);
}

int8 Lexer::next() {
  int32 ncur = idx + 1;

  if (ncur >= m_input.length()) {
    currentChar = EOF;
    idx = m_input.length();
    return EOF;
  }

  int8 nch = getchar(ncur);

  if (nch == LF || nch == CR) {
    advanceLineTracker();

    if (nch == CR && getchar(ncur + 1) == LF) {
      ncur++;
    }

    nch = LF;
  } else {
    col++;
  }

  idx = ncur;
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

Location Lexer::recordLocation() {
  Location loc;
  loc.index = idx;
  loc.column = col;
  loc.line = line;
  return loc;
}

int8 Lexer::getchar(int32 a) {
  if (a < 0 || a >= m_input.length()) {
    return EOF;
  }
  return m_input[a];
}

bool isWhitespace(const int8 ch) {
  return ch == ' '
      || ch == '\t'
      || ch == LF
      || ch == CR;
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

bool isNumeric(const int8 ch) {
  return ch >= '0' && ch <= '9';
}

bool isIdentifierStart(const int8 ch) {
  return (ch >= 'a' && ch <= 'z')
      || (ch >= 'A' && ch <= 'Z')
      || ch == '_'
      || ch == '$';
}

bool isIdentifierPart(const int8 ch) {
  return isIdentifierStart(ch) || isNumeric(ch);
}

Token * Lexer::readToken() {
  skipEmptyContent();

  if (currentChar == EOF) {
    return eoftoken();
  }

  tokenStart.index = idx;
  tokenStart.line = line;
  tokenStart.column = col;

  int8 p = peek();

  switch (currentChar) {
    case '{':
      next();
      return maketoken(TT_LCURL);
    case '}':
      next();
      return maketoken(TT_RCURL);

    case '[':
      next();
      return maketoken(TT_LSQUARE);
    case ']':
      next();
      return maketoken(TT_RSQUARE);

    case '(':
      next();
      return maketoken(TT_LBRACKET);
    case ')':
      next();
      return maketoken(TT_RBRACKET);

    case ':':
      next();
      return maketoken(TT_COLON);
    case ';':
      next();
      return maketoken(TT_SEMICOLON);
    case ',':
      next();
      return maketoken(TT_COMMA);
    case '?':
      next();
      return maketoken(TT_QUESTION);

    case '!':
      next();
      if (currentChar == '=') {
        next();
        return maketoken(TT_NEQ);
      }
      return maketoken(TT_INVERT);
    case '~':
      next();
      return maketoken(TT_BIT_INVERT);
    case '^':
      next();
      if (currentChar == '=') {
        next();
        return maketoken(TT_XOR_ASSIGN);
      }
      return maketoken(TT_XOR);
    case '%':
      next();
      if (currentChar == '=') {
        next();
        return maketoken(TT_PERCENT_ASSIGN);
      }
      return maketoken(TT_PERCENT);
    case '=':
      next();
      if (currentChar == '=') {
        next();
        return maketoken(TT_EQ);
      }
      return maketoken(TT_ASSIGN);
    case '&':
      next();
      if (currentChar == '=') {
        next();
        return maketoken(TT_BIT_AND_ASSIGN);
      }
      if (currentChar == '&') {
        next();
        if (currentChar == '=') {
          next();
          return maketoken(TT_LOGICAL_AND_ASSIGN);
        }
        return maketoken(TT_LOGICAL_AND);
      }
      return maketoken(TT_BIT_AND);
    case '|':
      next();
      if (currentChar == '=') {
        next();
        return maketoken(TT_WALL_ASSIGN);
      }
      if (currentChar == '|') {
        next();
        if (currentChar == '=') {
          next();
          return maketoken(TT_DWALL_ASSIGN);
        }
        return maketoken(TT_DWALL);
      }
      return maketoken(TT_WALL);
    case '*':
      next();
      if (currentChar == '=') {
        next();
        return maketoken(TT_STAR_ASSIGN);
      }
      if (currentChar == '*') {
        next();
        if (currentChar == '=') {
          next();
          return maketoken(TT_POW_ASSIGN);
        }
        return maketoken(TT_POW);
      }
      return maketoken(TT_STAR);
    case '/':
      next();
      if (currentChar == '=') {
        next();
        return maketoken(TT_SLASH_ASSIGN);
      }
      return maketoken(TT_SLASH);

    // Comparison
    case '<':
      next();
      if (currentChar == '<') {
        next();
        if (currentChar == '=') {
          next();
          return maketoken(TT_SHL_ASSIGN);
        }
        return maketoken(TT_SHL);
      }
      if (currentChar == '>') {
        next();
        return maketoken(TT_LAMBDA_ARROW);
      }
      if (currentChar == '=') {
        next();
        return maketoken(TT_LTE);
      }
      return maketoken(TT_LT);
    case '>':
      next();
      if (currentChar == '>') {
        next();
        if (currentChar == '=') {
          next();
          return maketoken(TT_SHR_ASSIGN);
        }
        if (currentChar == '>') {
          next();
          if (currentChar == '=') {
            next();
            return maketoken(TT_USHR_ASSIGN);
          }
          return maketoken(TT_USHR);
        }
        return maketoken(TT_SHR);
      }
      if (currentChar == '=') {
        next();
        return maketoken(TT_GTE);
      }
      return maketoken(TT_GT);

    // inc/dec operators
    case '-':
      next();
      if (currentChar == '-') {
        next();
        return maketoken(TT_DEC);
      }
      if (currentChar == '=') {
        next();
        return maketoken(TT_MINUS_ASSIGN);
      }
      return maketoken(TT_MINUS);
    case '+':
      next();
      if (currentChar == '+') {
        next();
        return maketoken(TT_INC);
      }
      if (currentChar == '=') {
        next();
        return maketoken(TT_PLUS_ASSIGN);
      }
      return maketoken(TT_PLUS);

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
        next();
        return maketoken(TT_DOT);
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

      printf("currentChar='%i'", currentChar);

      next();
      return maketoken(TT_UNKNOWN);

  }
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

Token* Lexer::maketoken(tokentype ttype) {
  Token* t = m_tokens->newToken();

  t->start = tokenStart;
  t->end = recordLocation();

  t->ttype = ttype;

  t->valueId = EMPTY_STRING;

  return t;
}

Token* Lexer::maketokenv(const tokentype ttype) {
  Token* t = maketoken(ttype);
  t->valueId = m_table->allocate(readbuf, readbufLen);
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
    throw std::runtime_error("Invalid Identifier/keyword");
  }

  tokentype keyw = keywordFromString(readbuf, readbufLen);

  if (keyw != TT_UNKNOWN) {
    return maketoken(keyw);
  }

  return maketokenv(TT_ID);
}

bool isHexChar(int8 ch) {
  return (ch >= '0' && ch <= '9')
      || (ch >= 'a' && ch <= 'f')
      || (ch >= 'A' && ch <= 'F');
}

#define TEN ((uint8) 10)

uint8 charHexValue(const int8 ch) {
  if (ch >= '0' && ch <= '9') {
    return ch - '0';
  }
  if (ch >= 'a' && ch <= 'f') {
    return TEN + (ch - 'a');
  }
  return TEN + (ch - 'A');
}

uint16 hexValue(const int8* buf) {
  return (charHexValue(buf[0]) << 12)
       | (charHexValue(buf[1]) << 8)
       | (charHexValue(buf[2]) << 4)
       | charHexValue(buf[3]);
}

uint32 utf8Encode(uint16 codepoint, int8 *out) {
  if (codepoint <= 0x7F) {
    out[0] = static_cast<int8>(codepoint);
    return 1;
  }
  if (codepoint <= 0x7FF) {
    out[0] = 0xC0 | (codepoint >> 6);
    out[1] = 0x80 | (codepoint & 0x3F);
    return 2;
  }

  out[0] = 0xE0 | (codepoint >> 12);
  out[1] = 0x80 | ((codepoint >> 6) & 0x3F);
  out[2] = 0x80 | (codepoint & 0x3F);
  return 3;
}

void Lexer::readHexEscape() {
  int32 len = 0;
  int8 chars[4];

  while (isHexChar(currentChar) && len < 4) {
    chars[len++] = currentChar;
    next();
  }

  if (len != 4) {
    throw std::runtime_error("Invalid unicode escape");
  }

  uint16 hexval = hexValue(chars);

  ensureReadBufWriteable(3);
  uint32 written = utf8Encode(hexval, readbuf + readbufLen);

  readbufLen += written;
  readbuf[readbufLen] = '\0';
}

void Lexer::clearReadBuf() {
  readbufLen = 0;
}

void Lexer::appendToReadBuf() {
  ensureReadBufWriteable(1);
  readbuf[readbufLen++] = currentChar;
  readbuf[readbufLen] = '\0';
}

void Lexer::appendToReadBuf(int8 ch) {
  ensureReadBufWriteable(1);
  readbuf[readbufLen++] = ch;
  readbuf[readbufLen] = '\0';
}

void Lexer::ensureReadBufWriteable(uint32 characters) {
  uint32 nlen = readbufLen + characters + 1;

  if (nlen <= readbufCap) {
    return;
  }

  uint32 ncap = readbufCap + 128;
  int8* nbuf = static_cast<int8*>(malloc(sizeof(int8) * ncap));

  if (!nbuf) {
    throw std::runtime_error("Failed to allocate bigger readbuf");
  }

  readbuf = nbuf;
  readbufCap = ncap;
}

Token* Lexer::readQuotedString() {
  int8 quote = currentChar;
  next();

  bool escaped = false;
  const int32 start = idx;
  int32 chlen = 0;

  clearReadBuf();

  while (true) {
    if (currentChar == EOF) {
      throw std::runtime_error("Unclosed string");
    }
    if (currentChar == LF || currentChar == CR) {
      throw std::runtime_error("Line break inside string");
    }

    if (currentChar == quote) {
      if (escaped) {
        escaped = false;
        appendToReadBuf();
        next();
        chlen++;
        continue;
      }

      break;
    }

    if (currentChar == '\\') {
      next();

      if (escaped) {
        chlen++;
        escaped = false;
        appendToReadBuf('\\');
      } else {
        escaped = true;
      }

      continue;
    }

    if (escaped) {
      int8 ch = currentChar;
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
          throw std::runtime_error("Invalid escape sequence");
      }

      chlen++;
      continue;
    }

    appendToReadBuf();
    next();
  }

  if (currentChar == quote) {
    next();
  } else {
    throw std::runtime_error("Unclosed string");
  }

  tokentype ttype = TT_STRING_LITERAL;

  if (quote == '\'') {
    ttype = TT_CHAR_LITERAL;
    if (chlen != 1) {
      throw std::runtime_error("Char too long");
    }
  }

  return maketokenv(ttype);
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
    int8 n = peek();
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
    throw std::runtime_error("Invalid number");
  }

  return maketokenv(ttype);
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
    throw std::runtime_error(errormsg);\
  }\
  return maketokenv(tt);\
  }

bool isOctoChar(int8 ch) {
  return ch >= '0' && ch <= '7';
}

bool isBinaryChar(int8 ch) {
  return ch == '0' || ch == '1';
}

SPECIAL_NUMBER_READER_METHOD(readHexLiteral, isHexChar, "Invalid hex sequence", TT_HEX_LITERAL)
SPECIAL_NUMBER_READER_METHOD(readOctoLiteral, isOctoChar, "Invalid oct sequence", TT_OCT_LITERAL)
SPECIAL_NUMBER_READER_METHOD(readBinaryLiteral, isBinaryChar, "Invalid binary sequence", TT_BIN_LITERAL)

tokentype keywordFromString(const int8* id, const uint32 len) {
  switch (len) {
    case 2:
      if (id[0] == 'i'
       && id[1] == 'f'
      ) {
        return TT_KEYW_IF;
      }
      if (id[0] == 'd'
       && id[1] == 'o'
      ) {
        return TT_KEYW_DO;
      }
      return TT_UNKNOWN;
    case 3:
      if (id[0] == 'f'
       && id[1] == 'o'
       && id[2] == 'r'
      ) {
        return TT_KEYW_FOR;
      }
      return TT_UNKNOWN;
    case 4:
      if (id[0] == 'e'
       && id[1] == 'l'
       && id[2] == 's'
       && id[3] == 'e'
      ) {
        return TT_KEYW_ELSE;
      }
      if (id[0] == 'n'
       && id[1] == 'u'
       && id[2] == 'l'
       && id[3] == 'l'
      ) {
        return TT_KEYW_NULL;
      }
      return TT_UNKNOWN;
    case 5:
      if (id[0] == 'w'
       && id[1] == 'h'
       && id[2] == 'i'
       && id[3] == 'l'
       && id[4] == 'e'
      ) {
        return TT_KEYW_WHILE;
      }
      if (id[0] == 'b'
       && id[1] == 'r'
       && id[2] == 'e'
       && id[3] == 'a'
       && id[4] == 'k'
      ) {
        return TT_KEYW_BREAK;
      }
      return TT_UNKNOWN;
    case 6:
      if (id[0] == 'r'
       && id[1] == 'e'
       && id[2] == 't'
       && id[3] == 'u'
       && id[4] == 'r'
       && id[5] == 'n'
      ) {
        return TT_KEYW_RETURN;
      }
      if (id[0] == 's'
       && id[1] == 't'
       && id[2] == 'r'
       && id[3] == 'u'
       && id[4] == 'c'
       && id[5] == 't'
      ) {
        return TT_KEYW_STRUCT;
      }
      if (id[0] == 'i'
       && id[1] == 'm'
       && id[2] == 'p'
       && id[3] == 'o'
       && id[4] == 'r'
       && id[5] == 't'
      ) {
        return TT_KEYW_IMPORT;
      }
      if (id[0] == 'm'
       && id[1] == 'o'
       && id[2] == 'd'
       && id[3] == 'u'
       && id[4] == 'l'
       && id[5] == 'e'
      ) {
        return TT_KEYW_MODULE;
      }
      return TT_UNKNOWN;
    case 8:
      if (id[0] == 'c'
       && id[1] == 'o'
       && id[2] == 'n'
       && id[3] == 't'
       && id[4] == 'i'
       && id[5] == 'n'
       && id[6] == 'u'
       && id[7] == 'e'
      ) {
        return TT_KEYW_CONTINUE;
      }
      if (id[0] == 'f'
       && id[1] == 'u'
       && id[2] == 'n'
       && id[3] == 'c'
       && id[4] == 't'
       && id[5] == 'i'
       && id[6] == 'o'
       && id[7] == 'n'
      ) {
        return TT_KEYW_FUNCTION;
      }
      return TT_UNKNOWN;
    default:
      return TT_UNKNOWN;
  }
}