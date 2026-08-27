#ifndef QUICKSCRIPT_LEXER_H
#define QUICKSCRIPT_LEXER_H

#include <iostream>

#include "../common.h"
#include "../stringtable.h"
#include "token.h"
#include "../errors.h"
#include "../strings/utf8.h"

#define COMMENT_CHAR '/'
#define STAR_CHAR '*'

class Lexer {
  private:
    int32 idx = EOF;
    uint32 line = 0;
    uint32 col = 0;

    utf32char currentChar = 0;

    std::string m_input;
    TokenList* m_tokens;

    StringTable* m_table;
    CompilerErrors* m_errors;

    Token* peekedToken = nullptr;
    Token* eofToken = nullptr;

    Location tokenStart;

    utf8char* readbuf = nullptr;
    uint32 readbufCap = 0;
    uint32 readbufLen = 0;

    bool ignoreComments = true;

  public:
    Lexer(const std::string& input, TokenList* m_tokens, StringTable* table, CompilerErrors* errors);
    ~Lexer();

    void setCommentsIgnored(bool ignored);

    void lex();

    utf32char peek(int32 ahead) const;
    utf32char peek() const;

    utf32char next();

    Token* peekToken();
    Token* nextToken();

    Location recordLocation() const;

  private:
    int8 getchar(int32 readIdx, utf32char* out) const;

    void skipEmptyContent();
    void skipLineComment();
    void skipBlockComment();

    Token* readToken();

    Token* readBlockComment();
    Token* readLineComment();

    Token* eoftoken();

    Token* token(tokentype ttype) const;
    Token* valueToken(tokentype ttype) const;

    Token* readIdOrKeyword();
    Token* readQuotedString();

    Token* readNumberLiteral();
    Token* readHexLiteral();
    Token* readOctoLiteral();
    Token* readBinaryLiteral();

    void readHexEscape();

    void clearReadBuf();
    void appendToReadBuf(utf32char ch);
    void appendToReadBuf();
    void ensureReadBufWriteable(uint32 characters);
};


#endif //QUICKSCRIPT_LEXER_H
