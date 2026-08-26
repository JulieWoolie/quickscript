#include "objects.h"

#define ARRAY_INDEX_METHOD(type, shorthand) \
  type QsArray::get##shorthand(const uint32 idx) const {\
    return reinterpret_cast<type*>(data)[idx];\
  }\
  void QsArray::set##shorthand(const uint32 idx, const type value) const {\
    reinterpret_cast<type*>(data)[idx] = value;\
  }

#define OBJ_PROP_METHOD(type, shorthand) \
  type QsObject::get##shorthand##Property(const uint64 offset) const { \
    return *reinterpret_cast<type*>(data + offset + REFCOUNT_PREFIX_SIZE);\
  }\
  void QsObject::set##shorthand##Property(const uint64 offset, const type value) const { \
    *reinterpret_cast<type*>(data + offset + REFCOUNT_PREFIX_SIZE) = value;\
  }

ARRAY_INDEX_METHOD(int8, I8)
ARRAY_INDEX_METHOD(uint8, U8)
ARRAY_INDEX_METHOD(int16, I16)
ARRAY_INDEX_METHOD(uint16, U16)
ARRAY_INDEX_METHOD(int32, I32)
ARRAY_INDEX_METHOD(uint32, U32)
ARRAY_INDEX_METHOD(int64, I64)
ARRAY_INDEX_METHOD(uint64, U64)
ARRAY_INDEX_METHOD(float32, F32)
ARRAY_INDEX_METHOD(float64, F64)

uint64 QsObject::address() const {
  return reinterpret_cast<uint64>(data);
}

uint32 QsObject::getRefCount() const {
  return *reinterpret_cast<uint32*>(data) & ~REFCOUNT_MASK;
}

void QsObject::setRefCount(const uint32 count) const {
  *reinterpret_cast<uint32*>(data) = count | REFCOUNT_MASK;
}

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

uint64 QsArray::address() const {
  if (refCount) {
    return reinterpret_cast<uint64>(refCount);
  }
  return reinterpret_cast<uint64>(data - LENGTH_PREFIX_SIZE);
}

uint32 QsArray::getRefCount() const {
  if (!refCount) {
    return NO_REFCOUNT;
  }
  return *refCount & ~REFCOUNT_MASK;
}

void QsArray::setRefCount(const uint32 count) const {
  if (!refCount) {
    return;
  }
  *refCount = count | REFCOUNT_MASK;
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

uint32 readQsArrayLength(void* ptr) {
  if (!ptr) {
    return 0;
  }

  const uint32* intPtr = static_cast<uint32*>(ptr);
  const uint32 first = *intPtr;

  if (first & REFCOUNT_MASK) {
    return intPtr[1];
  }

  return first;
}
