#include "compiler.h"

#include <stdarg.h>

#include "opcodespec.h"
#include "../interpreter/opcodes.h"

#define ALL_REGISTERS_USED 0xFFFFFFFFFFFFFFFF
#define NIL_REGISTER -1

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

struct CompilerContext {
  std::vector<ScriptType*> expectedType;
  std::vector<StackScope> scopes;

  BytecodeWriter* writer = nullptr;
  ConstStringPoolWriter* stringPool = nullptr;
  StringTable* stringTable = nullptr;

  uint64 registersInUse = 0;

  StackScope* getScope() {
    return &scopes.back();
  }

  StackScope* pushScope() {
    StackScope scope = StackScope();
    scope.level = scopes.size();
    scopes.push_back(scope);
    return &scopes.back();
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
    }

    uint8* writeptr = stringPool->data + stringPool->len;

    *reinterpret_cast<uint32*>(writeptr) = len;
    writeptr += sizeof(uint32);

    char* charptr = reinterpret_cast<char*>(writeptr);
    stringTable->copychars(id, charptr, len);
  }

  registeridopt findFreeRegister() const {
    if (registersInUse == ALL_REGISTERS_USED) {
      return NIL_REGISTER;
    }
    if (registersInUse == 0) {
      return 0;
    }

    uint64 m;
    for (int8 i = 0; i < 64; i++) {
      m = 1 << i;
      if (m & registersInUse) {
        continue;
      }
      return i;
    }

    return NIL_REGISTER;
  }

  bool registerInUse(const registerid id) const {
    uint64 m = 1L << id;
    return registersInUse & m;
  }

  void useRegister(const registerid id) {
    registersInUse |= (1L << id);
  }

  void freeRegister(const registerid id) {
    registersInUse &= ~(1L << id);
  }
};

#define OUTP_NIL    0
#define OUTP_ASTACK 1
#define OUTP_RSTACK 2
#define OUTP_PROP   3
#define OUTP_IDX    4

struct AddrOutput {
  uint8 outptype = OUTP_NIL;
  registeridopt objectRegister = NIL_REGISTER;
  registeridopt indexRegister = NIL_REGISTER;
  uint32 memoffset = 0;
  uint64 stackoffset = 0;
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
APPEND_METHOD(appendOpCode, LENGTH_OPCODE, opcode)

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

uint64 measureStackSize(Statement* s) {
  astnodetype kind = s->nodeKind();

  switch (kind) {
    case AST_LexicalDeclaration:
      return static_cast<LexicalDeclaration*>(s)->typeExpr->getReferencedType()->stackSizeBytes();
    default:
      return 0;
  }
}

void appendStatement(Statement* stat, CompilerContext* ctx) {
  astnodetype kind = stat->nodeKind();

  if (kind == AST_LexicalDeclaration) {
    LexicalDeclaration* lex = static_cast<LexicalDeclaration*>(stat);

    ScriptType* stype = lex->typeExpr->getReferencedType();

    StackScope* scope = ctx->getScope();
    StackSymbol* sym = scope->pushSymbol(lex->variableName->value, SSYM_VAR);
    sym->stacksize = stype->stackSizeBytes();

    return;
  }
}

#define BYTEWIDTH_OPCODE(size, writer, opcode) \
  switch (size) {\
    case 1: writer->appendOpCode(opcode##8); break;\
    case 2: writer->appendOpCode(opcode##16); break;\
    case 4: writer->appendOpCode(opcode##32); break;\
    default: writer->appendOpCode(opcode##64); break;\
  }

#define NUMTYPE_OPCODE(primkind, writer, opcode) \
  switch (primkind) { \
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

void appendExpr(Expr* expr, registerid resultreg, AddrOutput* addr, CompilerContext* ctx) {
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
    //  -   BinaryExpr
    //  - X UnaryExpr
    //  -   TernaryExpr

    case AST_PropertyAccessExpr: {
      PropertyAccessExpr* prop = static_cast<PropertyAccessExpr*>(expr);
      ScriptType* type = prop->target->resultType;

      typekind targetkind = type->kind();

      registerid targetreg = ctx->findFreeRegister();
      ctx->useRegister(targetreg);

      appendExpr(prop->target, targetreg, nullptr, ctx);

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

      writer->appendOpCode(OP_READOBJ32);
      writer->appendU8(targetreg);
      writer->appendU8(resultreg);
      writer->appendU8(propoff);

      if (addr) {
        addr->outptype = OUTP_PROP;
        addr->objectRegister = targetreg;
        addr->memoffset = propoff;
      } else {
        ctx->freeRegister(targetreg);
      }

      return;
    }

    case AST_IndexAccessExpr: {
      IndexAccessExpr* access = static_cast<IndexAccessExpr*>(expr);

      registerid targetreg = ctx->findFreeRegister();
      ctx->useRegister(targetreg);

      registerid idxreg = ctx->findFreeRegister();
      ctx->useRegister(idxreg);

      appendExpr(access->target, targetreg, nullptr, ctx);
      uint32 stackSize = access->resultType->stackSizeBytes();

      BYTEWIDTH_OPCODE(stackSize, writer, OP_READIDX)

      writer->appendU8(targetreg);
      writer->appendU8(resultreg);
      writer->appendU8(idxreg);
      writer->appendPadding(6);

      if (addr) {
        addr->outptype = OUTP_IDX;
        addr->objectRegister = targetreg;
        addr->indexRegister = idxreg;
      } else {
        ctx->freeRegister(targetreg);
        ctx->freeRegister(idxreg);
      }

      return;
    }

    case AST_Identifier: {
      Identifier* id = static_cast<Identifier*>(expr);
      StackSymType type = SSYM_NIL;

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
        BYTEWIDTH_OPCODE(sym->stacksize, writer, OP_RSREAD)

        writer->appendU8(resultreg);
        writer->appendU64(sym->stackoffset);

        return;
      }

      // TODO: Figure out how to read from upper levels of scopes
      //
      //   So I think the way this needs to be done (reading from upper scopes) depends on
      //   which variable or constant is being referenced. If we're referencing a global
      //   variable declared in the main scope of the script, then we use ASREAD (Absolute
      //   stack read) which takes in a memory offset relative to the start of the script's
      //   main scope, or to read ASREAD with a negative number if not.
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
      //   This is s confusing
      //

      // Main scope, aka, a global variable
      if (scope->level == 0) {
        BYTEWIDTH_OPCODE(sym->stacksize, writer, OP_ASREAD)

        writer->appendU8(resultreg);
        writer->appendU64(sym->stackoffset);

        return;
      }

      return;
    }

    case AST_UnaryExpr: {
      UnaryExpr* un = static_cast<UnaryExpr*>(expr);
      unaryop uop = un->op;

      if (uop == UOP_POS) {
        appendExpr(un->target, resultreg, nullptr, ctx);
        return;
      }

      switch (uop) {
        case UOP_POS:
          appendExpr(un->target, resultreg, nullptr, ctx);
          return;
        case UOP_NEG: {
          appendExpr(un->target, resultreg, nullptr, ctx);

          PrimitiveScriptType* primType = static_cast<PrimitiveScriptType*>(un->target->resultType);
          NUMTYPE_OPCODE(primType->primtype, writer, OP_NEG)

          writer->appendU8(resultreg);
          writer->appendU8(resultreg);
          writer->appendPadding(7);
          return;
        }
        case UOP_BIT_NOT:
          appendExpr(un->target, resultreg, nullptr, ctx);
          writer->appendOpCode(OP_BNEGATE);
          writer->appendU8(resultreg);
          writer->appendU8(resultreg);
          writer->appendPadding(7);
          return;
        case UOP_LOG_NOT:
          appendExpr(un->target, resultreg, nullptr, ctx);
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
        targetRegister = ctx->findFreeRegister();
        ctx->useRegister(targetRegister);
      } else {
        targetRegister = resultreg;
      }

      appendExpr(un->target, targetRegister, &addrout, ctx);

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

      switch (addrout.outptype) {
        case OUTP_IDX:
          BYTEWIDTH_OPCODE(primType->stackSizeBytes(), writer, OP_WRITEIDX)
          writer->appendU8(addrout.objectRegister);
          writer->appendU8(targetRegister);
          writer->appendU8(addrout.indexRegister);
          writer->appendPadding(PAD_WRITEIDX);
          break;
        case OUTP_PROP:
          BYTEWIDTH_OPCODE(primType->stackSizeBytes(), writer, OP_WRITEOBJ)
          writer->appendU8(addrout.objectRegister);
          writer->appendU8(targetRegister);
          writer->appendU32(addrout.memoffset);
          writer->appendPadding(PAD_WRITEOBJ);
          break;
        case OUTP_RSTACK:
          BYTEWIDTH_OPCODE(primType->stackSizeBytes(), writer, OP_RSWRITE)
          writer->appendU8(targetRegister);
          writer->appendU64(addrout.stackoffset);
          break;
        case OUTP_ASTACK:
          BYTEWIDTH_OPCODE(primType->stackSizeBytes(), writer, OP_ASWRITE)
          writer->appendU8(targetRegister);
          writer->appendU64(addrout.stackoffset);
          break;
        default:
          break;
      }

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

Bytecode compile(ScriptFileStatement* sfs, StringTable* table) {
  BytecodeWriter writer;

  const uint64 initcap = LENGTH_INSTRUCTION * 1024;

  writer.buf = static_cast<uint8*>(malloc(initcap));
  writer.bufcap = initcap;

  if (!writer.buf) {
    throw std::runtime_error("Failed to allocate initial bytecode buffer");
  }

  ConstStringPoolWriter stringPool;

  CompilerContext ctx;
  ctx.stringPool = &stringPool;
  ctx.stringTable = table;
  ctx.writer = &writer;


}
