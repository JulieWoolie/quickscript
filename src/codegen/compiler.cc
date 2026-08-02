#include "compiler.h"

#include <stdarg.h>

#include "opcodespec.h"
#include "../interpreter/opcodes.h"

#define BYTEWIDTH_OPCODE(size, writer, opcode) \
  switch (size) {\
    case 1: writer->appendOpCode(opcode##8); break;\
    case 2: writer->appendOpCode(opcode##16); break;\
    case 4: writer->appendOpCode(opcode##32); break;\
    default: writer->appendOpCode(opcode##64); break;\
  }

#define NUMTYPE_OPCODE(primkind, writer, opcode) \
  switch (primkind) { \
    case PK_BOOL:\
    case PK_UINT8: writer->appendOpCode(opcode##U8); break; \
    case PK_INT8: writer->appendOpCode(opcode##I8); break; \
    case PK_UINT16: writer->appendOpCode(opcode##U16); break; \
    case PK_INT16: writer->appendOpCode(opcode##I16); break; \
    case PK_UINT32: writer->appendOpCode(opcode##U32); break; \
    case PK_INT32: writer->appendOpCode(opcode##I32); break; \
    case PK_UINT64: writer->appendOpCode(opcode##U64); break; \
    case PK_INT64: writer->appendOpCode(opcode##I64); break; \
    case PK_FLOAT32: writer->appendOpCode(opcode##F32); break; \
    default: writer->appendOpCode(opcode##F64); break; \
  }

#define BIN_APPEND(pad) \
      writer->appendU8(r1);\
      writer->appendU8(r2);\
      writer->appendU8(r1);\
      writer->appendPadding(pad);

#define CMP_CASE(cmptype) \
  case BOP_##cmptype: \
    if (lkind == TK_PRIMITIVE) { \
      NUMTYPE_OPCODE(pk, writer, OP_##cmptype) \
    } else { \
      writer->appendOpCode(OP_##cmptype##ARR); \
    } \
    writer->appendU8(r1); \
    writer->appendU8(r2); \
    writer->appendU8(r1); \
    writer->appendPadding(PAD_##cmptype); \
    break;

#define EQUALITY_CASE(type)\
  case BOP_##type:\
    if (lkind == TK_PRIMITIVE) {\
      BYTEWIDTH_OPCODE(ltype->stackSizeBytes(), writer, OP_##type)\
    } else if (lkind == TK_ARRAY || lkind == TK_STRING) {\
      writer->appendOpCode(OP_##type##ARR);\
    } else if (lkind == TK_STRUCT) {\
      writer->appendOpCode(OP_##type##STRUCT);\
    }\
    BIN_APPEND(PAD_##type)\
    break;

#define MATH_CASE(type)\
  case BOP_##type:\
    NUMTYPE_OPCODE(pk, writer, OP_##type)\
    BIN_APPEND(PAD_##type)\
    break;

#define ALL_REGISTERS_USED 0xFFFFFFFFFFFFFFFF
#define NO_REGISTER -1

#define SSYM_NIL 0
#define SSYM_VAR 1
#define SSYM_FUNC 2

typedef uint8 StackSymType;

struct StackSymbol {
  stringid name = EMPTY_STRING;
  StackSymType type = SSYM_NIL;
  uint64 stackoffset = 0;
  uint64 stacksize = 0;
};

struct StackScope {
  std::vector<StackSymbol> symbols;
  uint64 stacksize = 0;
  uint32 level = 0;

  StackSymbol* pushSymbol(stringid name, StackSymType symType) {
    uint64 off = 0;
    for (const StackSymbol& sym : symbols) {
      off += sym.stacksize;
    }
    return &symbols.emplace_back(name, symType, off, 0);
  }
};

struct CompiledFunction {
  FunctionSignature* signature = nullptr;
  uint32 bodystart = 0;
  uint64 nameOffset = 0;
};

struct CompilerContext {
  std::vector<ScriptType*> expectedType;
  std::vector<StackScope> scopes;

  std::unordered_map<ScriptStructType*, uint32> declaredStructConstructors;
  std::vector<ScriptStructType*> declaredStructs;

  std::vector<CompiledFunction> functions;
  std::vector<FunctionDeclStatement*> funcQueue;

  BytecodeWriter* writer = nullptr;
  ConstStringPoolWriter* stringPool = nullptr;
  StringTable* stringTable = nullptr;
  TypeLookup* types = nullptr;

  uint64* registersInUse = nullptr;

  uint32 pushCompiledFunction(FunctionSignature* sign, uint32 start, uint64 nameOff) {
    uint32 idx = functions.size();
    functions.emplace_back(sign, start, nameOff);
    return idx;
  }

  StackScope* getScope() {
    return &scopes.back();
  }

  StackScope* pushScope() {
    StackScope scope = StackScope();
    scope.level = scopes.size();
    scopes.push_back(scope);
    return &scopes.back();
  }

  void popScope() {
    scopes.pop_back();
  }

  std::pair<StackScope*, StackSymbol*> findSymbol(stringid id, StackSymType symType) {
    for (StackScope& scope : scopes) {
      for (StackSymbol& sym : scope.symbols) {
        if (sym.name != id || sym.type != symType) {
          continue;
        }
        return {&scope, &sym};
      }
    }
    return {nullptr, nullptr};
  }

  uint64 emplaceConstString(stringid id) {
    std::unordered_map<stringid, uint64>* map = &stringPool->idToPoolOffset;

    if (map->contains(id)) {
      return map->at(id);
    }

    int32 len = stringTable->getlen(id);
    uint64 requiredcap = len + sizeof(uint32) + stringPool->len;

    if (requiredcap > stringPool->cap) {
      uint8* ndata = static_cast<uint8*>(malloc(requiredcap));
      if (!ndata) {
        throw std::runtime_error("Failed to resize string const pool buffer");
      }

      stringPool->data = ndata;
      stringPool->cap = requiredcap;
    }

    uint8* writeptr = stringPool->data + stringPool->len;

    *reinterpret_cast<uint32*>(writeptr) = len;
    writeptr += sizeof(uint32);

    char* charptr = reinterpret_cast<char*>(writeptr);
    stringTable->copychars(id, charptr, len);

    uint64 off = stringPool->len;
    stringPool->len += sizeof(uint32) + len;
    map->emplace(id, off);

    return off;
  }

  registeridopt acquireRegister() const {
    uint64 used = *registersInUse;

    if (used == ALL_REGISTERS_USED) {
      return NO_REGISTER;
    }
    if (used == 0) {
      return 0;
    }

    uint64 m;
    for (int8 i = 0; i < 64; i++) {
      m = 1 << i;
      if (m & used) {
        continue;
      }

      *registersInUse |= m;
      return i;
    }

    return NO_REGISTER;
  }

  bool registerInUse(const registerid id) const {
    uint64 m = 1L << id;
    return *registersInUse & m;
  }

  void useRegister(const registerid id) const {
    *registersInUse |= (1L << id);
  }

  void freeRegister(const registerid id) const {
    *registersInUse &= ~(1L << id);
  }
};

#define OUTP_NIL    0
#define OUTP_ASTACK 1
#define OUTP_RSTACK 2
#define OUTP_PROP   3
#define OUTP_IDX    4

struct AddrOutput {
  uint8 outptype = OUTP_NIL;
  registeridopt objectRegister = NO_REGISTER;
  registeridopt indexRegister = NO_REGISTER;
  uint32 memoffset = 0;
  int64 stackoffset = 0;
};

#define APPEND_METHOD(name, bytes, type) \
  void BytecodeWriter::name(type v) {\
    reserveSpace(bytes);\
    type* ptr = (type*) (buf + buflen);\
    *ptr = v;\
    buflen += bytes;\
  }

APPEND_METHOD(appendU8, 1, uint8)
APPEND_METHOD(appendI8, 1, int8)
APPEND_METHOD(appendU16, 2, uint16)
APPEND_METHOD(appendI16, 2, int16)
APPEND_METHOD(appendU32, 4, uint32)
APPEND_METHOD(appendI32, 4, int32)
APPEND_METHOD(appendU64, 8, uint64)
APPEND_METHOD(appendI64, 8, int64)
APPEND_METHOD(appendF32, 4, float32)
APPEND_METHOD(appendF64, 8, float64)

void BytecodeWriter::reserveSpace(uint64 memsize) {
  uint64 nsize = buflen + memsize;
  if (nsize < bufcap) {
    return;
  }

  const uint64 ncap = bufcap + 1024;
  uint8* ndata = static_cast<uint8*>(realloc(buf, ncap));

  if (!ndata) {
    throw std::runtime_error("Failed to grow bytecode buffer");
  }

  buf = ndata;
  bufcap = ncap;
}

void BytecodeWriter::appendOpCode(opcode code) {
  reserveSpace(LENGTH_INSTRUCTION);
  opcode* ptr = (opcode*) (buf + buflen);
  *ptr = code;
  buflen += LENGTH_OPCODE;
}

void BytecodeWriter::appendPadding(uint64 bytes) {
  if (!bytes) {
    return;
  }

  reserveSpace(bytes);
  memset(buf + buflen, 0, bytes);
  buflen += bytes;
}

void BytecodeWriter::appendInstruction(opcode code, ...) {
  va_list list;
  va_start(list, code);

  reserveSpace(LENGTH_INSTRUCTION);

  uint8* d = buf + buflen;
  *((opcode*) d) = code;

  d += LENGTH_OPCODE;

  appendOpCodeData(d, code, list);
  va_end(list);
}

void BytecodeWriter::writeInstructionCounter(const uint64 offset) const {
  uint32* ptr = reinterpret_cast<uint32*>(buf + offset);
  *ptr = getInstructionCounter();
}

uint32 BytecodeWriter::getInstructionCounter() const {
  return buflen / LENGTH_INSTRUCTION;
}

void compileWriteOperation(
  AddrOutput* addrout,
  uint32 stacksize,
  CompilerContext* ctx,
  registerid valueRegister
) {
  BytecodeWriter* writer = ctx->writer;

  switch (addrout->outptype) {
    case OUTP_IDX:
      BYTEWIDTH_OPCODE(stacksize, writer, OP_WRITEIDX)
      writer->appendU8(addrout->objectRegister);
      writer->appendU8(valueRegister);
      writer->appendU8(addrout->indexRegister);
      writer->appendPadding(PAD_WRITEIDX);
      break;
    case OUTP_PROP:
      BYTEWIDTH_OPCODE(stacksize, writer, OP_WRITEOBJ)
      writer->appendU8(addrout->objectRegister);
      writer->appendU8(valueRegister);
      writer->appendU32(addrout->memoffset);
      writer->appendPadding(PAD_WRITEOBJ);
      break;
    case OUTP_RSTACK:
      BYTEWIDTH_OPCODE(stacksize, writer, OP_RSWRITE)
      writer->appendU8(valueRegister);
      writer->appendU64(addrout->stackoffset);
      break;
    case OUTP_ASTACK:
      BYTEWIDTH_OPCODE(stacksize, writer, OP_ASWRITE)
      writer->appendU8(valueRegister);
      writer->appendU64(addrout->stackoffset);
      break;
    default:
      break;
  }

  if (addrout->objectRegister != NO_REGISTER) {
    ctx->freeRegister(addrout->objectRegister);
  }
  if (addrout->indexRegister != NO_REGISTER) {
    ctx->freeRegister(addrout->indexRegister);
  }
}

void compileExpr(Expr* expr, registerid resultreg, AddrOutput* addr, CompilerContext* ctx);

void compileIndexAccessExpr(
  IndexAccessExpr* access,
  registeridopt resultreg,
  AddrOutput* addr,
  CompilerContext* ctx
) {
  registerid targetreg = ctx->acquireRegister();
  registerid idxreg = ctx->acquireRegister();

  compileExpr(access->target, targetreg, nullptr, ctx);
  uint32 stackSize = access->resultType->stackSizeBytes();

  if (resultreg != NO_REGISTER) {
    BytecodeWriter* writer = ctx->writer;
    BYTEWIDTH_OPCODE(stackSize, writer, OP_READIDX)
    writer->appendU8(targetreg);
    writer->appendU8(resultreg);
    writer->appendU8(idxreg);
    writer->appendPadding(6);
  }

  if (addr) {
    addr->outptype = OUTP_IDX;
    addr->objectRegister = targetreg;
    addr->indexRegister = idxreg;
  } else {
    ctx->freeRegister(targetreg);
    ctx->freeRegister(idxreg);
  }
}

void compilePropertyAccess(
  PropertyAccessExpr* prop,
  registeridopt resultreg,
  AddrOutput* addr,
  CompilerContext* ctx
) {
  ScriptType* type = prop->target->resultType;

  typekind targetkind = type->kind();
  registerid targetreg = ctx->acquireRegister();

  compileExpr(prop->target, targetreg, nullptr, ctx);

  uint32 propoff;

  if (targetkind != TK_STRUCT) {
    // Only non struct property that is available is the length property on
    // arrays and strings which is always at offset 0x0
    propoff = 0;
  } else {
    std::string_view view = ctx->stringTable->getview(prop->property->value);
    ScriptStructType* structType = static_cast<ScriptStructType*>(type);

    propoff = 0;

    for (uint32 p = 0; p < structType->propertyCount; p++) {
      StructProperty* prop = &structType->properties[p];

      if (prop->propertyName != view) {
        propoff += prop->type->stackSizeBytes();
        continue;
      }

      break;
    }
  }

  if (resultreg != NO_REGISTER) {
    BytecodeWriter* writer = ctx->writer;
    writer->appendOpCode(OP_READOBJ32);
    writer->appendU8(targetreg);
    writer->appendU8(resultreg);
    writer->appendU8(propoff);
  }

  if (addr) {
    addr->outptype = OUTP_PROP;
    addr->objectRegister = targetreg;
    addr->memoffset = propoff;
  } else {
    ctx->freeRegister(targetreg);
  }
}

void compileIdentifier(
  Identifier* id,
  registeridopt resultreg,
  AddrOutput* addr,
  CompilerContext* ctx
) {
  StackSymType type = SSYM_NIL;
  BytecodeWriter* writer = ctx->writer;

  if (id->resultType->kind() == TK_FUNC) {
    type = SSYM_FUNC;
  } else {
    type = SSYM_VAR;
  }

  std::pair<StackScope*, StackSymbol*> pair = ctx->findSymbol(id->value, type);
  StackScope* scope = pair.first;
  StackSymbol* sym = pair.second;

  StackScope* current = ctx->getScope();

  // Variable declared in current scope
  if (scope == current) {
    if (resultreg != NO_REGISTER) {
      BYTEWIDTH_OPCODE(sym->stacksize, writer, OP_RSREAD)
      writer->appendU8(resultreg);
      writer->appendI64(sym->stackoffset);
    }

    if (addr) {
      addr->outptype = OUTP_RSTACK;
      addr->stackoffset = sym->stackoffset;
    }

    return;
  }

  // TODO: Figure out how to read from upper levels of scopes
  //
  //   So I think the way this needs to be done (reading from upper scopes) depends on
  //   which variable or constant is being referenced. If we're referencing a global
  //   variable declared in the main scope of the script, then we use ASREAD (Absolute
  //   stack read) which takes in a memory offset relative to the start of the script's
  //   main scope, or to read RSREAD with a negative number if not.
  //
  //   The thing is, a calling function can't reference anything from a caller's stack
  //   with this methodology because the function doesn't know anything in the above
  //   scope exists unless this is a function declared within a function, in which
  //   case... damn, that might cause issues.
  //
  //   Nested functions will expect the stack to be offset from where it was declared,
  //   but it can be declared near the top of the encasing function but be called at
  //   the end, so the stack can be different. Basically, the negative pointer needs
  //   to read NOT from the current scope but from a saved stack address... I think?
  //
  //   A nested function could also be called from within a loop, in which case,
  //   the stack will again be different because of fucking course it will be, loop's
  //   cause stack alloc calls and stack free calls meaning the stack pointer will
  //   change
  //
  //   This is s confusing
  //

  // Main scope, aka, a global variable
  if (scope->level == 0) {
    if (resultreg != NO_REGISTER) {
      BYTEWIDTH_OPCODE(sym->stacksize, writer, OP_ASREAD)
      writer->appendU8(resultreg);
      writer->appendU64(sym->stackoffset);
    }

    if (addr) {
      addr->outptype = OUTP_RSTACK;
      addr->stackoffset = sym->stackoffset;
    }

    return;
  }
}

void compileNonReadingExpr(Expr* expr, AddrOutput* addr, CompilerContext* ctx) {
  astnodetype kind = expr->nodeKind();
  switch (kind) {
    case AST_PropertyAccessExpr: {
      compilePropertyAccess(static_cast<PropertyAccessExpr*>(expr), NO_REGISTER, addr, ctx);
      return;
    }
    case AST_IndexAccessExpr: {
      compileIndexAccessExpr(static_cast<IndexAccessExpr*>(expr), NO_REGISTER, addr, ctx);
      return;
    }
    case AST_Identifier: {
      compileIdentifier(static_cast<Identifier*>(expr), NO_REGISTER, addr, ctx);
      return;
    }
    default:
      break;
  }
}

void compileBinaryExpr(BinaryExpr* bin, registerid r1, AddrOutput* addr, CompilerContext* ctx) {
  BytecodeWriter* writer = ctx->writer;
  binaryop bop = bin->op;
  binaryop nonAssign = bop & ~BOP_ASSIGN_FLAG;

  //
  // == How this should work ==
  //
  // r1 = result register
  //
  // 1. General operations:
  //   - Compile LHS with r1 as it's return register
  //   - Allocate register, named "r2"
  //   - Compile RHS with r2 as it's return register
  //   - Add IR instructions for performing operation itself with r1 and r2, output to r1
  //
  // The following operations should run in the above specified way:
  //   - Comparison operations:
  //       GT, LT, GTE, LTE
  //   - Equality checks:
  //       EQ, NEQ
  //   - Math:
  //       ADD, SUB, MUL, DIV, MOD, POW
  //   - Bitwise operations:
  //       SHL, SHR, USHR, XOR, BIT_OR, BIT_AND
  //
  // 2. Non-assignment boolean logic:
  //
  // This includes 2 operations, "OR" and "AND", these should run like this:
  //
  // AND Operation execution:
  //   - Compile LHS with r1 as its register
  //   - IF FALSE, jump to expression end
  //   - Compile RHS with r1 as its register
  //
  // OR Operation execution:
  //   - Compile LHS with r1 as its register
  //   - IF TRUE, jump to expression end
  //   - Compile RHS with r1 as its register
  //
  // 3. Assignment operation (only applies to the ASSIGN operation):
  //  - Compile a read-only version of LHS (Without reading the value, just get it's location)
  //  - Allocate a register, named "r2"
  //  - Compile RHS with r2 as its return register
  //  - Assign the value in r2 to the location of the value returned by LHS
  //
  // This needs to somehow read the address of the LHS without evaluating it, which I haven't
  // written a way to do yet, so that's interesting.
  //
  // 4. Mixed evaluate-and-assign operations (Basically +=, -=, *=, etc):
  //   - Set up an AddrOutput
  //   - Compile LHS, RHS and evaluate as mentioned in sections 1 and 2
  //   - Assign value to LHS' location
  //

  Expr* lhs = bin->lhs;
  Expr* rhs = bin->rhs;

  if (bop == BOP_ASSIGN) {
    AddrOutput out;
    compileNonReadingExpr(lhs, &out, ctx);

    compileExpr(rhs, r1, nullptr, ctx);

    uint32 stacksize = rhs->resultType->stackSizeBytes();
    compileWriteOperation(&out, stacksize, ctx, r1);

    return;
  }

  bool isAssignment = bop & BOP_ASSIGN_FLAG;
  AddrOutput out;

  if (nonAssign == BOP_LOG_AND || nonAssign == BOP_LOG_OR) {
    compileExpr(lhs, r1, &out, ctx);

    if (nonAssign == BOP_LOG_AND) {
      writer->appendOpCode(OP_JMPI0);
    } else {
      writer->appendOpCode(OP_JMPN0);
    }

    uint64 jumpAddrOffset = writer->buflen;

    writer->appendU32(0);
    writer->appendU8(r1);

    compileExpr(rhs, r1, &out, ctx);

    writer->writeInstructionCounter(jumpAddrOffset);

    if (isAssignment) {
      compileWriteOperation(&out, 1, ctx, r1);
    }

    return;
  }

  registerid r2 = ctx->acquireRegister();

  compileExpr(lhs, r2, &out, ctx);
  compileExpr(rhs, r1, nullptr, ctx);

  ScriptType* ltype = lhs->resultType;
  ScriptType* rtype = rhs->resultType;

  typekind lkind = ltype->kind();
  primitivekind pk = PK_NIL;

  if (lkind == TK_PRIMITIVE) {
    pk = static_cast<PrimitiveScriptType*>(ltype)->primtype;
  }

  switch (nonAssign) {
    case BOP_SHL:
      BYTEWIDTH_OPCODE(ltype->stackSizeBytes(), writer, OP_LSHIFT)
      BIN_APPEND(PAD_LSHIFT)
      break;
    case BOP_SHR:
    case BOP_USHR:
      BYTEWIDTH_OPCODE(ltype->stackSizeBytes(), writer, OP_RSHIFT)
      BIN_APPEND(PAD_RSHIFT)
      break;
    case BOP_BIT_OR:
      writer->appendOpCode(OP_BOR);
      BIN_APPEND(PAD_BOR)
      break;
    case BOP_BIT_AND:
      writer->appendOpCode(OP_BAND);
      BIN_APPEND(PAD_BAND)
      break;
    case BOP_XOR:
      if (pk == PK_BOOL) {
        writer->appendOpCode(OP_BXOR);
      } else {
        writer->appendOpCode(OP_LXOR);
      }
      BIN_APPEND(PAD_BXOR)
      break;

    case BOP_ADD:
      if (lkind == TK_STRING) {
        writer->appendOpCode(OP_STRCONCAT);
      } else {
        NUMTYPE_OPCODE(pk, writer, OP_ADD)
      }
      BIN_APPEND(PAD_ADD)
      break;
    case BOP_MUL:
      if (lkind == TK_STRING) {
        BYTEWIDTH_OPCODE(rtype->stackSizeBytes(), writer, OP_STRREP)
      } else {
        NUMTYPE_OPCODE(pk, writer, OP_ADD)
      }
      BIN_APPEND(PAD_ADD)
      break;

    EQUALITY_CASE(EQ)
    EQUALITY_CASE(NEQ)

    MATH_CASE(SUB)
    MATH_CASE(DIV)
    MATH_CASE(MOD)
    MATH_CASE(POW)

    CMP_CASE(GT)
    CMP_CASE(GTE)
    CMP_CASE(LT)
    CMP_CASE(LTE)

    default:
      break;
  }

  if (isAssignment) {
    compileWriteOperation(&out, rtype->stackSizeBytes(), ctx, r1);
  }
}

void compileRValue(ScriptType* type, Expr* val, CompilerContext* ctx, registerid out);

void compileExpr(Expr* expr, registerid resultreg, AddrOutput* addr, CompilerContext* ctx) {
  astnodetype kind = expr->nodeKind();
  BytecodeWriter* writer = ctx->writer;

  switch (kind) {
    // TODO: Expressions
    //  - X ID
    //  -   Call
    //  - X PropAccess
    //  - X IndexAccess
    //  - X BooleanLiteral
    //  - X CharLiteral
    //  - X StringLiteral
    //  - X IntLiteral
    //  - X FloatLiteral
    //  - X ObjectLiteral
    //  - X ArrayLiteral
    //  - X BinaryExpr
    //  - X UnaryExpr
    //  - X TernaryExpr

    case AST_BinaryExpr: {
      compileBinaryExpr(static_cast<BinaryExpr*>(expr), resultreg, addr, ctx);
      return;
    }

    case AST_TernaryExpr: {
      TernaryExpr* ternary = static_cast<TernaryExpr*>(expr);

      // How it should work:
      //   - Evaluate condition:
      //   - Depending on result:
      //     - Jump to left if true
      //     - Jump to right otherwise

      compileExpr(ternary->condition, resultreg, nullptr, ctx);

      writer->appendOpCode(OP_JMPI0);
      uint64 firstJumpOff = writer->buflen;
      writer->appendU32(0);
      writer->appendU8(resultreg);
      writer->appendPadding(PAD_JMPI0);

      compileExpr(ternary->left, resultreg, nullptr, ctx);

      writer->appendOpCode(OP_JMP);
      uint64 secondJumpOff = writer->buflen;
      writer->appendU32(0);
      writer->appendPadding(PAD_JMP);

      writer->writeInstructionCounter(firstJumpOff);

      compileExpr(ternary->right, resultreg, nullptr, ctx);

      writer->writeInstructionCounter(secondJumpOff);

      return;
    }

    case AST_PropertyAccessExpr: {
      compilePropertyAccess(static_cast<PropertyAccessExpr*>(expr), resultreg, addr, ctx);
      return;
    }

    case AST_IndexAccessExpr: {
      compileIndexAccessExpr(static_cast<IndexAccessExpr*>(expr), resultreg, addr, ctx);
      return;
    }

    case AST_Identifier: {
      compileIdentifier(static_cast<Identifier*>(expr), resultreg, addr, ctx);
      return;
    }

    case AST_ArrayLiteral: {
      ArrayLiteral* lit = static_cast<ArrayLiteral*>(expr);
      ScriptArrayType* arrType = static_cast<ScriptArrayType*>(lit->resultType);

      uint64 csize = arrType->componentType->stackSizeBytes();
      uint64 count = lit->values.size();
      uint64 memsize = sizeof(uint32) + (csize * count);

      writer->appendOpCode(OP_HEAPALLOC);
      writer->appendU8(resultreg);
      writer->appendU64(memsize);
      writer->appendPadding(PAD_HEAPALLOC);

      registerid valreg = ctx->acquireRegister();
      registerid idxreg = ctx->acquireRegister();

      for (uint32 i = 0; i < count; i++) {
        Expr* expr = lit->values[i];
        compileRValue(arrType->componentType, expr, ctx, valreg);

        writer->appendOpCode(OP_LOADCONST32);
        writer->appendU8(idxreg);
        writer->appendU32(i);
        writer->appendPadding(PAD_LOADCONST32);

        BYTEWIDTH_OPCODE(arrType->componentType->stackSizeBytes(), writer, OP_WRITEIDX)
        writer->appendU8(resultreg);
        writer->appendU8(valreg);
        writer->appendU8(idxreg);
        writer->appendPadding(PAD_WRITEIDX);
      }

      ctx->freeRegister(valreg);
      ctx->freeRegister(idxreg);

      return;
    }

    case AST_ObjectLiteral: {
      //
      // Here we have to assume that resultreg already contains an allocated object of this value
      // structype name = {property = false}
      // ^ a variable of type structype will be
      //   allocated regardless of if the value is there or not,
      //   generated constructor called as well
      //
      // Really the only thing that worries me is call arguments like this:
      // someFunction({prop = true})
      //
      // Because it could also be
      // structtype x = {prop = false}
      // someFunction(x)
      //
      // And in the 2nd case no value allocation is required, but in the 1st one a
      // value must be allocated beforehand, on the plus side, the value can also
      // instantly be freed
      //
      // Yeah whatever, resultreg already contains the allocated object
      //
      ObjectLiteral* lit = static_cast<ObjectLiteral*>(expr);
      ScriptStructType* stype = static_cast<ScriptStructType*>(lit->resultType);

      registerid propreg = ctx->acquireRegister();

      for (ObjectLiteralProperty* prop : lit->properties) {
        std::string_view propName = ctx->stringTable->getview(prop->propertyName->value);
        ScriptType* ptype = nullptr;
        uint32 off = 0;

        for (uint32 i = 0; i < stype->propertyCount; i++) {
          StructProperty* prop = &stype->properties[i];
          if (prop->propertyName == propName) {
            ptype = prop->type;
            break;
          }
          off += prop->type->stackSizeBytes();
        }

        if (ptype->kind() == TK_STRUCT) {
          writer->appendOpCode(OP_READOBJ64);
          writer->appendU8(resultreg);
          writer->appendU8(propreg);
          writer->appendU32(off);
          writer->appendPadding(PAD_READOBJ);
        }
        compileExpr(prop->value, propreg, nullptr, ctx);

        if (ptype->kind() != TK_STRUCT) {
          BYTEWIDTH_OPCODE(ptype->stackSizeBytes(), writer, OP_WRITEOBJ)
          writer->appendU8(resultreg);
          writer->appendU8(propreg);
          writer->appendU32(off);
          writer->appendPadding(PAD_WRITEOBJ);
        }
      }

      ctx->freeRegister(propreg);
      return;
    }

    case AST_UnaryExpr: {
      UnaryExpr* un = static_cast<UnaryExpr*>(expr);
      unaryop uop = un->op;

      if (uop == UOP_POS) {
        compileExpr(un->target, resultreg, nullptr, ctx);
        return;
      }

      switch (uop) {
        case UOP_POS:
          compileExpr(un->target, resultreg, nullptr, ctx);
          return;
        case UOP_NEG: {
          compileExpr(un->target, resultreg, nullptr, ctx);

          PrimitiveScriptType* primType = static_cast<PrimitiveScriptType*>(un->target->resultType);
          NUMTYPE_OPCODE(primType->primtype, writer, OP_NEG)

          writer->appendU8(resultreg);
          writer->appendU8(resultreg);
          writer->appendPadding(7);
          return;
        }
        case UOP_BIT_NOT:
          compileExpr(un->target, resultreg, nullptr, ctx);
          writer->appendOpCode(OP_BNEGATE);
          writer->appendU8(resultreg);
          writer->appendU8(resultreg);
          writer->appendPadding(7);
          return;
        case UOP_LOG_NOT:
          compileExpr(un->target, resultreg, nullptr, ctx);
          writer->appendOpCode(OP_LNEGATE);
          writer->appendU8(resultreg);
          writer->appendU8(resultreg);
          writer->appendPadding(7);
          return;
        default:
          break;
      }

      // uop is now one of: preinc, postinc, predec, postdec

      // How these should work:
      //   PREINC / PREDEC:
      //     - Run opcodes for target
      //     - Increment/Decrement resulting value
      //     - Write value to address
      //     - Done
      //   POSTINC / POSTDEC:
      //     - Find 2nd register for target expression
      //     - Run opcodes for target, using 2nd register as target
      //     - Copy value from 2nd register to result register
      //     - Increment/Decrement resulting value in 2nd register
      //     - Write value in 2nd register to address
      //     - Done
      //
      // What seems to change is that for POSTINC and POSTDEC we use a 2nd
      // temporary register to hold the value and copy it over to the actual one
      // before changing the stored value in memory
      //

      AddrOutput addrout;
      registerid targetRegister;
      bool isPostOp = uop == UOP_POSTINC || uop == UOP_POSTDEC;

      if (isPostOp) {
        targetRegister = ctx->acquireRegister();
      } else {
        targetRegister = resultreg;
      }

      compileExpr(un->target, targetRegister, &addrout, ctx);

      if (isPostOp) {
        writer->appendInstruction(OP_MOV, targetRegister, resultreg);
      }

      PrimitiveScriptType* primType = static_cast<PrimitiveScriptType*>(un->target->resultType);
      primitivekind primkind = primType->primtype;

      switch (uop) {
        case UOP_PREINC:
        case UOP_POSTINC:
          NUMTYPE_OPCODE(primkind, writer, OP_INC)
          writer->appendU8(targetRegister);
          writer->appendU8(targetRegister);
          writer->appendPadding(7);
          break;

        case UOP_PREDEC:
        case UOP_POSTDEC:
          NUMTYPE_OPCODE(primkind, writer, OP_DEC)
          writer->appendU8(targetRegister);
          writer->appendU8(targetRegister);
          writer->appendPadding(7);
          break;

        default:
          break;
      }

      compileWriteOperation(&addrout, primType->stackSizeBytes(), ctx, targetRegister);
      return;
    }

    case AST_StringLiteral: {
      StringLiteral* lit = static_cast<StringLiteral*>(expr);
      uint64 off = ctx->emplaceConstString(lit->value);

      writer->appendOpCode(OP_LOADCONSTSTR);
      writer->appendU8(resultreg);
      writer->appendU64(off);

      return;
    }
    case AST_FloatLiteral: {
      PrimitiveScriptType* pst = static_cast<PrimitiveScriptType*>(ctx->expectedType.back());
      primitivekind pk = pst->primtype;
      float64 val = static_cast<FloatLiteral*>(expr)->value;

      if (pk == PK_FLOAT32) {
        writer->appendOpCode(OP_LOADCONST32);
        writer->appendU8(resultreg);
        writer->appendF32(val);
        writer->appendPadding(PAD_LOADCONST32);
      } else {
        writer->appendOpCode(OP_LOADCONST64);
        writer->appendU8(resultreg);
        writer->appendF64(val);
      }

      return;
    }
    case AST_BooleanLiteral: {
      bool val = static_cast<BooleanLiteral*>(expr)->value;
      writer->appendOpCode(OP_LOADCONST8);
      writer->appendU8(resultreg);
      writer->appendU8(val);
      writer->appendPadding(PAD_LOADCONST8);
      return;
    }
    case AST_IntLiteral: {
      IntLiteral* il = static_cast<IntLiteral*>(expr);
      PrimitiveScriptType* pst = static_cast<PrimitiveScriptType*>(ctx->expectedType.back());
      primitivekind pk = pst->primtype;

      switch (pk) {
        case PK_BOOL:
          writer->appendOpCode(OP_LOADCONST8);
          writer->appendU8(resultreg);
          if (il->value) {
            writer->appendU8(1);
          } else {
            writer->appendU8(0);
          }
          writer->appendPadding(PAD_LOADCONST8);
          break;

        case PK_INT8:
        case PK_UINT8:
          writer->appendOpCode(OP_LOADCONST8);
          writer->appendU8(resultreg);
          writer->appendU8(il->value);
          writer->appendPadding(PAD_LOADCONST8);
          break;

        case PK_UINT16:
        case PK_INT16:
          writer->appendOpCode(OP_LOADCONST16);
          writer->appendU8(resultreg);
          writer->appendU16(il->value);
          writer->appendPadding(PAD_LOADCONST16);
          break;

        case PK_UINT32:
        case PK_INT32:
          writer->appendOpCode(OP_LOADCONST32);
          writer->appendU8(resultreg);
          writer->appendU32(il->value);
          writer->appendPadding(PAD_LOADCONST32);
          break;

        case PK_UINT64:
        case PK_INT64:
          writer->appendOpCode(OP_LOADCONST64);
          writer->appendU8(resultreg);
          writer->appendU64(il->value);
          break;

        case PK_FLOAT32:
          writer->appendOpCode(OP_LOADCONST32);
          writer->appendU8(resultreg);
          writer->appendF32(static_cast<float32>(il->value));
          writer->appendPadding(4);
          break;
        case PK_FLOAT64:
          writer->appendOpCode(OP_LOADCONST32);
          writer->appendU8(resultreg);
          writer->appendF64(static_cast<float64>(il->value));
          break;

        default:
          // Not possible
          break;
      }

      return;
    }

    default:
      break;
  }
}

void compileStructInitCall(ScriptStructType* type, CompilerContext* ctx, registerid out) {
  BytecodeWriter* writer = ctx->writer;

  uint32 funcTableIndex = ctx->declaredStructConstructors[type];
  uint64 heapsize = type->heapSize();

  writer->appendOpCode(OP_HEAPALLOC);
  writer->appendU8(out);
  writer->appendU64(heapsize);

  writer->appendOpCode(OP_PUSHARG);
  writer->appendU8(out);
  writer->appendPadding(PAD_PUSHARG);

  writer->appendOpCode(OP_VINVOKE);
  writer->appendU32(funcTableIndex);
  writer->appendPadding(PAD_VINVOKE);
}

void compileValueInitialiser(ScriptType* type, CompilerContext* ctx, registerid out) {
  BytecodeWriter* writer = ctx->writer;

  switch (type->kind()) {
    case TK_PRIMITIVE:
    case TK_STRING:
    case TK_ARRAY:
      BYTEWIDTH_OPCODE(type->stackSizeBytes(), writer, OP_LOADCONST)
      writer->appendU8(out);
      writer->appendU64(0);
      break;

    case TK_STRUCT:
      compileStructInitCall(static_cast<ScriptStructType*>(type), ctx, out);
      break;

    default:
      break;
  }
}

uint64 measureBlock(Block* block) {
  uint64 bsize = 0;
  for (Statement* stat : block->statements) {
    if (stat->nodeKind() != AST_LexicalDeclaration) {
      continue;
    }
    bsize += static_cast<LexicalDeclaration*>(stat)->typeExpr->referencedType->stackSizeBytes();
  }
  return bsize;
}

uint32 blockBegin(BytecodeWriter* writer, uint64 bytes, uint64* addrOut) {
  uint32 instr = writer->getInstructionCounter();

  writer->appendOpCode(OP_STACKALLOC);

  if (addrOut) {
    *addrOut = writer->buflen;
  }

  writer->appendU64(bytes);
  writer->appendPadding(PAD_STACKALLOC);

  return instr;
}

void blockEnd(BytecodeWriter* writer, uint64 bytes) {
  writer->appendOpCode(OP_STACKFREE);
  writer->appendU64(bytes);
  writer->appendPadding(PAD_STACKFREE);

  writer->appendOpCode(OP_RET);
  writer->appendPadding(PAD_RET);
}

void compileStatement(Statement* stat, CompilerContext* ctx);

void compileBlock(Block* block, CompilerContext* ctx, uint64 size = 0) {
  blockBegin(ctx->writer, size, nullptr);
  for (Statement* statement : block->statements) {
    compileStatement(statement, ctx);
  }
  blockEnd(ctx->writer, size);
}

void compileLexDecl(LexicalDeclaration* lex, CompilerContext* ctx) {
  ScriptType* stype = lex->typeExpr->referencedType;

  StackScope* scope = ctx->getScope();
  StackSymbol* sym = scope->pushSymbol(lex->variableName->value, SSYM_VAR);
  sym->stacksize = stype->stackSizeBytes();

  BytecodeWriter* writer = ctx->writer;

  registerid valreg = ctx->acquireRegister();
  compileRValue(stype, lex->value, ctx, valreg);

  BYTEWIDTH_OPCODE(lex->value->resultType->stackSizeBytes(), writer, OP_RSWRITE)
  writer->appendU8(valreg);
  writer->appendU64(sym->stackoffset);

  ctx->freeRegister(valreg);
}

void compileIfStatement(IfStatement* stat, CompilerContext* ctx) {
  registerid reg = ctx->acquireRegister();
  BytecodeWriter* writer = ctx->writer;

  compileExpr(stat->condition, reg, nullptr, ctx);

  writer->appendOpCode(OP_JMPI0);
  uint64 condFailedJumpAddr = writer->buflen;

  writer->appendU32(0);
  writer->appendU8(reg);
  writer->appendPadding(PAD_JMPI0);

  compileStatement(stat->body, ctx);

  if (stat->elseBody) {
    writer->appendOpCode(OP_JMP);
    uint64 afterElseAddr = writer->buflen;
    writer->appendU32(0);
    writer->appendPadding(PAD_JMP);

    writer->writeInstructionCounter(condFailedJumpAddr);

    compileStatement(stat->elseBody, ctx);

    writer->writeInstructionCounter(afterElseAddr);
  } else {
    writer->writeInstructionCounter(condFailedJumpAddr);
  }
}

void compileFuncDecl(FunctionDeclStatement* stat, CompilerContext* ctx) {
  uint64 stacksize = 0;
  for (FunctionParam* p : stat->arguments) {
    stacksize += p->paramType->referencedType->stackSizeBytes();
  }
  stacksize += measureBlock(stat->functionBody);

  uint32 start = blockBegin(ctx->writer, stacksize, nullptr);



  blockEnd(ctx->writer, stacksize);
}

void compileStatement(Statement* stat, CompilerContext* ctx) {
  astnodetype kind = stat->nodeKind();
  BytecodeWriter* writer = ctx->writer;

  switch (kind) {
    case AST_LexicalDeclaration:
      compileLexDecl(static_cast<LexicalDeclaration*>(stat), ctx);
      return;
    case AST_FunctionDeclStatement:
      ctx->funcQueue.push_back(static_cast<FunctionDeclStatement*>(stat));
      return;
    case AST_IfStatement:
      compileIfStatement(static_cast<IfStatement*>(stat), ctx);
      return;

    case AST_Block: {
      Block* b = static_cast<Block*>(stat);
      uint64 size = measureBlock(b);
      compileBlock(b, ctx, size);
      return;
    }

    default:
      return;
  }
}

void compileRValue(ScriptType* type, Expr* val, CompilerContext* ctx, registerid out) {
  if (!val) {
    compileValueInitialiser(type, ctx, out);
    return;
  }

  if (type->kind() == TK_STRUCT) {
    compileStructInitCall(static_cast<ScriptStructType*>(type), ctx, out);
  }

  compileExpr(val, out, nullptr, ctx);
}

void compileStructDecl(StructDecl* decl, CompilerContext* ctx) {
  ScriptStructType* stype = decl->type;
  ctx->declaredStructs.push_back(stype);

  BytecodeWriter* writer = ctx->writer;
  uint32 pcount = decl->properties.size();
  uint64 stacksize = sizeof(uint64);

  ctx->pushScope();

  uint32 start = blockBegin(writer, stacksize, nullptr);

  registerid selfreg = ctx->acquireRegister();
  registerid propreg = ctx->acquireRegister();

  writer->appendOpCode(OP_RSREAD64);
  writer->appendU8(selfreg);
  writer->appendU64(0);
  writer->appendPadding(PAD_RSREAD);

  uint32 off = 0;
  for (uint32 i = 0; i < pcount; i++) {
    StructPropertyDecl* pdecl = decl->properties.at(i);
    StructProperty* ptype = &stype->properties[i];

    compileRValue(ptype->type, pdecl->value, ctx, propreg);

    BYTEWIDTH_OPCODE(ptype->type->stackSizeBytes(), writer, OP_WRITEOBJ)
    writer->appendU8(selfreg);
    writer->appendU8(propreg);
    writer->appendU32(off);

    off += ptype->type->stackSizeBytes();
  }

  ctx->popScope();
  ctx->freeRegister(selfreg);
  ctx->freeRegister(propreg);

  blockEnd(writer, stacksize);

  std::string fname;
  fname.append(stype->structName);
  fname.append(".<init>");

  stringid id = ctx->stringTable->allocate(fname);
  uint64 constPoolOff = ctx->emplaceConstString(id);

  FunctionSignatureParam param = {
    .type = stype,
    .varargs = false
  };
  FunctionSignature sign = FunctionSignature();
  sign.returnType = ctx->types->getVoidType();
  sign.paramCount = 1;
  sign.params = &param;

  FunctionSignature* emplaced = ctx->types->emplaceFunctionType(&sign);
  uint32 funcIdx = ctx->pushCompiledFunction(emplaced, start, constPoolOff);

  ctx->declaredStructConstructors[stype] = funcIdx;
}

Bytecode compile(ScriptFileStatement* sfs, StringTable* table) {
  BytecodeWriter writer;

  const uint64 initcap = LENGTH_INSTRUCTION * 1024;

  writer.buf = static_cast<uint8*>(malloc(initcap));
  writer.bufcap = initcap;

  if (!writer.buf) {
    throw std::runtime_error("Failed to allocate initial bytecode buffer");
  }

  ConstStringPoolWriter stringPool;
  uint64 registersInUse = 0;

  CompilerContext ctx;
  ctx.stringPool = &stringPool;
  ctx.stringTable = table;
  ctx.writer = &writer;
  ctx.registersInUse = &registersInUse;

  ctx.pushScope();

  for (Statement* stat : sfs->statements) {
    if (stat->nodeKind() != AST_LexicalDeclaration) {
      continue;
    }
    compileStructDecl(static_cast<StructDecl*>(stat), &ctx);
  }

  ctx.popScope();

  return {
    .data = nullptr,
    .len = 0
  };
}
