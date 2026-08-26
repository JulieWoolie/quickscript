#ifndef QUICKSCRIPT_QSSTRINGS_H
#define QUICKSCRIPT_QSSTRINGS_H

#include "../common.h"

typedef uint8 utf8char;
typedef uint32 utf32char;

#define FOURBYTE_HEAD   0b11110000
#define THREEBYTE_HEAD  0b11100000
#define TWOBYTE_HEAD    0b11000000
#define CONTINUE_HEAD   0b10000000

#define CONTINUE_MASK 0b00111111

#define CONTINUE_TEST  0b11000000
#define FOURBYTE_TEST  0b11111000
#define THREEBYTE_TEST 0b11110000
#define TWOBYTE_TEST   0b11100000
#define ONEBYTE_TEST   0b10000000

#define MAX_1BYTE 0x7F
#define MAX_2BYTE 0x7FF
#define MAX_3BYTE 0xFFFF
#define MAX_4BYTE 0x10FFFF

#define IS_INVALID_CONTINUE_CODEUNIT(x) ((x & CONTINUE_TEST) != CONTINUE_MASK)

#define CHT_INVALID 0
#define CHT_1BYTE 1
#define CHT_2BYTE 2
#define CHT_3BYTE 3
#define CHT_4BYTE 4
#define CHT_CONTINUATION 5
typedef uint8 CodeUnitType;

CodeUnitType getCodeUnitType(utf8char ch);

uint8 getUtf8ByteLength(utf32char ch);

uint8 encodeToUtf8(utf32char ch, utf8char* out);

uint8 decodeUtf8(const utf8char* in, utf32char* out, uint32 len);

#endif //QUICKSCRIPT_QSSTRINGS_H
