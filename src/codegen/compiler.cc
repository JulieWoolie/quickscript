#include "compiler.h"

#include "CompilerContext.h"
#include "../interpreter/interpreter.h"
#include "../interpreter/ir_file.h"
#include "../interpreter/opcodes.h"
#include "../types/ConstTypes.h"

#define BYTEWIDTH_OPCODE(size, writer, opcode) \
  switch (size) {\
    case 1: writer.startInstr(opcode##8); break;\
    case 2: writer.startInstr(opcode##16); break;\
    case 4: writer.startInstr(opcode##32); break;\
    default: writer.startInstr(opcode##64); break;\
  }

#define NUMTYPE_OPCODE(primkind, writer, opcode) \
  switch (primkind) { \
    case PK_BOOL:\
    case PK_UINT8: writer.startInstr(opcode##U8); break; \
    case PK_INT8: writer.startInstr(opcode##I8); break; \
    case PK_UINT16: writer.startInstr(opcode##U16); break; \
    case PK_INT16: writer.startInstr(opcode##I16); break; \
    case PK_UINT32: writer.startInstr(opcode##U32); break; \
    case PK_INT32: writer.startInstr(opcode##I32); break; \
    case PK_UINT64: writer.startInstr(opcode##U64); break; \
    case PK_INT64: writer.startInstr(opcode##I64); break; \
    case PK_FLOAT32: writer.startInstr(opcode##F32); break; \
    default: writer.startInstr(opcode##F64); break; \
  }

#define BIN_APPEND \
      writer.appendU8(r1);\
      writer.appendU8(r2);\
      writer.appendU8(r1);\
      writer.endInstr();

#define CMP_CASE(cmptype) \
  case BOP_##cmptype: \
    if (lkind == TK_PRIMITIVE) { \
      NUMTYPE_OPCODE(lpk, writer, OP_##cmptype) \
    } else { \
      writer.startInstr(OP_##cmptype##ARR); \
    } \
    BIN_APPEND \
    break;

#define EQUALITY_CASE(type)\
  case BOP_##type:\
    if (lkind == TK_PRIMITIVE) {\
      BYTEWIDTH_OPCODE(ltype->stackSizeBytes(), writer, OP_##type)\
    } else if (lkind == TK_ARRAY || lkind == TK_STRING) {\
      writer.startInstr(OP_##type##ARR);\
    } else if (lkind == TK_STRUCT) {\
      writer.startInstr(OP_##type##STRUCT);\
    }\
    BIN_APPEND\
    break;

#define MATH_CASE(type)\
  case BOP_##type:\
    NUMTYPE_OPCODE(lpk, writer, OP_##type)\
    BIN_APPEND\
    break;

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

static void compileWriteOperation(
  const AddrOutput* addrout,
  const uint32 stackSize,
  CompilerContext& ctx,
  const registerid valueRegister
) {
  BytecodeWriter& writer = ctx.getWriter();

  switch (addrout->outptype) {
    case OUTP_IDX:
      BYTEWIDTH_OPCODE(stackSize, writer, OP_WRITEIDX)
      writer.appendU8(addrout->objectRegister);
      writer.appendU8(valueRegister);
      writer.appendU8(addrout->indexRegister);
      writer.endInstr();
      break;
    case OUTP_PROP:
      BYTEWIDTH_OPCODE(stackSize, writer, OP_WRITEOBJ)
      writer.appendU8(addrout->objectRegister);
      writer.appendU8(valueRegister);
      writer.appendU32(addrout->memoffset);
      writer.endInstr();
      break;
    case OUTP_RSTACK:
      BYTEWIDTH_OPCODE(stackSize, writer, OP_RSWRITE)
      writer.appendU8(valueRegister);
      writer.appendU64(addrout->stackoffset);
      break;
    case OUTP_ASTACK:
      BYTEWIDTH_OPCODE(stackSize, writer, OP_ASWRITE)
      writer.appendU8(valueRegister);
      writer.appendU64(addrout->stackoffset);
      break;
    default:
      break;
  }

  if (addrout->objectRegister != NO_REGISTER) {
    ctx.freeRegister(addrout->objectRegister);
  }
  if (addrout->indexRegister != NO_REGISTER) {
    ctx.freeRegister(addrout->indexRegister);
  }
}

static void compileExpr(Expr* expr, registerid out, AddrOutput* addr, CompilerContext& ctx);

static void compileIndexAccessExpr(
  const IndexAccessExpr* access,
  const registeridopt out,
  AddrOutput* addr,
  CompilerContext& ctx
) {
  const registerid targetReg = ctx.acquireRegister();
  const registerid idxReg = ctx.acquireRegister();

  compileExpr(access->target, targetReg, nullptr, ctx);
  uint32 stackSize = access->resultType->stackSizeBytes();

  if (out != NO_REGISTER) {
    BytecodeWriter& writer = ctx.getWriter();
    BYTEWIDTH_OPCODE(stackSize, writer, OP_READIDX)
    writer.appendU8(targetReg);
    writer.appendU8(out);
    writer.appendU8(idxReg);
    writer.endInstr();
  }

  if (addr) {
    addr->outptype = OUTP_IDX;
    addr->objectRegister = targetReg;
    addr->indexRegister = idxReg;
  } else {
    ctx.freeRegister(targetReg);
    ctx.freeRegister(idxReg);
  }
}

static void compilePropertyAccess(
  const PropertyAccessExpr* prop,
  const registeridopt out,
  AddrOutput* addr,
  CompilerContext& ctx
) {
  ScriptType* type = prop->target->resultType;

  const typekind targetKind = type->kind();
  const registerid targetReg = ctx.acquireRegister();

  compileExpr(prop->target, targetReg, nullptr, ctx);

  uint32 propertyOffset;

  if (targetKind != TK_STRUCT) {
    // Only non struct property that is available is the length property on
    // arrays and strings which is always at offset 0x0
    propertyOffset = 0;
  } else {
    std::string_view view = ctx.getSemantics().getStrings().getview(prop->property->value);
    ScriptStructType* structType = static_cast<ScriptStructType*>(type);

    propertyOffset = 0;

    for (uint32 p = 0; p < structType->getPropertyCount(); p++) {
      const StructProperty* structProp = structType->getProperty(p);

      if (structProp->name != view) {
        propertyOffset += structProp->type->stackSizeBytes();
        continue;
      }

      break;
    }
  }

  if (out != NO_REGISTER) {
    BytecodeWriter& writer = ctx.getWriter();
    writer.startInstr(OP_READOBJ32);
    writer.appendU8(targetReg);
    writer.appendU8(out);
    writer.appendU8(propertyOffset);
  }

  if (addr) {
    addr->outptype = OUTP_PROP;
    addr->objectRegister = targetReg;
    addr->memoffset = propertyOffset;
  } else {
    ctx.freeRegister(targetReg);
  }
}

static void compileIdentifier(
  Identifier* id,
  const registeridopt out,
  AddrOutput* addr,
  CompilerContext& ctx
) {
  BytecodeWriter& writer = ctx.getWriter();

  if (id->resultType->kind() == TK_FUNC) {
    FunctionSignature* sign = static_cast<FunctionSignature*>(id->resultType);
    const stringid name = id->value;

    const int32 funcIdx = ctx.findFunctionIndex(name, sign);

    writer.startInstr(OP_FUNCLOOKUP);
    writer.appendU8(out);

    if (funcIdx == -1) {
      const uint64 idxAddr = writer.getAddress();
      writer.appendU32(0);
      ctx.pushIncompleteCall(name, sign, idxAddr);
    } else {
      writer.appendU32(funcIdx);
    }

    writer.endInstr();
  }

  SemanticContext& semantics = ctx.getSemantics();

  Symbol* sym = semantics.getSymbolLookup()[id];
  Scope* scope = semantics.getScopeLookup()[sym];

  // Variable declared in current scope
  if (scope == current) {
    if (out != NO_REGISTER) {
      BYTEWIDTH_OPCODE(sym->stackSize, writer, OP_RSREAD)
      writer.appendU8(out);
      writer.appendU64(sym->stackOffset);
    }

    if (addr) {
      addr->outptype = OUTP_RSTACK;
      addr->stackoffset = sym->stackOffset;
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
  if (scope->getLevel() == 0) {
    if (out != NO_REGISTER) {
      BYTEWIDTH_OPCODE(sym->stackSize, writer, OP_ASREAD)
      writer.appendU8(out);
      writer.appendU64(sym->stackOffset);
    }

    if (addr) {
      addr->outptype = OUTP_RSTACK;
      addr->stackoffset = sym->stackOffset;
    }

    return;
  }
}

static void compileNonReadingExpr(Expr* expr, AddrOutput* addr, CompilerContext& ctx) {
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

static void compileBinaryExpr(
  const BinaryExpr* bin,
  const registerid r1,
  CompilerContext& ctx
) {
  BytecodeWriter& writer = ctx.getWriter();

  const binaryop bop = bin->op;
  const binaryop nonAssign = bop & ~BOP_ASSIGN_FLAG;

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

    uint32 stackSize = rhs->resultType->stackSizeBytes();
    compileWriteOperation(&out, stackSize, ctx, r1);

    return;
  }

  const bool isAssignment = bop & BOP_ASSIGN_FLAG;
  AddrOutput out;

  if (nonAssign == BOP_LOG_AND || nonAssign == BOP_LOG_OR) {
    compileExpr(lhs, r1, &out, ctx);

    if (nonAssign == BOP_LOG_AND) {
      writer.startInstr(OP_JMPI0);
    } else {
      writer.startInstr(OP_JMPN0);
    }

    const uint64 jumpAddrOffset = writer.getAddress();

    writer.appendU32(0);
    writer.appendU8(r1);

    compileExpr(rhs, r1, &out, ctx);

    writer.writeInstructionCounter(jumpAddrOffset);

    if (isAssignment) {
      compileWriteOperation(&out, 1, ctx, r1);
    }

    return;
  }

  ScriptType* ltype = lhs->resultType;
  ScriptType* rtype = rhs->resultType;

  const registerid r2 = ctx.acquireRegister();

  compileExpr(lhs, r2, &out, ctx);
  compileExpr(rhs, r1, nullptr, ctx);

  const typekind lkind = ltype->kind();
  primitivekind lpk = PK_NIL;
  primitivekind rpk = PK_NIL;

  if (lkind == TK_PRIMITIVE) {
    lpk = static_cast<PrimitiveScriptType*>(ltype)->getPrimitiveType();
  }
  if (rtype->kind() == TK_PRIMITIVE) {
    rpk = static_cast<PrimitiveScriptType*>(rtype)->getPrimitiveType();
  }

  switch (nonAssign) {
    case BOP_SHL:
      writer.startInstr(OP_LSHIFT);
      BIN_APPEND
      break;
    case BOP_SHR:
    case BOP_USHR:
      writer.startInstr(OP_RSHIFT);
      BIN_APPEND
      break;
    case BOP_BIT_OR:
      writer.startInstr(OP_BOR);
      BIN_APPEND
      break;
    case BOP_BIT_AND:
      writer.startInstr(OP_BAND);
      BIN_APPEND
      break;
    case BOP_XOR:
      if (lpk == PK_BOOL) {
        writer.startInstr(OP_BXOR);
      } else {
        writer.startInstr(OP_LXOR);
      }
      BIN_APPEND
      break;

    case BOP_ADD:
      if (lkind == TK_STRING) {
        writer.startInstr(OP_STRCONCAT);
      } else {
        NUMTYPE_OPCODE(lpk, writer, OP_ADD)
      }
      BIN_APPEND
      break;
    case BOP_MUL:
      if (lkind == TK_STRING) {
        BYTEWIDTH_OPCODE(rtype->stackSizeBytes(), writer, OP_STRREP)
      } else {
        NUMTYPE_OPCODE(lpk, writer, OP_ADD)
      }
      BIN_APPEND
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
  } else {
    switch (out.outptype) {
      case OUTP_IDX:
        ctx.freeRegister(out.indexRegister);
      case OUTP_PROP:
        ctx.freeRegister(out.objectRegister);
        break;
      default:
        break;
    }
  }
}

static void compileRValue(ScriptType* type, Expr* val, CompilerContext& ctx, registerid out);

static void compileExpr(Expr* expr, const registerid out, AddrOutput* addr, CompilerContext& ctx) {
  const astnodetype kind = expr->nodeKind();
  BytecodeWriter& writer = ctx.getWriter();

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
      compileBinaryExpr(static_cast<BinaryExpr*>(expr), out, ctx);
      return;
    }

    case AST_TernaryExpr: {
      const TernaryExpr* ternary = static_cast<TernaryExpr*>(expr);

      // How it should work:
      //   - Evaluate condition:
      //   - Depending on result:
      //     - Jump to left if true
      //     - Jump to right otherwise

      compileExpr(ternary->condition, out, nullptr, ctx);

      writer.startInstr(OP_JMPI0);
      const uint64 firstJumpOff = writer.getAddress();
      writer.appendU32(0);
      writer.appendU8(out);
      writer.endInstr();

      compileExpr(ternary->left, out, nullptr, ctx);

      writer.startInstr(OP_JMP);
      const uint64 secondJumpOff = writer.getAddress();
      writer.appendU32(0);
      writer.endInstr();

      writer.writeInstructionCounter(firstJumpOff);

      compileExpr(ternary->right, out, nullptr, ctx);

      writer.writeInstructionCounter(secondJumpOff);
      return;
    }

    case AST_PropertyAccessExpr: {
      compilePropertyAccess(static_cast<PropertyAccessExpr*>(expr), out, addr, ctx);
      return;
    }

    case AST_IndexAccessExpr: {
      compileIndexAccessExpr(static_cast<IndexAccessExpr*>(expr), out, addr, ctx);
      return;
    }

    case AST_Identifier: {
      compileIdentifier(static_cast<Identifier*>(expr), out, addr, ctx);
      return;
    }

    case AST_ArrayLiteral: {
      const ArrayLiteral* lit = static_cast<ArrayLiteral*>(expr);
      const ScriptArrayType* arrType = static_cast<ScriptArrayType*>(lit->resultType);

      ScriptType* cType = arrType->getComponentType();

      const uint64 componentSize = cType->stackSizeBytes();
      const uint64 count = lit->values.size();
      const uint64 memSize = sizeof(uint32) + (componentSize * count);

      writer.startInstr(OP_HEAPALLOC);
      writer.appendU8(out);
      writer.appendU64(memSize);
      writer.endInstr();

      const registerid valueReg = ctx.acquireRegister();
      const registerid indexReg = ctx.acquireRegister();

      for (uint32 i = 0; i < count; i++) {
        Expr* value = lit->values[i];
        compileRValue(cType, value, ctx, valueReg);

        writer.startInstr(OP_LOADCONST32);
        writer.appendU8(indexReg);
        writer.appendU32(i);
        writer.endInstr();

        BYTEWIDTH_OPCODE(componentSize, writer, OP_WRITEIDX)
        writer.appendU8(out);
        writer.appendU8(valueReg);
        writer.appendU8(indexReg);
        writer.endInstr();
      }

      ctx.freeRegister(valueReg);
      ctx.freeRegister(indexReg);

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
      const ObjectLiteral* lit = static_cast<ObjectLiteral*>(expr);
      const ScriptStructType* stype = static_cast<ScriptStructType*>(lit->resultType);

      const registerid propertyReg = ctx.acquireRegister();

      for (const ObjectLiteralProperty* prop : lit->properties) {
        const std::string_view propName = prop->propertyName->value->view();
        const ScriptType* ptype = nullptr;

        uint32 off = 0;

        for (uint32 i = 0; i < stype->getPropertyCount(); i++) {
          const StructProperty* typeProperty = stype->getProperty(i);
          if (typeProperty->name == propName) {
            ptype = typeProperty->type;
            break;
          }
          off += typeProperty->type->stackSizeBytes();
        }

        if (ptype->kind() == TK_STRUCT) {
          writer.startInstr(OP_READOBJ64);
          writer.appendU8(out);
          writer.appendU8(propertyReg);
          writer.appendU32(off);
          writer.endInstr();
        }
        compileExpr(prop->value, propertyReg, nullptr, ctx);

        if (ptype->kind() != TK_STRUCT) {
          BYTEWIDTH_OPCODE(ptype->stackSizeBytes(), writer, OP_WRITEOBJ)
          writer.appendU8(out);
          writer.appendU8(propertyReg);
          writer.appendU32(off);
          writer.endInstr();
        }
      }

      ctx.freeRegister(propertyReg);
      return;
    }

    case AST_UnaryExpr: {
      const UnaryExpr* un = static_cast<UnaryExpr*>(expr);
      const unaryop uop = un->op;

      switch (uop) {
        case UOP_POS:
          compileExpr(un->target, out, nullptr, ctx);
          return;
        case UOP_NEG: {
          compileExpr(un->target, out, nullptr, ctx);

          PrimitiveScriptType* primType = static_cast<PrimitiveScriptType*>(un->target->resultType);
          NUMTYPE_OPCODE(primType->getPrimitiveType(), writer, OP_NEG)
          writer.appendU8(out);
          writer.appendU8(out);
          writer.endInstr();
          return;
        }
        case UOP_BIT_NOT:
          compileExpr(un->target, out, nullptr, ctx);
          writer.startInstr(OP_BNEGATE);
          writer.appendU8(out);
          writer.appendU8(out);
          writer.endInstr();
          return;
        case UOP_LOG_NOT:
          compileExpr(un->target, out, nullptr, ctx);
          writer.startInstr(OP_LNEGATE);
          writer.appendU8(out);
          writer.appendU8(out);
          writer.endInstr();
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

      AddrOutput addrOut;
      registerid targetRegister;

      const bool isPostOp = uop == UOP_POSTINC || uop == UOP_POSTDEC;

      if (isPostOp) {
        targetRegister = ctx.acquireRegister();
      } else {
        targetRegister = out;
      }

      compileExpr(un->target, targetRegister, &addrOut, ctx);

      if (isPostOp) {
        writer.startInstr(OP_MOV);
        writer.appendU8(targetRegister);
        writer.appendU8(out);
        writer.endInstr();
      }

      PrimitiveScriptType* primType = static_cast<PrimitiveScriptType*>(un->target->resultType);
      const primitivekind pk = primType->getPrimitiveType();

      switch (uop) {
        case UOP_PREINC:
        case UOP_POSTINC:
          NUMTYPE_OPCODE(pk, writer, OP_INC)
          writer.appendU8(targetRegister);
          writer.appendU8(targetRegister);
          writer.endInstr();
          break;

        case UOP_PREDEC:
        case UOP_POSTDEC:
          NUMTYPE_OPCODE(pk, writer, OP_DEC)
          writer.appendU8(targetRegister);
          writer.appendU8(targetRegister);
          writer.endInstr();
          break;

        default:
          break;
      }

      compileWriteOperation(&addrOut, primType->stackSizeBytes(), ctx, targetRegister);
      return;
    }

    case AST_StringLiteral: {
      const StringLiteral* lit = static_cast<StringLiteral*>(expr);
      const StringPoolAddress off = ctx.getStringPool().emplace(lit->value);

      writer.startInstr(OP_LOADCONSTSTR);
      writer.appendU8(out);
      writer.appendU64(off);

      return;
    }
    case AST_FloatLiteral: {
      const PrimitiveScriptType* pst = static_cast<PrimitiveScriptType*>(expr->resultType);
      const primitivekind pk = pst->getPrimitiveType();
      const float64 val = static_cast<FloatLiteral*>(expr)->value;

      if (pk == PK_FLOAT32) {
        writer.startInstr(OP_LOADCONST32);
        writer.appendU8(out);
        writer.appendF32(static_cast<float32>(val));
        writer.endInstr();
      } else {
        writer.startInstr(OP_LOADCONST64);
        writer.appendU8(out);
        writer.appendF64(val);
      }

      return;
    }
    case AST_BooleanLiteral: {
      const bool val = static_cast<BooleanLiteral*>(expr)->value;
      writer.startInstr(OP_LOADCONST8);
      writer.appendU8(out);
      writer.appendU8(val);
      writer.endInstr();
      return;
    }
    case AST_IntLiteral: {
      const IntLiteral* il = static_cast<IntLiteral*>(expr);
      const PrimitiveScriptType* pst = static_cast<PrimitiveScriptType*>(expr->resultType);
      const primitivekind pk = pst->getPrimitiveType();

      switch (pk) {
        case PK_BOOL:
          writer.startInstr(OP_LOADCONST8);
          writer.appendU8(out);
          if (il->value) {
            writer.appendU8(1);
          } else {
            writer.appendU8(0);
          }
          writer.endInstr();
          break;

        case PK_INT8:
        case PK_UINT8:
          writer.startInstr(OP_LOADCONST8);
          writer.appendU8(out);
          writer.appendU8(il->value);
          writer.endInstr();
          break;

        case PK_UINT16:
        case PK_INT16:
          writer.startInstr(OP_LOADCONST16);
          writer.appendU8(out);
          writer.appendU16(il->value);
          writer.endInstr();
          break;

        case PK_UINT32:
        case PK_INT32:
          writer.startInstr(OP_LOADCONST32);
          writer.appendU8(out);
          writer.appendU32(il->value);
          writer.endInstr();
          break;

        case PK_UINT64:
        case PK_INT64:
          writer.startInstr(OP_LOADCONST64);
          writer.appendU8(out);
          writer.appendU64(il->value);
          break;

        case PK_FLOAT32:
          writer.startInstr(OP_LOADCONST32);
          writer.appendU8(out);
          writer.appendF32(static_cast<float32>(il->value));
          writer.endInstr();
          break;
        case PK_FLOAT64:
          writer.startInstr(OP_LOADCONST32);
          writer.appendU8(out);
          writer.appendF64(static_cast<float64>(il->value));
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

static void compileFuncCall(CompilerContext& ctx, const uint32 funcIdx, const registerid out) {
  BytecodeWriter& writer = ctx.getWriter();

  writer.startInstr(OP_FUNCLOOKUP);
  writer.appendU8(out);
  writer.appendU32(funcIdx);
  writer.endInstr();

  writer.startInstr(OP_INVOKE);
  writer.appendU8(out);
  writer.appendU8(out);
  writer.endInstr();
}

static void compileStructInitCall(ScriptStructType* type, CompilerContext& ctx, const registerid out) {
  BytecodeWriter& writer = ctx.getWriter();

  const uint32 funcTableIndex = ctx.getStructConstructorIndex(type);
  const uint64 heapSize = type->getHeapSize();

  writer.startInstr(OP_HEAPALLOC);
  writer.appendU8(out);
  writer.appendU64(heapSize);

  writer.startInstr(OP_PUSHARG);
  writer.appendU8(out);
  writer.endInstr();

  compileFuncCall(ctx, funcTableIndex, out);
}

static void compileValueInitialiser(ScriptType* type, CompilerContext& ctx, const registerid out) {
  BytecodeWriter& writer = ctx.getWriter();

  switch (type->kind()) {
    case TK_PRIMITIVE:
    case TK_STRING:
    case TK_ARRAY:
      BYTEWIDTH_OPCODE(type->stackSizeBytes(), writer, OP_LOADCONST)
      writer.appendU8(out);
      writer.appendU64(0);
      break;

    case TK_STRUCT:
      compileStructInitCall(static_cast<ScriptStructType*>(type), ctx, out);
      break;

    default:
      break;
  }
}

static uint64 measureBlock(const Block* block) {
  uint64 bsize = 0;
  for (Statement* stat : block->statements) {
    if (stat->nodeKind() != AST_LexicalDeclaration) {
      continue;
    }
    bsize += static_cast<LexicalDeclaration*>(stat)->typeExpr->referencedType->stackSizeBytes();
  }
  return bsize;
}

static uint32 blockBegin(BytecodeWriter& writer, uint64 bytes, uint64* addrOut) {
  const uint32 instr = writer.getInstructionCounter();

  writer.startInstr(OP_STACKALLOC);

  if (addrOut) {
    *addrOut = writer.getAddress();
  }

  writer.appendU64(bytes);
  writer.endInstr();

  return instr;
}

static void blockEnd(BytecodeWriter& writer, uint64 bytes) {
  writer.startInstr(OP_STACKFREE);
  writer.appendU64(bytes);
  writer.endInstr();

  writer.startInstr(OP_RET);
  writer.endInstr();
}

static void compileStatement(Statement* stat, CompilerContext& ctx);

static void compileBlock(const Block* block, CompilerContext& ctx, const uint64 size = 0) {
  blockBegin(ctx.getWriter(), size, nullptr);
  for (Statement* statement : block->statements) {
    compileStatement(statement, ctx);
  }
  blockEnd(ctx.getWriter(), size);
}

static void compileLexDecl(LexicalDeclaration* lex, CompilerContext& ctx) {
  ScriptType* stype = lex->typeExpr->referencedType;

  BytecodeWriter& writer = ctx.getWriter();

  const registerid valueReg = ctx.acquireRegister();
  compileRValue(stype, lex->value, ctx, valueReg);

  LocalVarSymbol* lvs = static_cast<LocalVarSymbol*>(ctx.getSemantics().getSymbolLookup()[lex]);

  BYTEWIDTH_OPCODE(lex->value->resultType->stackSizeBytes(), writer, OP_RSWRITE)
  writer.appendU8(valueReg);
  writer.appendU64(lvs->getStackOffset());

  ctx.freeRegister(valueReg);
}

static void compileIfStatement(const IfStatement* stat, CompilerContext& ctx) {
  const registerid reg = ctx.acquireRegister();
  BytecodeWriter& writer = ctx.getWriter();

  compileExpr(stat->condition, reg, nullptr, ctx);

  writer.startInstr(OP_JMPI0);
  const uint64 condFailedJumpAddr = writer.getAddress();

  writer.appendU32(0);
  writer.appendU8(reg);
  writer.endInstr();

  compileStatement(stat->body, ctx);

  if (stat->elseBody) {
    writer.startInstr(OP_JMP);
    const uint64 afterElseAddr = writer.getAddress();
    writer.appendU32(0);
    writer.endInstr();

    writer.writeInstructionCounter(condFailedJumpAddr);

    compileStatement(stat->elseBody, ctx);

    writer.writeInstructionCounter(afterElseAddr);
  } else {
    writer.writeInstructionCounter(condFailedJumpAddr);
  }
}

static void compileFuncDecl(const FunctionDeclStatement* stat, CompilerContext& ctx) {
  uint64 stackSize = 0;
  for (FunctionParam* p : stat->arguments) {
    stackSize += p->paramType->referencedType->stackSizeBytes();
  }
  stackSize += measureBlock(stat->functionBody);

  uint32 start = blockBegin(ctx.getWriter(), stackSize, nullptr);



  blockEnd(ctx.getWriter(), stackSize);
}

void compileStatement(Statement* stat, CompilerContext& ctx) {
  const astnodetype kind = stat->nodeKind();
  BytecodeWriter& writer = ctx.getWriter();

  switch (kind) {
    case AST_LexicalDeclaration:
      compileLexDecl(static_cast<LexicalDeclaration*>(stat), ctx);
      return;
    case AST_FunctionDeclStatement:
      ctx.enqueueFunction(static_cast<FunctionDeclStatement*>(stat));
      return;
    case AST_IfStatement:
      compileIfStatement(static_cast<IfStatement*>(stat), ctx);
      return;

    case AST_Block: {
      const Block* b = static_cast<Block*>(stat);
      const uint64 size = measureBlock(b);

      compileBlock(b, ctx, size);
      return;
    }

    default:
      return;
  }
}

void compileRValue(ScriptType* type, Expr* val, CompilerContext& ctx, registerid out) {
  if (!val) {
    compileValueInitialiser(type, ctx, out);
    return;
  }

  if (type->kind() == TK_STRUCT) {
    compileStructInitCall(static_cast<ScriptStructType*>(type), ctx, out);
  }

  compileExpr(val, out, nullptr, ctx);
}

static void compileStructDecl(const StructDecl* decl, CompilerContext& ctx) {
  ScriptStructType* stype = decl->type;
  BytecodeWriter& writer = ctx.getWriter();

  const uint32 propCount = decl->properties.size();
  constexpr uint64 stackSize = POINTER_SIZE;

  const uint32 start = blockBegin(writer, stackSize, nullptr);

  const registerid selfReg = ctx.acquireRegister();
  const registerid propReg = ctx.acquireRegister();

  writer.startInstr(OP_RSREAD64);
  writer.appendU8(selfReg);
  writer.appendU64(0);
  writer.endInstr();

  uint32 off = 0;
  for (uint32 i = 0; i < propCount; i++) {
    const StructPropertyDecl* propDecl = decl->properties.at(i);
    const StructProperty* ptype = stype->getProperty(i);

    compileRValue(ptype->type, propDecl->value, ctx, propReg);

    BYTEWIDTH_OPCODE(ptype->type->stackSizeBytes(), writer, OP_WRITEOBJ)
    writer.appendU8(selfReg);
    writer.appendU8(propReg);
    writer.appendU32(off);

    off += ptype->type->stackSizeBytes();
  }

  ctx.freeRegister(selfReg);
  ctx.freeRegister(propReg);

  blockEnd(writer, stackSize);

  std::string funcName;
  funcName.append(stype->getTypeName());
  funcName.append(".<init>");

  const stringid id = ctx.getSemantics().getStrings().allocate(funcName);

  ScriptType* ctorParams[1];
  ctorParams[0] = stype;

  FunctionSignature* sign = FunctionSignature::create(ConstTypes::VOID(), false, 1, ctorParams);
  ctx.getSemantics().getTypes().emplaceType(sign);

  const uint32 funcIdx = ctx.pushCompiledFunction(id, start, sign);

  ctx.pushStructConstructor(stype, funcIdx);
}

BytecodeFile compile(ScriptFileStatement* sfs, SemanticContext& ctx) {
  uint64 registerBitSet = 0;
  CompilerContext cctx = CompilerContext(ctx, &registerBitSet);

  constexpr uint64 initcap = LENGTH_INSTRUCTION * 1024;
  cctx.getWriter().reserveSpace(initcap);

  for (Statement* stat : sfs->statements) {
    if (stat->nodeKind() != AST_StructDecl) {
      continue;
    }
    compileStructDecl(static_cast<StructDecl*>(stat), cctx);
  }

  ctx.popScope();

  BytecodeFile file;
  file.instructionBuf = cctx.getWriter().getBuffer();
  file.instructionsSize = cctx.getWriter().getLength();
  file.constStringPool = cctx.getStringPool().getData();
  file.stringPoolSize = cctx.getStringPool().getLength();

  return file;
}
