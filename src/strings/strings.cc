#include "strings.h"

bool isNumeric(const utf32char ch) {
  return ch >= '0' && ch <= '9';
}

bool isIdentifierStart(const utf32char ch) {
  return (ch >= 'a' && ch <= 'z')
      || (ch >= 'A' && ch <= 'Z')
      || ch == '_'
      || ch == '$';
}

bool isIdentifierPart(const utf32char ch) {
  return isIdentifierStart(ch)
      || isNumeric(ch);
}

bool isHexChar(const utf32char ch) {
  return (ch >= '0' && ch <= '9')
      || (ch >= 'a' && ch <= 'f')
      || (ch >= 'A' && ch <= 'F');
}

bool isWhitespace(const int8 ch) {
  return ch == ' '
      || ch == '\t'
      || ch == LF
      || ch == CR;
}