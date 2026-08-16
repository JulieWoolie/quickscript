#include "ir_file.h"

#define CREATE_WRITE_METHOD(name, type) \
  void name(type x) {\
    ensureHasSpace(sizeof(type));\
    *reinterpret_cast<type*>(buf + len) = x;\
    len += sizeof(type);\
  }\
  void name##At(type x, uint64 off) {\
    *reinterpret_cast<type*>(buf + off) = x;\
  }

struct BinaryWriter {
  uint8* buf;
  uint64 len = 0;
  uint64 cap = 0;

  void ensureHasSpace(const uint64 bytes) {
    const uint64 reqSize = len + bytes;
    if (reqSize <= cap) {
      return;
    }

    const uint64 newCap = reqSize + 1024;
    uint8* newBuf = static_cast<uint8*>(realloc(buf, newCap));

    if (!newBuf) {
      throw std::runtime_error("Failed to grow buffer");
    }

    buf = newBuf;
    cap = newCap;
  }

  CREATE_WRITE_METHOD(writeU64, uint64)
  CREATE_WRITE_METHOD(writeU32, uint32)
  CREATE_WRITE_METHOD(writeU16, uint16)
  CREATE_WRITE_METHOD(writeU8, uint8)

  void copyDataFrom(const void* from, uint64 bytes) {
    ensureHasSpace(bytes);
    memcpy(buf, from, bytes);
    len += bytes;
  }
};

static void writeTypeTable(const BytecodeFile& file, BinaryWriter& writer) {
  TypeTableEntry** table = file.typeTable;
  uint64 tableEntries = file.typeTableSize;

  for (uint32 i = 0; i < tableEntries; i++) {
    TypeTableEntry* entry = table[i];
    const TypeTableType entryType = entry->type;

    writer.writeU32(entry->index);
    writer.writeU8(entryType);

    switch (entryType) {
      case TYPE_TABLE_STRUCT: {
        TypeTableStruct* t = static_cast<TypeTableStruct*>(entry);
        writer.writeU64(t->nameOffset);
        writer.writeU32(t->constructorFuncIndex);
        writer.writeU32(t->propertyCount);
        for (uint32 propIdx = 0; propIdx < t->propertyCount; propIdx++) {
          TypeTableStructProperty* prop = &t->properties[propIdx];
          writer.writeU64(prop->nameOffset);
          writer.writeU32(prop->valueOffset);
          writer.writeU32(prop->type);
        }
        break;
      }
      case TYPE_TABLE_ARRAY: {
        const TypeTableArray* arr = static_cast<TypeTableArray*>(entry);
        writer.writeU32(arr->componentType);
        break;
      }
      case TYPE_TABLE_SIGNATURE: {
        const TypeTableFuncSign* funcSign = static_cast<const TypeTableFuncSign*>(entry);
        writer.writeU32(funcSign->returnType);
        writer.writeU32(funcSign->argumentCount);
        writer.writeU8(funcSign->varargs);
        for (uint32 argIdx = 0; argIdx < funcSign->argumentCount; argIdx++) {
          writer.writeU32(funcSign->argTypes[argIdx]);
        }
      }
    }
  }
}

static void writeFunctionTable(const BytecodeFile& file, BinaryWriter& writer) {
  const uint32 funcTableSize = file.funcTableEntries;
  FunctionTableEntry* table = file.funcTable;

  for (uint32 i = 0; i < funcTableSize; i++) {
    FunctionTableEntry* te = &table[i];
    writer.writeU64(te->nameOffset);
    writer.writeU32(te->startingInstruction);
    writer.writeU32(te->signatureIndex);
  }
}

uint8* serializeBytecodeFile(const BytecodeFile& file, uint64* sizeOut) {
  BinaryWriter writer;
  writer.len = 0;

  const uint64 initialCap = HEADER_LEN + file.instructionsSize + file.stringPoolSize + 1024;
  writer.buf = static_cast<uint8*>(malloc(initialCap));
  writer.cap = initialCap;

  if (!writer.cap) {
    *sizeOut = 0;
    return nullptr;
  }

  // First write the entire file, then write the header
  writer.len = HEADER_LEN;

  const uint64 strPoolStart = writer.len;
  const uint64 strPoolSize = file.stringPoolSize;

  // Const String Pool
  writer.copyDataFrom(file.constStringPool, strPoolSize);

  // Type Table
  const uint64 typeTableStart = writer.len;
  writeTypeTable(file, writer);
  const uint64 typeTableSize = writer.len - typeTableStart;

  // Func Table
  const uint64 fTableStart = writer.len;
  writeFunctionTable(file, writer);
  const uint64 fTableSize = writer.len - fTableStart;

  // Instructions Buffer
  const uint64 instrStart = writer.len;
  writer.copyDataFrom(file.instructionBuf, file.instructionsSize);
  const uint64 instrSize = file.instructionsSize;

  const uint64 finalLength = writer.len;

  // Header
  writer.len = 0;
  writer.copyDataFrom(FILE_PREFIX, PREFIX_LEN);
  writer.writeU16(CURRENT_FILE_VERSION);

  uint64* sections = reinterpret_cast<uint64*>(writer.buf + writer.len);
  sections[HSECT_STRPOOL_OFF] = strPoolStart;
  sections[HSECT_STRPOOL_SIZE] = strPoolSize;
  sections[HSECT_TYPES_OFF] = typeTableStart;
  sections[HSECT_TYPES_SIZE] = typeTableSize;
  sections[HSECT_FTABLE_OFF] = fTableStart;
  sections[HSECT_FTABLE_SIZE] = fTableSize;
  sections[HSECT_INSTR_OFF] = instrStart;
  sections[HSECT_INSTR_SIZE] = instrSize;

  *sizeOut = finalLength;
  return writer.buf;
}
