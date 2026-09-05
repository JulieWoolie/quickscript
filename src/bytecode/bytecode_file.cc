#include "bytecode_file.h"

#include <vector>

#include "../interpreter/opcodes.h"
#include "../interpreter/opcode_printer.h"
#include "../strings/utf8.h"

#define CREATE_WRITE_METHOD(name, type) \
  void name(type x) {\
    ensureHasSpace(sizeof(type));\
    *reinterpret_cast<type*>(buf + len) = x;\
    len += sizeof(type);\
  }\
  void name##At(type x, uint64 off) {\
    *reinterpret_cast<type*>(buf + off) = x;\
  }

#define CREATE_READ_METHOD(name, type) \
  type name() {\
    if (!hasRemaining(sizeof(type))) {\
      return 0;\
    }\
    type value = *reinterpret_cast<const type*>(buffer + cursor);\
    cursor += sizeof(type);\
    return value;\
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
    memcpy(buf + len, from, bytes);
    len += bytes;
  }
};

struct BinaryReader {
  const uint8* buffer;
  const uint64 capacity;

  uint64 cursor = 0;

  bool hasRemaining(const uint64 bytes = 1) const {
    return (cursor + bytes) <= capacity;
  }

  CREATE_READ_METHOD(readU8, uint8)
  CREATE_READ_METHOD(readU16, uint16)
  CREATE_READ_METHOD(readU32, uint32)
  CREATE_READ_METHOD(readU64, uint64)

  BinaryReader subReader(const uint64 start, const uint64 size) {
    return BinaryReader(buffer + start, size, 0);
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
        writer.writeU64(t->heapSize);
        writer.writeU8(t->alignment);
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
    writer.writeU64(te->stackSize);
    writer.writeU32(te->startingInstruction);
    writer.writeU32(te->signatureIndex);
  }
}

static void writeInstructions(const BytecodeFile& file, BinaryWriter& writer) {
  uint64 off = 0;
  while (off < file.instructionsSize) {
    const opcode code = *reinterpret_cast<opcode*>(file.instructionBuf + off);
    const uint8 len = getInstructionLength(code);

    writer.copyDataFrom(file.instructionBuf + off, len);

    off += LENGTH_INSTRUCTION;
  }
}

BytecodeFile::BytecodeFile() {

}

BytecodeFile::~BytecodeFile() {
  if (constStringPool) {
    free(constStringPool);
    constStringPool = nullptr;
    stringPoolSize = 0;
  }

  if (typeTable) {
    for (uint32 i = 0; i < typeTableSize; i++) {
      TypeTableEntry* e = typeTable[i];
      freeTypeTableEntry(e);
    }

    freeTypeTable(typeTable);
    typeTable = nullptr;
    typeTableSize = 0;
  }

  if (funcTable) {
    freeFunctionTableArray(funcTable);
    funcTable = nullptr;
    funcTableEntries = 0;
  }

  if (instructionBuf) {
    free(instructionBuf);
  }

  instructionBuf = nullptr;
  instructionsSize = 0;
  instructionCount = 0;

  globalScopeSize = 0;
  entryPointIndex = 0;
}


BytecodeFile& BytecodeFile::create() {
  BytecodeFile* bfile = new BytecodeFile();
  return *bfile;
}

void BytecodeFile::destroy(const BytecodeFile& bfile) {
  delete &bfile;
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
  writeInstructions(file, writer);
  const uint64 instrSize = writer.len - instrStart;

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
  sections[HSECT_INSTR_COUNT] = file.instructionCount;
  sections[HSECT_GLOBAL_SCOPE_SIZE] = file.globalScopeSize;
  sections[HSECT_ENTRYPOINT_FUNC_IDX] = file.entryPointIndex;

  *sizeOut = finalLength;
  return writer.buf;
}

static bool hasCorrectFilePrefix(const BinaryReader& reader) {
  if (!reader.hasRemaining(PREFIX_LEN)) {
    return false;
  }

  constexpr conststring str = FILE_PREFIX;

  for (uint32 i = 0; i < PREFIX_LEN; i++) {
    if (reader.buffer[i] != str[i]) {
      return false;
    }
  }

  return true;
}

static BytecodeReadResult readStringPool(BinaryReader& reader, BytecodeFile& out) {
  while (reader.hasRemaining(4)) {
    const uint32 len = reader.readU32();

    if (!reader.hasRemaining(len)) {
      return IR_RESULT_MALFORMED_STRING_POOL;
    }

    const uint8* strBuf = reader.buffer + reader.cursor;

    if (!isValidUtf8String(strBuf, len)) {
      return IR_RESULT_MALFORMED_STRING_POOL;
    }

    reader.cursor += len;
  }

  uint8* strPoolBuf = static_cast<uint8*>(malloc(reader.capacity));
  if (!strPoolBuf) {
    return IR_RESULT_STRING_POOL_ALLOC_FAILED;
  }

  memcpy(strPoolBuf, reader.buffer, reader.capacity);

  out.stringPoolSize = reader.capacity;
  out.constStringPool = strPoolBuf;

  return IR_RESULT_OK;
}

static BytecodeReadResult readTypeTable(BinaryReader& reader, BytecodeFile& out) {
  std::vector<TypeTableEntry*> entries;

  while (reader.hasRemaining(5)) {
    const uint32 typeIndex = reader.readU32();
    const TypeTableType entryType = reader.readU8();

    if (typeIndex <= LAST_RESERVED_TYPE_INDEX) {
      // Free the entries so we don't create a memory leak
      for (TypeTableEntry* e : entries) {
        freeTypeTableEntry(e);
      }
      return IR_RESULT_MALFORMED_TYPETABLE;
    }

    switch (entryType) {
      case TYPE_TABLE_ARRAY: {
        const uint32 componentTypeIndex = reader.readU32();

        TypeTableArray* arr = TypeTableArray::create();
        arr->index = typeIndex;
        arr->componentType = componentTypeIndex;

        entries.push_back(arr);
        break;
      }
      case TYPE_TABLE_STRUCT: {
        const uint64 nameOffset = reader.readU64();
        const uint64 heapSize = reader.readU64();
        const uint64 alignment = reader.readU8();
        const uint32 ctorFuncIdx = reader.readU32();
        const uint32 propCount = reader.readU32();

        TypeTableStruct* entry = TypeTableStruct::create(propCount);
        entry->index = typeIndex;
        entry->nameOffset = nameOffset;
        entry->constructorFuncIndex = ctorFuncIdx;
        entry->propertyCount = propCount;
        entry->alignment = alignment;
        entry->heapSize = heapSize;

        for (uint32 i = 0; i < propCount; i++) {
          TypeTableStructProperty* prop = &entry->properties[i];
          prop->nameOffset = reader.readU64();
          prop->valueOffset = reader.readU32();
          prop->type = reader.readU32();
        }

        entries.push_back(entry);
        break;
      }
      case TYPE_TABLE_SIGNATURE: {
        const uint32 returnTypeIdx = reader.readU32();
        const uint32 argCount = reader.readU32();
        const bool variadic = reader.readU8();

        TypeTableFuncSign* entry = TypeTableFuncSign::create(argCount);
        entry->index = typeIndex;
        entry->returnType = returnTypeIdx;
        entry->argumentCount = argCount;
        entry->varargs = variadic;

        for (uint32 i = 0; i < argCount; i++) {
          entry->argTypes[i] = reader.readU32();
        }

        entries.push_back(entry);
        break;
      }
      default:
        // Free the entries so we don't create a memory leak
        for (TypeTableEntry* e : entries) {
          freeTypeTableEntry(e);
        }
        return IR_RESULT_MALFORMED_TYPETABLE;
    }
  }

  const uint32 entryCount = entries.size();
  TypeTableEntry** typeTable = createTypeTable(entryCount);

  for (uint32 i = 0; i < entryCount; i++) {
    typeTable[i] = entries[i];
  }

  out.typeTable = typeTable;
  out.typeTableSize = entryCount;

  return IR_RESULT_OK;
}

static BytecodeReadResult readFunctionTable(BinaryReader& reader, BytecodeFile& out) {
  constexpr uint64 funcTableEntrySize = 8 + 8 + 4 + 4;
  const uint32 entryCount = reader.capacity / funcTableEntrySize;

  FunctionTableEntry* table = createFunctionTableArray(entryCount);

  for (uint32 i = 0; i < entryCount; i++) {
    const uint64 nameOff = reader.readU64();
    const uint64 stackSize = reader.readU64();
    const uint32 firstInstr = reader.readU32();
    const uint32 signatureIdx = reader.readU32();

    table[i] = {
      .nameOffset = nameOff,
      .signatureIndex = signatureIdx,
      .startingInstruction = firstInstr,
      .stackSize = stackSize
    };
  }

  out.funcTable = table;
  out.funcTableEntries = entryCount;

  return IR_RESULT_OK;
}

static BytecodeReadResult unpackInstructions(BinaryReader& reader, const uint64 instrCount, BytecodeFile& out) {
  const uint64 memSize = instrCount * LENGTH_INSTRUCTION;

  uint8* instrBuf = static_cast<uint8*>(malloc(memSize));
  uint64 instrWriteOffset = 0;

  memset(instrBuf, 0, memSize);

  for (uint32 i = 0; i < instrCount; i++) {
    const uint8* start = reader.buffer + reader.cursor;
    const opcode code = *reinterpret_cast<const opcode*>(start);

    const uint8 instrLength = getInstructionLength(code);

    if (instrLength == 0) {
      free(instrBuf);
      return IR_RESULT_MALFORMED_INSTRUCTIONS;
    }

    memcpy(instrBuf + instrWriteOffset, start, instrLength);
    instrWriteOffset += LENGTH_INSTRUCTION;
    reader.cursor += instrLength;
  }

  out.instructionCount = instrCount;
  out.instructionsSize = memSize;
  out.instructionBuf = instrBuf;

  return IR_RESULT_OK;
}

BytecodeReadResult deserializeBytecodeFile(const uint8* buf, const uint64 bufSize, BytecodeFile& out) {
  BinaryReader reader = {
    .buffer = buf,
    .capacity = bufSize,
    .cursor = 0
  };

  if (!hasCorrectFilePrefix(reader)) {
    return IR_RESULT_INVALID_PREFIX;
  }

  reader.cursor = PREFIX_LEN;

  const uint16 fileVersion = reader.readU16();
  if (fileVersion > CURRENT_FILE_VERSION) {
    return IR_RESULT_FILE_VERSION_NEWER;
  }

  const uint64* sections = reinterpret_cast<const uint64*>(reader.buffer + PREFIX_LEN + HEADER_VERSION_SIZE);
  const uint64 strPoolStart = sections[HSECT_STRPOOL_OFF];
  const uint64 strPoolSize = sections[HSECT_STRPOOL_SIZE];
  const uint64 typeTableStart = sections[HSECT_TYPES_OFF];
  const uint64 typeTableSize = sections[HSECT_TYPES_SIZE];
  const uint64 fTableStart = sections[HSECT_FTABLE_OFF];
  const uint64 fTableSize = sections[HSECT_FTABLE_SIZE];
  const uint64 instrStart = sections[HSECT_INSTR_OFF];
  const uint64 instrSize = sections[HSECT_INSTR_SIZE];
  const uint64 instructionCount = sections[HSECT_INSTR_COUNT];
  const uint64 globalScopeSize = sections[HSECT_GLOBAL_SCOPE_SIZE];
  const uint64 entryPointIndex = sections[HSECT_ENTRYPOINT_FUNC_IDX];

  out.entryPointIndex = entryPointIndex;
  out.globalScopeSize = globalScopeSize;

  // Read string pool
  BinaryReader strPoolReader = reader.subReader(strPoolStart, strPoolSize);
  const BytecodeReadResult strPoolResult = readStringPool(strPoolReader, out);
  if (strPoolResult != IR_RESULT_OK) {
    return strPoolResult;
  }

  // Read type table
  BinaryReader typeTableReader = reader.subReader(typeTableStart, typeTableSize);
  const BytecodeReadResult typeTableResult = readTypeTable(typeTableReader, out);
  if (typeTableResult != IR_RESULT_OK) {
    return typeTableResult;
  }

  BinaryReader funcTableReader = reader.subReader(fTableStart, fTableSize);
  const BytecodeReadResult fTableResult = readFunctionTable(funcTableReader, out);
  if (fTableResult != IR_RESULT_OK) {
    return fTableResult;
  }

  BinaryReader instrReader = reader.subReader(instrStart, instrSize);
  const BytecodeReadResult instrResult = unpackInstructions(instrReader, instructionCount, out);
  if (instrResult != IR_RESULT_OK) {
    return instrResult;
  }

  return IR_RESULT_OK;
}

conststring getReadResultMessage(const BytecodeReadResult res) {
  switch (res) {
    case IR_RESULT_OK: return "OK";
    case IR_RESULT_INVALID_PREFIX: return "Invalid file prefix";
    case IR_RESULT_FILE_VERSION_NEWER: return "Unsupported file version";
    case IR_RESULT_STRING_POOL_ALLOC_FAILED: return "Failed to allocate string pool buffer";
    case IR_RESULT_MALFORMED_STRING_POOL: return "Malformed string pool data";
    case IR_RESULT_MALFORMED_TYPETABLE: return "Malformed type table data";
    case IR_RESULT_MALFORMED_INSTRUCTIONS: return "Malformed IR instructions data";
    default: return "Unknown";
  }
}

static void writePooledString(FILE* out, uint64 off, uint8* strPool) {
  const uint32 len = *reinterpret_cast<uint32*>(strPool + off);
  conststring strData = reinterpret_cast<conststring>(strPool + off + sizeof(uint32));
  fwrite(strData, 1, len, out);
}

static conststring TypeTableType_name(const TypeTableType type) {
  switch (type) {
    case TYPE_TABLE_ARRAY: return "ARRAY";
    case TYPE_TABLE_SIGNATURE: return "FUNCSIGN";
    case TYPE_TABLE_STRUCT: return "STRUCT";
    default: return "";
  }
}

void printBytecodeFile(const BytecodeFile& file, FILE* printFile) {
  fprintf(printFile, "FILE_VERSION = %d", CURRENT_FILE_VERSION);
  fprintf(printFile, "\nGLOBAL_SCOPE_SIZE = %llu", file.globalScopeSize);
  fprintf(printFile, "\nENTRYPOINT_FUNCTION_INDEX = %llu", file.entryPointIndex);

  uint8* stringPool = file.constStringPool;
  const uint64 stringPoolSize = file.stringPoolSize;
  uint64 strPoolOff = 0;

  fprintf(printFile, "\nCONST_STRING_POOL = {");
  while (strPoolOff < stringPoolSize) {
    const uint32 strSize = *reinterpret_cast<uint32*>(stringPool + strPoolOff);
    fprintf(printFile, "\n  [off=%llu size=%d]: ", strPoolOff, strSize);
    writePooledString(printFile, strPoolOff, stringPool);
    strPoolOff += strSize + sizeof(uint32);
  }
  fprintf(printFile, "\n}");

  const uint64 typeTableSize = file.typeTableSize;
  TypeTableEntry** typeTable = file.typeTable;

  fprintf(printFile, "\nTYPE_TABLE = {");
  for (uint32 i = 0; i < typeTableSize; i++) {
    TypeTableEntry* entry = typeTable[i];
    fprintf(printFile, "\n  [%llu] {\n    type = %s", entry->index, TypeTableType_name(entry->type));

    switch (entry->type) {
      case TYPE_TABLE_STRUCT: {
        TypeTableStruct* str = static_cast<TypeTableStruct*>(entry);
        fprintf(printFile, "\n    name_offset = %llu # ", str->nameOffset);
        writePooledString(printFile, str->nameOffset, stringPool);
        fprintf(printFile, "\n    alignment = %d", str->alignment);
        fprintf(printFile, "\n    heap_size = %llu", str->heapSize);
        fprintf(printFile, "\n    constructor_entry = %d", str->constructorFuncIndex);
        fprintf(printFile, "\n    properties = [");

        for (uint32 pIdx = 0; pIdx < str->propertyCount; pIdx++) {
          TypeTableStructProperty* prop = &str->properties[pIdx];

          if (pIdx != 0) {
            fprintf(printFile, ",");
          }

          fprintf(printFile, "\n      {");
          fprintf(printFile, "\n        name_offset = %llu # ", prop->nameOffset);
          writePooledString(printFile, prop->nameOffset, stringPool);

          fprintf(printFile, "\n        value_offset = %llu", prop->valueOffset);
          fprintf(printFile, "\n        type_index = ");
          printTypeIndex(printFile, prop->type);

          fprintf(printFile, "\n      }");
        }

        if (str->propertyCount != 0) {
          fprintf(printFile, "\n    ");
        }

        fprintf(printFile, "]");
        break;
      }
      case TYPE_TABLE_SIGNATURE: {
        TypeTableFuncSign* fSign = static_cast<TypeTableFuncSign*>(entry);
        fprintf(printFile, "\n    return_type = ");
        printTypeIndex(printFile, fSign->returnType);

        fprintf(printFile, "\n    variadic = %d", fSign->varargs);
        fprintf(printFile, "\n    arguments = [");
        for (uint32 aIdx = 0; aIdx < fSign->argumentCount; aIdx++) {
          if (aIdx != 0) {
            fprintf(printFile, ", ");
          }
          printTypeIndex(printFile, fSign->argTypes[aIdx]);
        }
        fprintf(printFile, "]");
        break;
      }
      case TYPE_TABLE_ARRAY: {
        TypeTableArray* arr = static_cast<TypeTableArray*>(entry);
        fprintf(printFile, "\n    component_type = ");
        printTypeIndex(printFile, arr->componentType);
        break;
      }
    }

    fprintf(printFile, "\n  }");
  }
  fprintf(printFile, "\n}");

  const uint32 funcTableSize = file.funcTableEntries;
  FunctionTableEntry* funcTable = file.funcTable;

  fprintf(printFile, "\nFUNCTION_TABLE = {");
  for (uint32 i = 0; i < funcTableSize; i++) {
    FunctionTableEntry* entry = &funcTable[i];
    fprintf(printFile, "\n  [%d] {", i);
    fprintf(printFile, "\n    name_offset = %llu # ", entry->nameOffset);
    writePooledString(printFile, entry->nameOffset, stringPool);

    fprintf(printFile, "\n    signature_index = %llu", entry->signatureIndex);
    fprintf(printFile, "\n    first_instruction = %llu", entry->startingInstruction);
    fprintf(printFile, "\n    stack_size = %llu", entry->stackSize);
    fprintf(printFile, "\n  }");
  }
  fprintf(printFile, "\n}");

  fprintf(printFile, "\nINSTRUCTIONS = {");

  uint8* instrBuf = file.instructionBuf;
  const uint64 instrSize = file.instructionsSize;
  uint64 instrOff = 0;
  uint32 instrCount = 0;

  while (instrOff < instrSize) {
    uint8* buf = instrBuf + instrOff;

    for (uint32 fTableIdx = 0; fTableIdx < funcTableSize; fTableIdx++) {
      const FunctionTableEntry* entry = &funcTable[fTableIdx];
      if (entry->startingInstruction != instrCount) {
        continue;
      }
      if (instrCount != 0) {
        fprintf(printFile, "\n");
      }
      fprintf(printFile, "\n  # FUNCTION START: ");
      writePooledString(printFile, entry->nameOffset, stringPool);
      break;
    }

    fprintf(printFile, "\n  [%4d] ", instrCount);
    printInstructionToString(buf, printFile, stringPool);

    instrOff += LENGTH_INSTRUCTION;
    instrCount++;
  }
  fprintf(printFile, "\n}");
}
