#include "objects.h"

#define ARRAY_INDEX_METHOD(type, shorthand) \
  type& QsArray::shorthand##At(const uint32 idx) {\
    return reinterpret_cast<type*>(data)[idx];\
  }

ARRAY_INDEX_METHOD(int8, i8)
ARRAY_INDEX_METHOD(uint8, u8)
ARRAY_INDEX_METHOD(int16, i16)
ARRAY_INDEX_METHOD(uint16, u16)
ARRAY_INDEX_METHOD(int32, i32)
ARRAY_INDEX_METHOD(uint32, u32)
ARRAY_INDEX_METHOD(int64, i64)
ARRAY_INDEX_METHOD(uint64, u64)
ARRAY_INDEX_METHOD(float32, f32)
ARRAY_INDEX_METHOD(float64, f64)

uint64 QsArray::address() const {
  return reinterpret_cast<uint64>(data - LENGTH_PREFIX_SIZE);
}

uint64 QsString::address() const {
  return reinterpret_cast<uint64>(characterData - LENGTH_PREFIX_SIZE);
}

QsArray castToQsArray(void* ptr) {
  if (!ptr) {
    return EMPTY_QS_ARRAY;
  }

  const uint32 len = *static_cast<uint32*>(ptr);
  return {
    .length = len,
    .data = static_cast<uint8*>(ptr) + LENGTH_PREFIX_SIZE
  };
}

QsString castToQsString(void* ptr) {
  if (!ptr) {
    return EMPTY_QS_STRING;
  }

  const uint32 len = *static_cast<uint32*>(ptr);
  return {
    .length = len,
    .characterData = static_cast<int8*>(ptr) + LENGTH_PREFIX_SIZE
  };
}

QsObject castToQsObject(void* ptr) {
  return {static_cast<uint8*>(ptr)};
}
