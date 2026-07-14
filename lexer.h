#ifndef QUICKSCRIPT_LEXER_H
#define QUICKSCRIPT_LEXER_H

#include <iostream>

#include "common.h"
#include "token.h"

#define COMMENT_CHAR '/'
#define STAR_CHAR '*'
#define LF '\n'
#define CR '\r'

tokentype keywordFromString(const std::string& id);

class Lexer {
  private:
    int32 idx = EOF;
    uint32 line = 0;
    uint32 col = 0;

    int8 currentChar = 0;

    std::string m_input;
    TokenList* m_tokens;

    Token* peekedToken = nullptr;
    Token* eofToken = nullptr;

    Location tokenStart;

  public:
    Lexer(const std::string& input, TokenList* m_tokens);

    int8 peek(int32 ahead);
    int8 peek();

    int8 next();

    Token* peekToken();
    Token* nextToken();

    Location recordLocation();

  private:
    int8 getchar(int32 idx);

    void advanceLineTracker();

    void skipEmptyContent();
    void skipLineComment();
    void skipBlockComment();

    Token* readToken();

    Token* eoftoken();

    Token* maketoken(tokentype ttype);

    Token* readIdOrKeyword();
    Token* readQuotedString();

    Token* readNumberLiteral();
    Token* readHexLiteral();
    Token* readOctoLiteral();
    Token* readBinaryLiteral();

    void readHexEscape();
};


#endif //QUICKSCRIPT_LEXER_H
