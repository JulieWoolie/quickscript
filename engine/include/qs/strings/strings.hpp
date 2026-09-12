#ifndef QS_STRINGS_H
#define QS_STRINGS_H
#include "qs/strings/utf8.hpp"

#define LF '\n'
#define CR '\r'

bool isNumeric(const utf32char ch);

bool isIdentifierStart(const utf32char ch);

bool isIdentifierPart(const utf32char ch);

bool isHexChar(const utf32char ch);

bool isWhitespace(const utf32char ch);

#endif //QS_STRINGS_H
