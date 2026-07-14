#include "lexer.h"

#include <utility>

conststring tokentype_name(tokentype ttype) {
  switch (ttype) {
    case TT_EOF:
      return "END_OF_FILE";

    case TT_ID:
      return "IDENTIFIER";

    case TT_LBRACKET:
      return "'('";
    case TT_RBRACKET:
      return "')'";
    case TT_LCURL:
      return "'{'";
    case TT_RCURL:
      return "'}'";
    case TT_LSQUARE:
      return "'['";
    case TT_RSQUARE:
      return "']'";
    case TT_COLON:
      return "':'";
    case TT_SEMICOLON:
      return "';'";
    case TT_COMMA:
      return "','";
    case TT_DOT:
      return "'.'";

    case TT_BIT_AND:
      return "'&'";
    case TT_BIT_AND_ASSIGN:
      return "'&='";
    case TT_LOGICAL_AND:
      return "'&&'";
    case TT_LOGICAL_AND_ASSIGN:
      return "'&&='";
    case TT_WALL:
      return "'|'";
    case TT_WALL_ASSIGN:
      return "'|='";
    case TT_STAR:
      return "'*'";
    case TT_STAR_ASSIGN:
      return "'*='";
    case TT_SLASH:
      return "'/'";
    case TT_SLASH_ASSIGN:
      return "'/='";
    case TT_PLUS:
      return "'+'";
    case TT_PLUS_ASSIGN:
      return "'+='";
    case TT_MINUS:
      return "'-'";
    case TT_MINUS_ASSIGN:
      return "'-='";
    case TT_XOR:
      return "'^'";
    case TT_XOR_ASSIGN:
      return "'^='";
    case TT_PERCENT:
      return "'%'";
    case TT_PERCENT_ASSIGN:
      return "'%='";
    case TT_SHL:
      return "'<<'";
    case TT_SHL_ASSIGN:
      return "'<<='";
    case TT_SHR:
      return "'>>'";
    case TT_SHR_ASSIGN:
      return "'>>='";
    case TT_USHR:
      return "'>>>'";
    case TT_USHR_ASSIGN:
      return "'>>>='";
    case TT_LT:
      return "'<'";
    case TT_LTE:
      return "'<='";
    case TT_GT:
      return "'>'";
    case TT_GTE:
      return "'>='";
    case TT_INC:
      return "'++'";
    case TT_DEC:
      return "'--'";
    case TT_EQ:
      return "'=='";
    case TT_NEQ:
      return "'!='";
    case TT_ASSIGN:
      return "'='";
    case TT_INVERT:
      return "'!'";
    case TT_BIT_INVERT:
      return "'~'";
    case TT_LAMBDA_ARROW:
      return "'=>'";
    case TT_POW:
      return "'**'";
    case TT_POW_ASSIGN:
      return "'**='";

    case TT_HEX_LITERAL:
      return "HEX_LITERAL";
    case TT_OCT_LITERAL:
      return "OCT_LITERAL";
    case TT_BIN_LITERAL:
      return "BIN_LITERAL";
    case TT_INT_LITERAL:
      return "INT_LITERAL";
    case TT_FLOAT_LITERAL:
      return "FLOAT_LITERAL";
    case TT_STRING_LITERAL:
      return "STRING_LITERAL";
    case TT_CHAR_LITERAL:
      return "CHAR_LITERAL";

    case TT_KEYW_TRUE:
      return "'true'";
    case TT_KEYW_FALSE:
      return "'false'";
    case TT_KEYW_NULL:
      return "'null'";
    case TT_KEYW_IF:
      return "'if'";
    case TT_KEYW_ELSE:
      return "'else'";
    case TT_KEYW_BREAK:
      return "'break'";
    case TT_KEYW_CONTINUE:
      return "'continue'";
    case TT_KEYW_RETURN:
      return "'return'";
    case TT_KEYW_WHILE:
      return "'while'";
    case TT_KEYW_FOR:
      return "'for'";
    case TT_KEYW_FUNCTION:
      return "'function'";
    case TT_KEYW_STRUCT:
      return "'struct'";
    case TT_KEYW_MODULE:
      return "'module'";
    case TT_KEYW_IMPORT:
      return "'import'";
    case TT_KEYW_DO:
      return "'do'";

    default:
      return "UNKNOWN";
  }
}

Lexer::Lexer(const std::string& input) {
  m_input = input;

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
  Token* t = new Token();

  t->start = tokenStart;
  t->end = recordLocation();

  t->ttype = ttype;

  t->valueStart = -1;
  t->valueEnd = -1;

  return t;
}
Token* Lexer::readIdOrKeyword() {
  int32 start = idx;

  while (isIdentifierPart(currentChar)) {
    next();
  }

  int32 end = idx;
  if (start == end) {
    throw std::runtime_error("Invalid Identifier/keyword");
  }

  std::string sub = m_input.substr(start, end - start);
  tokentype keyw = keywordFromString(sub);

  if (keyw != TT_UNKNOWN) {
    return maketoken(keyw);
  }

  Token* t = maketoken(TT_ID);
  t->valueStart = start;
  t->valueEnd = end;

  return t;
}

bool isHexChar(int8 ch) {
  return (ch >= '0' && ch <= '9')
      || (ch >= 'a' && ch <= 'f')
      || (ch >= 'A' && ch <= 'F');
}

void Lexer::readHexEscape() {
  int32 start = idx;
  int32 end = idx;
  int32 len = 0;

  while (isHexChar(currentChar) && len < 4) {
    next();
    len++;
  }

  end = idx;

  if (len != 4) {
    throw std::runtime_error("Invalid unicode escape");
  }
}

Token* Lexer::readQuotedString() {
  int8 quote = currentChar;
  next();

  bool escaped = false;
  const int32 start = idx;
  int32 chlen = 0;

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
        case 'r':
        case 'R':
        case 'n':
        case 'N':
          // All good, skip
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

    next();
    chlen++;
  }

  const int32 end = idx;

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

  Token* tok = maketoken(ttype);
  tok->valueStart = start;
  tok->valueEnd = end;

  return tok;
}

Token* Lexer::readNumberLiteral() {
  tokentype ttype = TT_INT_LITERAL;

  int32 start = idx;
  int32 end = start;

  while (isNumeric(currentChar)) {
    next();
  }

  if (currentChar == '.' && isNumeric(peek())) {
    next();
    ttype = TT_FLOAT_LITERAL;

    while (isNumeric(currentChar)) {
      next();
    }
  }

  if (currentChar == 'e' || currentChar == 'E') {
    int8 n = peek();
    if (n == '+' || n == '-') {
      n = peek(1);
    }

    if (isNumeric(n)) {
      next();
      if (currentChar == '+' || currentChar == '-') {
        next();
      }

      while (isNumeric(currentChar)) {
        next();
      }
    }
  }

  end = idx;

  if (start == end) {
    throw std::runtime_error("Invalid number");
  }

  Token* tok = maketoken(ttype);
  tok->valueStart = start;
  tok->valueEnd = end;

  return tok;
}

Token* Lexer::readHexLiteral() {
  next(); // Skip 0
  next(); // Skip x

  int32 start = idx;

  while (isHexChar(currentChar)) {
    next();
  }

  int32 end = idx;

  if (start == end) {
    throw std::runtime_error("Invalid hex sequence");
  }

  Token* tok = maketoken(TT_HEX_LITERAL);
  tok->valueStart = start;
  tok->valueEnd = end;

  return tok;
}

bool isOctoChar(int8 ch) {
  return ch >= '0' && ch <= '7';
}

Token* Lexer::readOctoLiteral() {
  next(); // Skip 0
  next(); // Skip o

  int32 start = idx;

  while (isOctoChar(currentChar)) {
    next();
  }

  int32 end = idx;

  if (start == end) {
    throw std::runtime_error("Invalid oct sequence");
  }

  Token* tok = maketoken(TT_OCT_LITERAL);
  tok->valueStart = start;
  tok->valueEnd = end;

  return tok;
}

Token* Lexer::readBinaryLiteral() {
  next(); // Skip 0
  next(); // Skip b

  int32 start = idx;

  while (currentChar == '0' || currentChar == '1') {
    next();
  }

  int32 end = idx;

  if (start == end) {
    throw std::runtime_error("Invalid binary sequence");
  }

  Token* tok = maketoken(TT_BIN_LITERAL);
  tok->valueStart = start;
  tok->valueEnd = end;

  return tok;
}

tokentype keywordFromString(const std::string& id) {
  switch (id.length()) {
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