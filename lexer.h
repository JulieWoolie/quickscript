#ifndef QUICKSCRIPT_LEXER_H
#define QUICKSCRIPT_LEXER_H

#include <iostream>

#include "common.h"

#define COMMENT_CHAR '/'
#define STAR_CHAR '*'
#define LF '\n'
#define CR '\r'

// ======= Token Types (TT) =======

// Special types
#define TT_UNKNOWN              0000
#define TT_EOF                  (-1)

// IDs
#define TT_ID                   1001

// Single chars
#define TT_LBRACKET             2000
#define TT_RBRACKET             2001
#define TT_LCURL                2002
#define TT_RCURL                2003
#define TT_LSQUARE              2004
#define TT_RSQUARE              2005
#define TT_COLON                2006
#define TT_SEMICOLON            2007
#define TT_COMMA                2008
#define TT_DOT                  2009

// Operators
#define TT_BIT_AND              3000
#define TT_BIT_AND_ASSIGN       3001
#define TT_LOGICAL_AND          3002
#define TT_LOGICAL_AND_ASSIGN   3003
#define TT_WALL                 3004
#define TT_WALL_ASSIGN          3005
#define TT_STAR                 3006
#define TT_STAR_ASSIGN          3007
#define TT_SLASH                3008
#define TT_SLASH_ASSIGN         3009
#define TT_PLUS                 3010
#define TT_PLUS_ASSIGN          3011
#define TT_MINUS                3012
#define TT_MINUS_ASSIGN         3013
#define TT_XOR                  3014
#define TT_XOR_ASSIGN           3015
#define TT_PERCENT              3016
#define TT_PERCENT_ASSIGN       3017
#define TT_SHL                  3018
#define TT_SHL_ASSIGN           3019
#define TT_SHR                  3020
#define TT_SHR_ASSIGN           3021
#define TT_USHR                 3022
#define TT_USHR_ASSIGN          3023
#define TT_LT                   3024
#define TT_LTE                  3025
#define TT_GT                   3026
#define TT_GTE                  3027
#define TT_INC                  3028
#define TT_DEC                  3029
#define TT_EQ                   3030
#define TT_NEQ                  3031
#define TT_ASSIGN               3032
#define TT_INVERT               3033
#define TT_BIT_INVERT           3034
#define TT_LAMBDA_ARROW         3035
#define TT_POW                  3036
#define TT_POW_ASSIGN           3037

// Literals
#define TT_HEX_LITERAL          4000
#define TT_OCT_LITERAL          4001
#define TT_BIN_LITERAL          4002
#define TT_INT_LITERAL          4003
#define TT_FLOAT_LITERAL        4004
#define TT_STRING_LITERAL       4005
#define TT_CHAR_LITERAL         4006

// Keywords
#define TT_KEYW_TRUE            5000
#define TT_KEYW_FALSE           5001
#define TT_KEYW_NULL            5002
#define TT_KEYW_IF              5003
#define TT_KEYW_ELSE            5004
#define TT_KEYW_BREAK           5005
#define TT_KEYW_CONTINUE        5006
#define TT_KEYW_RETURN          5007
#define TT_KEYW_WHILE           5008
#define TT_KEYW_FOR             5009
#define TT_KEYW_FUNCTION        5010
#define TT_KEYW_STRUCT          5011
#define TT_KEYW_MODULE          5012
#define TT_KEYW_IMPORT          5013
#define TT_KEYW_DO              5014

// =====================

typedef int16 tokentype;

struct Location {
  int32 index = EOF;
  uint32 line = 0;
  uint32 column = 0;
};

struct Token {
  tokentype ttype = TT_UNKNOWN;

  Location start = {};
  Location end = {};

  int32 valueStart = -1;
  int32 valueEnd = -1;
};

conststring tokentype_name(tokentype ttype);

tokentype keywordFromString(const std::string& id);

class TokenList {
  Token* m_dataStart = nullptr;
  uint32 capacity = 0;
  uint32 items = 0;

  public:
    TokenList() = default;

    uint32 size() const;

    Token* get(uint32 idx);

    Token* newToken();
};

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
