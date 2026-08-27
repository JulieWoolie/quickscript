#ifndef QUICKSCRIPT_STRINGS_H
#define QUICKSCRIPT_STRINGS_H
#include "utf8.h"

#define LF '\n'
#define CR '\r'

bool isNumeric(const utf32char ch);

bool isIdentifierStart(const utf32char ch);

bool isIdentifierPart(const utf32char ch);

bool isHexChar(const utf32char ch);

bool isWhitespace(const int8 ch);

#endif //QUICKSCRIPT_STRINGS_H
