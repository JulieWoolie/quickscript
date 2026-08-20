#include "objects.h"

#define ARRAY_INDEX_METHOD(type, shorthand) \
  type& QsArray::shorthand##At(const uint32 idx) {\
    return reinterpret_cast<type*>(data)[idx];\
  }

#define OBJ_PROP_METHOD(type, shorthand) \
  type QsObject::get##shorthand##Property(const uint64 offset) const { \
    return *reinterpret_cast<type*>(data + offset + REFCOUNT_PREFIX_SIZE);\
  }\
  void QsObject::set##shorthand##Property(const uint64 offset, const type value) const { \
    *reinterpret_cast<type*>(data + offset + REFCOUNT_PREFIX_SIZE) = value;\
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

OBJ_PROP_METHOD(int8, I8)
OBJ_PROP_METHOD(uint8, U8)
OBJ_PROP_METHOD(int16, I16)
OBJ_PROP_METHOD(uint16, U16)
OBJ_PROP_METHOD(int32, I32)
OBJ_PROP_METHOD(uint32, U32)
OBJ_PROP_METHOD(int64, I64)
OBJ_PROP_METHOD(uint64, U64)
OBJ_PROP_METHOD(float32, F32)
OBJ_PROP_METHOD(float64, F64)

uint32& QsObject::referenceCount() const {
  return *reinterpret_cast<uint32*>(data);
}

uint64 QsArray::address() const {
  if (refCount) {
    return reinterpret_cast<uint64>(refCount);
  }
  return reinterpret_cast<uint64>(data - LENGTH_PREFIX_SIZE);
}

QsArray castToQsArray(void* ptr) {
  if (!ptr) {
    return EMPTY_QS_ARRAY;
  }

  uint32 len = 0;
  uint32* refCount = nullptr;
  uint8* data = nullptr;

  uint32* intPtr = static_cast<uint32*>(ptr);
  const uint32 firstInt = *intPtr;

  if (firstInt & REFCOUNT_MASK) {
    refCount = intPtr;
    len = intPtr[1];
    data = static_cast<uint8*>(ptr) + LENGTH_PREFIX_SIZE + REFCOUNT_PREFIX_SIZE;
  } else {
    refCount = nullptr;
    len = firstInt;
    data = static_cast<uint8*>(ptr) + LENGTH_PREFIX_SIZE;
  }

  return {
    .length = len,
    .refCount = refCount,
    .data = data
  };
}

QsObject castToQsObject(void* ptr) {
  return {static_cast<uint8*>(ptr)};
}
