
#include "keyw_lookup.h"

#include <cstring>

/* C++ code produced by gperf version 3.0.1 */
/* Command-line: gperf -t --readonly-tables qs-keywords.gperf  */
/* Computed positions: -k'1,$' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif

#line 1 "qs-keywords.gperf"

struct keyword {
    const char *name;
    tokentype token;
};
#line 10 "qs-keywords.gperf"
struct keyword;

#define TOTAL_KEYWORDS 40
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 8
#define MIN_HASH_VALUE 2
#define MAX_HASH_VALUE 60
/* maximum key range = 59, duplicates = 0 */

class Perfect_Hash
{
private:
  static inline unsigned int hash (const char *str, unsigned int len);
public:
  static const struct keyword *keyword_lookup (const char *str, unsigned int len);
};

inline unsigned int
Perfect_Hash::hash (const char *str, unsigned int len)
{
  static const unsigned char asso_values[] =
    {
      61, 61, 61, 61, 61, 61, 61, 61, 61, 61,
      61, 61, 61, 61, 61, 61, 61, 61, 61, 61,
      61, 61, 61, 61, 61, 61, 61, 61, 61, 61,
      61, 61, 61, 61, 61, 61, 61, 61, 61, 61,
      61, 61, 61, 61, 61, 61, 61, 61, 61, 61,
      35, 61, 10, 61, 30, 61, 45, 61, 61, 61,
      61, 61, 61, 61, 61, 61, 61, 61, 61, 61,
      61, 61, 61, 61, 61, 61, 61, 61, 61, 61,
      61, 61, 61, 61, 61, 61, 61, 61, 61, 61,
      61, 61, 61, 61, 61, 61, 61, 61, 35,  5,
       0,  5,  5, 30, 61,  5, 61, 20,  0, 40,
       0,  0, 61, 61,  0, 10, 15,  0, 61, 45,
      61, 61, 61, 61, 61, 61, 61, 61, 61, 61,
      61, 61, 61, 61, 61, 61, 61, 61, 61, 61,
      61, 61, 61, 61, 61, 61, 61, 61, 61, 61,
      61, 61, 61, 61, 61, 61, 61, 61, 61, 61,
      61, 61, 61, 61, 61, 61, 61, 61, 61, 61,
      61, 61, 61, 61, 61, 61, 61, 61, 61, 61,
      61, 61, 61, 61, 61, 61, 61, 61, 61, 61,
      61, 61, 61, 61, 61, 61, 61, 61, 61, 61,
      61, 61, 61, 61, 61, 61, 61, 61, 61, 61,
      61, 61, 61, 61, 61, 61, 61, 61, 61, 61,
      61, 61, 61, 61, 61, 61, 61, 61, 61, 61,
      61, 61, 61, 61, 61, 61, 61, 61, 61, 61,
      61, 61, 61, 61, 61, 61, 61, 61, 61, 61,
      61, 61, 61, 61, 61, 61
    };
  return len + asso_values[(unsigned char)str[len - 1]] + asso_values[(unsigned char)str[0]];
}

const struct keyword *
Perfect_Hash::keyword_lookup (const char *str, unsigned int len)
{
  static const unsigned char lengthtable[] =
    {
       0,  0,  2,  0,  4,  5,  6,  0,  3,  4,  5,  6,  2,  8,
       4,  5,  6,  0,  8,  4,  5,  6,  7,  3,  4,  5,  6,  0,
       0,  0,  5,  6,  0,  0,  4,  5,  6,  0,  0,  4,  5,  6,
       7,  0,  4,  5,  6,  7,  0,  0,  5,  6,  0,  0,  4,  5,
       0,  0,  0,  0,  5
    };
  static const struct keyword wordlist[] =
    {
      {""}, {""},
#line 33 "qs-keywords.gperf"
      {"do", TT_KEYW_DO},
      {""},
#line 21 "qs-keywords.gperf"
      {"null", TT_KEYW_NULL},
#line 39 "qs-keywords.gperf"
      {"uchar", TT_KEYW_UINT8},
#line 26 "qs-keywords.gperf"
      {"return", TT_KEYW_RETURN},
      {""},
#line 28 "qs-keywords.gperf"
      {"for", TT_KEYW_FOR},
#line 38 "qs-keywords.gperf"
      {"char", TT_KEYW_INT8},
#line 37 "qs-keywords.gperf"
      {"ubyte", TT_KEYW_UINT8},
#line 47 "qs-keywords.gperf"
      {"double", TT_KEYW_FLOAT64},
#line 22 "qs-keywords.gperf"
      {"if", TT_KEYW_IF},
#line 29 "qs-keywords.gperf"
      {"function", TT_KEYW_FUNCTION},
#line 23 "qs-keywords.gperf"
      {"else", TT_KEYW_ELSE},
#line 20 "qs-keywords.gperf"
      {"false", TT_KEYW_FALSE},
#line 54 "qs-keywords.gperf"
      {"uint64", TT_KEYW_UINT64},
      {""},
#line 25 "qs-keywords.gperf"
      {"continue", TT_KEYW_CONTINUE},
#line 43 "qs-keywords.gperf"
      {"uint", TT_KEYW_UINT32},
#line 55 "qs-keywords.gperf"
      {"int64", TT_KEYW_INT64},
#line 41 "qs-keywords.gperf"
      {"ushort", TT_KEYW_UINT16},
#line 57 "qs-keywords.gperf"
      {"float64", TT_KEYW_FLOAT64},
#line 42 "qs-keywords.gperf"
      {"int", TT_KEYW_INT32},
#line 19 "qs-keywords.gperf"
      {"true", TT_KEYW_TRUE},
#line 46 "qs-keywords.gperf"
      {"float", TT_KEYW_FLOAT32},
#line 32 "qs-keywords.gperf"
      {"import", TT_KEYW_IMPORT},
      {""}, {""}, {""},
#line 40 "qs-keywords.gperf"
      {"short", TT_KEYW_INT16},
#line 30 "qs-keywords.gperf"
      {"struct", TT_KEYW_STRUCT},
      {""}, {""},
#line 44 "qs-keywords.gperf"
      {"long", TT_KEYW_INT64},
#line 45 "qs-keywords.gperf"
      {"ulong", TT_KEYW_UINT64},
#line 50 "qs-keywords.gperf"
      {"uint16", TT_KEYW_UINT16},
      {""}, {""},
#line 34 "qs-keywords.gperf"
      {"bool", TT_KEYW_BOOL},
#line 51 "qs-keywords.gperf"
      {"int16", TT_KEYW_INT16},
#line 52 "qs-keywords.gperf"
      {"uint32", TT_KEYW_UINT32},
#line 35 "qs-keywords.gperf"
      {"boolean", TT_KEYW_BOOL},
      {""},
#line 36 "qs-keywords.gperf"
      {"byte", TT_KEYW_INT8},
#line 53 "qs-keywords.gperf"
      {"int32", TT_KEYW_INT32},
#line 58 "qs-keywords.gperf"
      {"string", TT_KEYW_STRING},
#line 56 "qs-keywords.gperf"
      {"float32", TT_KEYW_FLOAT32},
      {""}, {""},
#line 48 "qs-keywords.gperf"
      {"uint8", TT_KEYW_UINT8},
#line 31 "qs-keywords.gperf"
      {"module", TT_KEYW_MODULE},
      {""}, {""},
#line 49 "qs-keywords.gperf"
      {"int8", TT_KEYW_INT8},
#line 27 "qs-keywords.gperf"
      {"while", TT_KEYW_WHILE},
      {""}, {""}, {""}, {""},
#line 24 "qs-keywords.gperf"
      {"break", TT_KEYW_BREAK}
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        if (len == lengthtable[key])
          {
            const char *s = wordlist[key].name;

            if (*str == *s && !memcmp (str + 1, s + 1, len - 1))
              return &wordlist[key];
          }
    }
  return 0;
}


tokentype tokenTypeFromString(conststring str, uint32 len) {
  const keyword *kw = Perfect_Hash::keyword_lookup(str, len);
  if (!kw) {
    return TT_UNKNOWN;
  }
  return kw->token;
}
