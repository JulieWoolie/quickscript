#ifndef QUICKSCRIPT_IR_FILE_H
#define QUICKSCRIPT_IR_FILE_H

#include "bytecode_functable.h"
#include "bytecode_typetable.h"
#include "../common.h"
#include "../types/types.h"

#define IR_RESULT_OK 0
#define IR_RESULT_INVALID_PREFIX 1
#define IR_RESULT_FILE_VERSION_NEWER 2
#define IR_RESULT_STRING_POOL_ALLOC_FAILED 3
#define IR_RESULT_MALFORMED_STRING_POOL 4
#define IR_RESULT_MALFORMED_TYPETABLE 5
#define IR_RESULT_MALFORMED_INSTRUCTIONS 6
typedef uint32 BytecodeReadResult;

#define FILE_PREFIX "quickscript"
#define PREFIX_LEN 11
#define HEADER_SECTION_SIZE 8
#define HEADER_VERSION_SIZE 2
#define HEADER_SECTIONS 11
#define HEADER_LEN (PREFIX_LEN + HEADER_VERSION_SIZE + (HEADER_SECTION_SIZE * HEADER_SECTIONS))

#define HSECT_STRPOOL_OFF 0
#define HSECT_STRPOOL_SIZE 1
#define HSECT_TYPES_OFF 2
#define HSECT_TYPES_SIZE 3
#define HSECT_FTABLE_OFF 4
#define HSECT_FTABLE_SIZE 5
#define HSECT_INSTR_OFF 6
#define HSECT_INSTR_SIZE 7
#define HSECT_INSTR_COUNT 8
#define HSECT_GLOBAL_SCOPE_SIZE 9
#define HSECT_ENTRYPOINT_FUNC_IDX 10

#define CURRENT_FILE_VERSION 0

struct BytecodeFile {
  uint8* constStringPool = nullptr;
  uint64 stringPoolSize = 0;

  TypeTableEntry** typeTable = nullptr;
  uint64 typeTableSize = 0;

  FunctionTableEntry* funcTable = nullptr;
  uint32 funcTableEntries = 0;
  uint64 entryPointIndex = 0;

  uint8* instructionBuf = nullptr;
  uint64 instructionsSize = 0;
  uint64 instructionCount = 0;

  uint64 globalScopeSize = 0;

  BytecodeFile();
  ~BytecodeFile();

  static BytecodeFile& create();

  static void destroy(const BytecodeFile& bfile);
};

uint8* serializeBytecodeFile(const BytecodeFile& file, uint64* sizeOut);

BytecodeReadResult deserializeBytecodeFile(const uint8* buf, uint64 bufSize, BytecodeFile& out);

conststring getReadResultMessage(BytecodeReadResult res);

void printBytecodeFile(const BytecodeFile& file, FILE* printFile);

#endif //QUICKSCRIPT_IR_FILE_H
