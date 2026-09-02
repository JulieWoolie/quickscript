#include "utf8.h"

#define IS_INVALID_CONTINUE_CODEUNIT(x) ((x & CONTINUE_TEST) != CONTINUE_MASK)

CodeUnitType getCodeUnitType(const utf8char ch) {
  if ((ch & ONEBYTE_TEST) == 0) {
    return CHT_1BYTE;
  }
  if ((ch & TWOBYTE_TEST) == TWOBYTE_HEAD) {
    return CHT_2BYTE;
  }
  if ((ch & THREEBYTE_TEST) == THREEBYTE_HEAD) {
    return CHT_3BYTE;
  }
  if ((ch & FOURBYTE_TEST) == FOURBYTE_HEAD) {
    return CHT_4BYTE;
  }
  if ((ch & CONTINUE_TEST) == CONTINUE_HEAD) {
    return CHT_CONTINUATION;
  }
  return CHT_INVALID;
}

uint8 getUtf8ByteLength(const utf32char ch) {
  if (ch <= MAX_1BYTE) {
    return 1;
  }
  if (ch <= MAX_2BYTE) {
    return 2;
  }
  if (ch <= MAX_3BYTE) {
    return 3;
  }
  if (ch <= MAX_4BYTE) {
    return 4;
  }
  return 0;
}

uint8 encodeToUtf8(const utf32char ch, utf8char* out) {
  if (ch <= MAX_1BYTE) {
    out[0] = ch;
    return 1;
  }
  if (ch <= MAX_2BYTE) {
    out[0] = TWOBYTE_HEAD  | (ch >> 6);
    out[1] = CONTINUE_HEAD | (ch & CONTINUE_MASK);
    return 2;
  }
  if (ch <= MAX_3BYTE) {
    out[0] = THREEBYTE_HEAD | (ch >> 12);
    out[1] = CONTINUE_HEAD  | ((ch >> 6) & CONTINUE_MASK);
    out[2] = CONTINUE_HEAD  | (ch & CONTINUE_MASK);
    return 3;
  }
  if (ch <= MAX_4BYTE) {
    out[0] = FOURBYTE_HEAD | (ch >> 18);
    out[1] = CONTINUE_HEAD | ((ch >> 12) & CONTINUE_MASK);
    out[2] = CONTINUE_HEAD | ((ch >> 6) & CONTINUE_MASK);
    out[3] = CONTINUE_HEAD | (ch & CONTINUE_MASK);
    return 4;
  }
  return 0;
}

uint8 decodeUtf8(const utf8char* in, utf32char* out, const uint32 len) {
  const utf8char first = in[0];
  const CodeUnitType type = getCodeUnitType(first);

  switch (type) {
    case CHT_4BYTE: {
      if (len < 4) {
        return 0;
      }

      const utf8char second = in[1];
      const utf8char third  = in[2];
      const utf8char fourth = in[3];

      if (IS_INVALID_CONTINUE_CODEUNIT(second)
        || IS_INVALID_CONTINUE_CODEUNIT(third)
        || IS_INVALID_CONTINUE_CODEUNIT(fourth)
      ) {
        return 0;
      }

      utf32char codepoint = ((first & ~FOURBYTE_HEAD) << 18)
           | ((second & CONTINUE_MASK) << 12)
           | ((third & CONTINUE_MASK) <<  6)
           | (fourth & CONTINUE_MASK);

      if (codepoint > MAX_4BYTE) {
        return 0;
      }

      *out = codepoint;
      return 4;
    }
    case CHT_3BYTE: {
      if (len < 3) {
        return 0;
      }

      const utf8char second = in[1];
      const utf8char third  = in[2];

      if (IS_INVALID_CONTINUE_CODEUNIT(second) || IS_INVALID_CONTINUE_CODEUNIT(third)) {
        return 0;
      }

      *out = ((first & ~THREEBYTE_HEAD) << 12)
           | ((second & CONTINUE_MASK) << 6)
           | (third & CONTINUE_MASK);

      return 3;
    }
    case CHT_2BYTE: {
      if (len < 2) {
        return 0;
      }

      const utf8char second = in[1];

      if (IS_INVALID_CONTINUE_CODEUNIT(second)) {
        return 0;
      }

      *out = ((first & ~TWOBYTE_HEAD) << 6)
           | (second & CONTINUE_MASK);

      return 2;
    }

    case CHT_1BYTE:
      *out = first;
      return 1;

    default:
      return 0;
  }
}

bool isValidUtf8String(const utf8char* buf, const uint32 len) {
  uint32 idx = 0;
  utf32char ch = 0;

  while (idx < len) {
    uint8 readBytes = decodeUtf8(buf + idx, &ch, len - idx);
    if (readBytes == 0) {
      return false;
    }
    idx += readBytes;
  }

  return true;
}
