#ifndef QUICKSCRIPT_LEXER_H
#define QUICKSCRIPT_LEXER_H

#include <iostream>

#include "../common.h"
#include "../stringtable.h"
#include "token.h"

#define COMMENT_CHAR '/'
#define STAR_CHAR '*'
#define LF '\n'
#define CR '\r'

class Lexer {
  private:
    int32 idx = EOF;
    uint32 line = 0;
    uint32 col = 0;

    int8 currentChar = 0;

    std::string m_input;
    TokenList* m_tokens;

    StringTable* m_table;

    Token* peekedToken = nullptr;
    Token* eofToken = nullptr;

    Location tokenStart;

    int8* readbuf = nullptr;
    uint32 readbufCap = 0;
    uint32 readbufLen = 0;

    bool ignoreComments = true;

  public:
    Lexer(const std::string& input, TokenList* m_tokens, StringTable* table);

    void setCommentsIgnored(bool ignored);

    void lex();

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

    Token* readBlockComment();
    Token* readLineComment();

    Token* eoftoken();

    Token* maketoken(tokentype ttype);
    Token* maketokenv(tokentype ttype);

    Token* readIdOrKeyword();
    Token* readQuotedString();

    Token* readNumberLiteral();
    Token* readHexLiteral();
    Token* readOctoLiteral();
    Token* readBinaryLiteral();

    void readHexEscape();

    void clearReadBuf();
    void appendToReadBuf(int8 ch);
    void appendToReadBuf();
    void ensureReadBufWriteable(uint32 characters);
};


#endif //QUICKSCRIPT_LEXER_H
