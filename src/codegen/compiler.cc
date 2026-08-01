#include "compiler.h"
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
#define OUTP_STACK  1
#define OUTP_PROP   2
#define OUTP_IDX    3

struct AddrOutput {
  uint8 outptype = OUTP_NIL;
  registeridopt reg1 = NIL_REGISTER;
  registeridopt reg2 = NIL_REGISTER;
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
APPEND_METHOD(appendOpCode, 2, opcode)

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
  reserveSpace(bytes);
  memset(buf + buflen, 0, bytes);
  buflen += bytes;
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

#define BYTEWIDTH_SWITCH(size, u8, u16, u32, u64) \
  switch (size) {\
    case 1: u8 break;\
    case 2: u16 break;\
    case 4: u32 break;\
    default: u64 break;\
  }

#define BYTEWIDTH_OPCODE(size, writer, u8, u16, u32, u64) \
  switch (size) {\
    case 1: writer->appendOpCode(u8); break;\
    case 2: writer->appendOpCode(u16); break;\
    case 4: writer->appendOpCode(u32); break;\
    default: writer->appendOpCode(u64); break;\
  }

void appendExpr(Expr* expr, registerid resultreg, AddrOutput* addr, CompilerContext* ctx) {
  astnodetype kind = expr->nodeKind();
  BytecodeWriter* writer = ctx->writer;

  switch (kind) {
    case AST_IndexAccessExpr: {
      IndexAccessExpr* access = static_cast<IndexAccessExpr*>(expr);

      registerid targetreg = ctx->findFreeRegister();
      ctx->useRegister(targetreg);

      registerid idxreg = ctx->findFreeRegister();
      ctx->useRegister(idxreg);

      appendExpr(access->target, targetreg, nullptr, ctx);
      uint32 stackSize = access->resultType->stackSizeBytes();

      BYTEWIDTH_OPCODE(stackSize, writer, OP_READIDX8, OP_READIDX16, OP_READIDX32, OP_READIDX64)

      writer->appendU8(targetreg);
      writer->appendU8(resultreg);
      writer->appendU8(idxreg);
      writer->appendPadding(6);

      if (addr) {
        addr->outptype = OUTP_IDX;
        addr->reg1 = targetreg;
        addr->reg2 = idxreg;
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

      if (scope == current) {
        BYTEWIDTH_OPCODE(sym->stacksize, writer, OP_RSREAD8, OP_RSREAD16, OP_RSREAD32, OP_RSREAD64)

        writer->appendU8(resultreg);
        writer->appendU64(sym->stackoffset);

        return;
      }

      // TODO: Figure out how to read from upper levels of scopes
      //
      //   So I think the way this needs to be done (reading from upper scopes) depends on
      //   which variable or constant is being referenced. If we're referencing a global
      //   variable declared in the main scope of the script, then we use ASREAD (Absolute
      //   read) which takes in a memory offset relative to the start of the script's main
      //   scope, or to read ASREAD with a negative number if not.
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
        BYTEWIDTH_OPCODE(sym->stacksize, writer, OP_ASREAD8, OP_ASREAD16, OP_ASREAD32, OP_ASREAD64)

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
        case UOP_NEG:
          appendExpr(un->target, resultreg, nullptr, ctx);
          writer->appendOpCode(OP_NEG);
          writer->appendU8(resultreg);
          writer->appendU8(resultreg);
          writer->appendPadding(7);
          return;
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
      AddrOutput addrout;
      appendExpr(un->target, resultreg, &addrout, ctx);



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
        writer->appendPadding(3);
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
      writer->appendPadding(5);
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
          writer->appendPadding(5);
          break;

        case PK_INT8:
        case PK_UINT8:
          writer->appendOpCode(OP_LOADCONST8);
          writer->appendU8(resultreg);
          writer->appendU8(il->value);
          writer->appendPadding(5);
          break;

        case PK_UINT16:
        case PK_INT16:
          writer->appendOpCode(OP_LOADCONST16);
          writer->appendU8(resultreg);
          writer->appendU16(il->value);
          writer->appendPadding(6);
          break;

        case PK_UINT32:
        case PK_INT32:
          writer->appendOpCode(OP_LOADCONST32);
          writer->appendU8(resultreg);
          writer->appendU32(il->value);
          writer->appendPadding(4);
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
  writer.reserveSpace(1024);

  ConstStringPoolWriter stringPool;

  CompilerContext ctx;
  ctx.stringPool = &stringPool;
  ctx.stringTable = table;
  ctx.writer = &writer;


}
