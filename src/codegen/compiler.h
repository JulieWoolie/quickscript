#ifndef QUICKSCRIPT_COMPILER_H
#define QUICKSCRIPT_COMPILER_H

#include "../common.h"
#include "../interpreter/opcodes.h"
#include "../parse/syntaxtree.h"

typedef uint8 registerid;
typedef int8 registeridopt;

struct Bytecode {
  uint8* data = nullptr;
  uint64 len = 0;
};

struct ConstStringPoolWriter {
  uint8* data = nullptr;
  uint64 cap = 0;
  uint64 len = 0;
  std::unordered_map<stringid, uint64> idToPoolOffset;
};

struct BytecodeWriter {
  uint8* buf = nullptr;
  uint64 bufcap = 0;
  uint64 buflen = 0;

  void reserveSpace(uint64 memsize);

  void appendOpCode(opcode code);
  
  void appendU8(uint8 u8);
  void appendI8(int8 i8);
  void appendU16(uint16 u16);
  void appendI16(int16 i16);
  void appendU32(uint32 u32);
  void appendI32(int32 i32);
  void appendU64(uint64 u64);
  void appendI64(int64 i64);
  void appendF32(float32 f32);
  void appendF64(float64 f64);
  void appendPadding(uint64 bytes);
};

Bytecode compile(ScriptFileStatement* sfs);

#endif //QUICKSCRIPT_COMPILER_H
