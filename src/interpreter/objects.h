#ifndef QUICKSCRIPT_OBJECTS_H
#define QUICKSCRIPT_OBJECTS_H

#include "../common.h"

#define LENGTH_PREFIX_SIZE 4
#define EMPTY_QS_ARRAY QsArray(0, nullptr)
#define EMPTY_QS_STRING QsString(0, nullptr)

static_assert(sizeof(uint32) == LENGTH_PREFIX_SIZE);

struct QsArray {
  const uint32 length = 0;
  uint8* data = nullptr;

  uint64 address() const;

  int8& i8At(uint32 idx);
  uint8& u8At(uint32 idx);
  int16& i16At(uint32 idx);
  uint16& u16At(uint32 idx);
  int32& i32At(uint32 idx);
  uint32& u32At(uint32 idx);
  int64& i64At(uint32 idx);
  uint64& u64At(uint32 idx);
  float32& f32At(uint32 idx);
  float64& f64At(uint32 idx);
};

struct QsString {
  const uint32 length = 0;
  int8* characterData = nullptr;

  uint64 address() const;
};

struct QsObject {
  uint8* data = nullptr;

  int8 getI8Property(uint64 offset);
  uint8 getU8Property(uint64 offset);
  int16 getI16Property(uint64 offset);
  uint16 getU16Property(uint64 offset);
  int32 getI32Property(uint64 offset);
  uint32 getU32Property(uint64 offset);
  int64 getI64Property(uint64 offset);
  uint64 getU64Property(uint64 offset);
  float32 getF32Property(uint64 offset);
  float64 getF64Property(uint64 offset);
};

QsArray castToQsArray(void* ptr);

QsString castToQsString(void* ptr);

QsObject castToQsObject(void* ptr);

#endif //QUICKSCRIPT_OBJECTS_H
