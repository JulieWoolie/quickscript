
#include "token.h"
#include <stdexcept>

conststring tokentype_name(tokentype ttype) {
  switch (ttype) {
    case TT_EOF:
      return "END_OF_FILE";
    case TT_LCOMMENT:
      return "LINECOMMENT";
    case TT_BCOMMENT:
      return "BLOCKCOMMENT";

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
    case TT_THREE_DOTS:
      return "'...'";

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
    case TT_DWALL:
      return "'||'";
    case TT_DWALL_ASSIGN:
      return "'||='";
    case TT_QUESTION:
      return "'?'";

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
    case TT_KEYW_BOOL:
      return "'bool'";
    case TT_KEYW_UINT8:
      return "'uint8'";
    case TT_KEYW_INT8:
      return "'int8'";
    case TT_KEYW_UINT16:
      return "'uint16'";
    case TT_KEYW_INT16:
      return "'int16'";
    case TT_KEYW_UINT32:
      return "'uint32'";
    case TT_KEYW_INT32:
      return "'int32'";
    case TT_KEYW_UINT64:
      return "'uint64'";
    case TT_KEYW_INT64:
      return "'int64'";
    case TT_KEYW_FLOAT32:
      return "'float32'";
    case TT_KEYW_FLOAT64:
      return "'float64'";
    case TT_KEYW_STRING:
      return "'string'";
    case TT_KEYW_CONST:
      return "'const'";
    case TT_KEYW_VOID:
      return "'void'";

    default:
      return "UNKNOWN";
  }
}


uint32 TokenList::size() const {
  return items;
}

Token* TokenList::get(uint32 idx) {
  if (idx > items) {
    return nullptr;
  }
  return m_dataStart + idx;
}

Token* TokenList::newToken() {
  uint32 nidx = items;

  if (nidx >= capacity) {
    uint32 ncap = capacity + 100;
    Token* ndata = static_cast<Token *>(realloc(m_dataStart, sizeof(Token) * ncap));

    if (!ndata) {
      throw std::runtime_error("Failed to allocate more space for tokens");
    }

    m_dataStart = ndata;
    capacity = ncap;
  }

  items++;

  Token* tok = m_dataStart + nidx;

  tok->ttype = TT_UNKNOWN;

  tok->start.line = 0;
  tok->start.index = 0;
  tok->start.column = 0;

  tok->end.line = 0;
  tok->end.index = 0;
  tok->end.column = 0;

  tok->valueId = EMPTY_STRING;

  return tok;
}
