#include "compiler.h"

#include "CompilerContext.h"
#include "../interpreter/interpreter.h"
#include "../interpreter/ir_file.h"
#include "../interpreter/opcodes.h"
#include "../types/ConstTypes.h"

#define RUNTIME_CHECKS

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
#define OUTP_GLOBAL 1
#define OUTP_STACK  2
#define OUTP_PROP   3
#define OUTP_IDX    4

struct AddrOutput {
  uint8 outptype = OUTP_NIL;
  registeridopt objectRegister = NO_REGISTER;
  registeridopt indexRegister = NO_REGISTER;
  uint32 memoffset = 0;
  int64 stackoffset = 0;
};

static void compileStoredExpr(ScriptType* type, Expr* expr, CompilerContext& ctx, uint64 off, bool local);

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
    case OUTP_STACK:
      BYTEWIDTH_OPCODE(stackSize, writer, OP_SWRITE)
      writer.appendU8(valueRegister);
      writer.appendU64(addrout->stackoffset);
      writer.endInstr();
      break;
    case OUTP_GLOBAL:
      BYTEWIDTH_OPCODE(stackSize, writer, OP_GWRITE)
      writer.appendU8(valueRegister);
      writer.appendU64(addrout->stackoffset);
      writer.endInstr();
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

static void compileFuncLookup(CompilerContext& ctx, LocalFuncSymbol* lfs, const registerid out) {
  BytecodeWriter& writer = ctx.getWriter();
  writer.startInstr(OP_LFUNCLOOKUP);

  const int32 fIdx = ctx.findFunctionIndex(lfs);

  if (fIdx == -1) {
    uint64 addr = writer.getAddress();
    ctx.pushIncompleteCall(lfs, addr);
    writer.appendU32(0);
  } else {
    writer.appendU32(fIdx);
  }

  writer.appendU8(out);
  writer.endInstr();
}

static void compileIdentifier(
  Identifier* id,
  const registeridopt out,
  AddrOutput* addr,
  CompilerContext& ctx
) {
  BytecodeWriter& writer = ctx.getWriter();
  SemanticContext& semantics = ctx.getSemantics();

  Symbol* sym = semantics.getSymbolLookup()[id];
  Scope* scope = semantics.getScopeLookup()[sym];

  if (sym->stype() == SYM_LocalFunc) {
    LocalFuncSymbol* lfs = static_cast<LocalFuncSymbol*>(sym);
    compileFuncLookup(ctx, lfs, out);
    return;
  }

  if (sym->stype() == SYM_LocalVar) {
    LocalVarSymbol* lvs = static_cast<LocalVarSymbol*>(sym);
    bool isMain = scope->getType() == SCOPE_MAIN;

    if (out != NO_REGISTER) {
      if (isMain) {
        BYTEWIDTH_OPCODE(lvs->getStackSize(), writer, OP_GREAD)
      } else {
        BYTEWIDTH_OPCODE(lvs->getStackSize(), writer, OP_SREAD)
      }

      writer.appendU8(out);
      writer.appendU64(lvs->getStackOffset());
      writer.endInstr();
    }

    if (addr) {
      addr->outptype = isMain ? OUTP_GLOBAL : OUTP_STACK;
      addr->memoffset = lvs->getStackOffset();
    }
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
        NUMTYPE_OPCODE(lpk, writer, OP_MUL)
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

  ctx.freeRegister(r2);

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

static void compileCallExpr(const CallExpr* call, const registerid out, CompilerContext& ctx) {
  Scope* scope = ctx.getCurrentScope();
  BytecodeWriter& writer = ctx.getWriter();

  uint64 offsetStart = scope->getStackSize();
  for (const Expr* arg : call->arguments) {
    offsetStart -= arg->resultType->stackSizeBytes();
  }

  registerid funcReg = ctx.acquireRegister();
  compileExpr(call->target, funcReg, nullptr, ctx);

  for (Expr* arg : call->arguments) {
    ScriptType* argType = arg->resultType;
    const uint64 size = argType->stackSizeBytes();
    compileStoredExpr(argType, arg, ctx, offsetStart, true);
    offsetStart += size;
  }

  if (call->resultType == ConstTypes::VOID()) {
    writer.startInstr(OP_INVOKEV);
  } else {
    BYTEWIDTH_OPCODE(call->resultType->stackSizeBytes(), writer, OP_INVOKE)
  }

  writer.appendU8(funcReg);
  writer.appendU8(out);
  writer.endInstr();

  ctx.freeRegister(funcReg);
}

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
    //  - X ObjectAllocExpr
    //  - X GetStackPointer

    case AST_CallExpr:
      compileCallExpr(static_cast<CallExpr*>(expr), out, ctx);
      return;

    case AST_GetStackPointer:
      writer.startInstr(OP_GETSTACKPTR);
      writer.appendU8(out);
      writer.endInstr();
      return;

    case AST_ObjectAllocExpr: {
      ObjectAllocExpr* allocExpr = static_cast<ObjectAllocExpr*>(expr);
      ScriptStructType* forType = static_cast<ScriptStructType*>(allocExpr->resultType);

      const uint32 propCount = forType->getPropertyCount();
      uint64 memSize = 0;

      for (uint32 i = 0; i < propCount; i++) {
        memSize += forType->getProperty(i)->type->stackSizeBytes();
      }

      writer.startInstr(OP_HEAPALLOC);
      writer.appendU8(out);
      writer.appendU64(memSize);
      writer.endInstr();

      return;
    }

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

      if (isPostOp) {
        ctx.freeRegister(targetRegister);
      }
      return;
    }

    case AST_StringLiteral: {
      const StringLiteral* lit = static_cast<StringLiteral*>(expr);
      const StringPoolAddress off = ctx.getStringPool().emplace(lit->value);

      writer.startInstr(OP_LOADCONSTSTR);
      writer.appendU8(out);
      writer.appendU64(off);
      writer.endInstr();

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


static void compileStructInitCall(ScriptStructType* type, CompilerContext& ctx, const registerid out) {
  SemanticContext& semantics = ctx.getSemantics();
  BytecodeWriter& writer = ctx.getWriter();

  LocalFuncSymbol* lfc = semantics.getConstructors()[type];
  compileFuncLookup(ctx, lfc, out);

  writer.startInstr(OP_INVOKE64);
  writer.appendU8(out);
  writer.appendU8(out);
  writer.endInstr();
}

static void compileStatement(Statement* stat, CompilerContext& ctx);

static void compileBlock(const Block* block, CompilerContext& ctx) {
  for (Statement* statement : block->statements) {
    ctx.setReturnCalled(false);
    compileStatement(statement, ctx);
  }
}

static void compileLexDecl(LexicalDeclaration* lex, CompilerContext& ctx) {
  ScriptType* stype = lex->typeExpr->referencedType;
  LocalVarSymbol* lvs = static_cast<LocalVarSymbol*>(ctx.getSemantics().getSymbolLookup()[lex]);
  compileStoredExpr(stype, lex->value, ctx, lvs->getStackOffset(), true);
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

    ctx.setReturnCalled(false);
    compileStatement(stat->elseBody, ctx);

    writer.writeInstructionCounter(afterElseAddr);
  } else {
    writer.writeInstructionCounter(condFailedJumpAddr);
  }

  ctx.freeRegister(reg);
}

static void compileLocalFunction(const LocalFunction* lf, CompilerContext& ctx) {
  FunctionDeclStatement* stat = lf->getDecl();
  LocalFuncSymbol* lfs = static_cast<LocalFuncSymbol*>(ctx.getSemantics().getSymbolLookup()[stat]);

  Scope* scope = lf->getScope();
  ctx.setCurrentScope(scope);

  BytecodeWriter& writer = ctx.getWriter();
  const uint32 start = writer.getInstructionCounter();
  const uint64 stackSize = scope->getStackSize();

  if (stackSize != 0) {
    writer.startInstr(OP_STACKALLOC);
    writer.appendU64(scope->getStackSize());
    writer.endInstr();
  }

  for (Statement* statement : stat->functionBody->statements) {
    compileStatement(statement, ctx);
  }

  if (!ctx.wasReturnCalled()) {
    if (stackSize != 0) {
      writer.startInstr(OP_STACKFREE);
      writer.appendU64(scope->getStackSize());
      writer.endInstr();
    }

    writer.startInstr(OP_RET);
    writer.endInstr();
  }

  ctx.setReturnCalled(false);
  ctx.pushCompiledFunction(lfs, start);
}

static void writeControlFlowAddresses(
  CompilerContext& ctx,
  const stringid label,
  const uint32 endInstr,
  const uint32 continueInstr
) {
  std::vector<ControlFlowCall>& cfCalls = ctx.getControlFlowCalls();
  const BytecodeWriter& writer = ctx.getWriter();

  for (auto it = cfCalls.begin(); it != cfCalls.end(); ) {
    ControlFlowCall& call = *it;

    if (call.label && call.label != label) {
      continue;
    }

    if (call.type == CFT_BREAK) {
      writer.writeInstructionCounter(call.writeAddress, endInstr);
    } else {
      writer.writeInstructionCounter(call.writeAddress, continueInstr);
    }
  }
}

static void compileForLoop(ForStatement* loop, CompilerContext& ctx) {
  BytecodeWriter& writer = ctx.getWriter();

  compileLexDecl(loop->first, ctx);

  const registerid conditionReg = ctx.acquireRegister();
  const registerid thirdReg = ctx.acquireRegister();

  const uint64 conditionInstr = writer.getInstructionCounter();

  compileExpr(loop->second, conditionReg, nullptr, ctx);

  writer.startInstr(OP_JMPI0);
  const uint64 endWriteAddr = writer.getAddress();
  writer.appendU32(0);
  writer.appendU8(conditionReg);
  writer.endInstr();

  compileStatement(loop->loopBody, ctx);
  compileExpr(loop->third, thirdReg, nullptr, ctx);

  const uint64 endInstr = writer.getInstructionCounter();
  writer.writeInstructionCounter(endWriteAddr, endInstr);

  const stringid label = loop->label ? loop->label->value : nullptr;
  writeControlFlowAddresses(ctx, label, endInstr, conditionInstr);

  ctx.freeRegister(thirdReg);
  ctx.freeRegister(conditionReg);
}

static void compileWhileLoop(WhileStatement* loop, CompilerContext& ctx) {
  registerid conditionRegister = ctx.acquireRegister();
  uint64 endWriteAddr = 0;
  BytecodeWriter& writer = ctx.getWriter();

  const bool doWhile = loop->doWhile;

  if (!doWhile) {
    compileExpr(loop->condition, conditionRegister, nullptr, ctx);

    writer.startInstr(OP_JMPI0);
    endWriteAddr = writer.getAddress();
    writer.appendU32(0);
    writer.appendU8(conditionRegister);
    writer.endInstr();
  }

  const uint32 startInstr = writer.getInstructionCounter();

  compileStatement(loop->body, ctx);

  const uint32 endInstr = writer.getInstructionCounter();

  stringid label = loop->label ? loop->label->value : nullptr;
  writeControlFlowAddresses(ctx, label, endInstr, startInstr);

  if (doWhile) {
    compileExpr(loop->condition, conditionRegister, nullptr, ctx);

    writer.startInstr(OP_JMPN0);
    writer.appendU32(startInstr);
    writer.appendU8(conditionRegister);
    writer.endInstr();
  } else {
    writer.writeInstructionCounter(endWriteAddr, endInstr);
  }

  ctx.freeRegister(conditionRegister);
}

static void compileControlFlow(ControlFlowStatement* cft, CompilerContext& ctx) {
  BytecodeWriter& writer = ctx.getWriter();

  writer.startInstr(OP_JMP);
  const uint64 addr = writer.getAddress();
  writer.appendU32(0);
  writer.endInstr();

  std::vector<ControlFlowCall>& cfCalls = ctx.getControlFlowCalls();
  cfCalls.emplace_back(cft->type, nullptr, addr);
}

static void compileReturn(const ReturnStatement* ret, CompilerContext& ctx) {
  BytecodeWriter& writer = ctx.getWriter();

  if (ret->value) {
    compileStoredExpr(ret->value->resultType, ret->value, ctx, 0, true);
  }

  Scope* scope = ctx.getCurrentScope();
  writer.startInstr(OP_STACKFREE);
  writer.appendU64(scope->getStackSize());
  writer.endInstr();

  writer.startInstr(OP_RET);
  writer.endInstr();

  ctx.setReturnCalled(true);
}

static void compileStatement(Statement* stat, CompilerContext& ctx) {
  const astnodetype kind = stat->nodeKind();

  BytecodeWriter& writer = ctx.getWriter();
  writer.startInstr(OP_PUSHLINE);
  writer.appendU32(stat->location.line);
  writer.endInstr();

  switch (kind) {
    case AST_Block:
      compileBlock(static_cast<Block*>(stat), ctx);
      return;
    case AST_IfStatement:
      compileIfStatement(static_cast<IfStatement*>(stat), ctx);
      return;
    case AST_ForStatement:
      compileForLoop(static_cast<ForStatement*>(stat), ctx);
      return;
    case AST_LexicalDeclaration:
      compileLexDecl(static_cast<LexicalDeclaration*>(stat), ctx);
      return;
    case AST_WhileStatement:
      compileWhileLoop(static_cast<WhileStatement*>(stat), ctx);
      return;
    case AST_ControlFlowStatement:
      compileControlFlow(static_cast<ControlFlowStatement*>(stat), ctx);
      return;
    case AST_ReturnStatement:
      compileReturn(static_cast<ReturnStatement*>(stat), ctx);
      return;
    case AST_ExprStatement: {
      const registerid resultRegister = ctx.acquireRegister();
      compileExpr(static_cast<ExprStatement*>(stat)->expression, resultRegister, nullptr, ctx);
      ctx.freeRegister(resultRegister);
      return;
    }
    default:
      return;
  }
}

void compileRValue(ScriptType* type, Expr* val, CompilerContext& ctx, registerid out) {
  if (type->kind() == TK_STRUCT && (!val || val->nodeKind() == AST_ObjectLiteral)) {
    compileStructInitCall(static_cast<ScriptStructType*>(type), ctx, out);
  }
  compileExpr(val, out, nullptr, ctx);
}

static bool isStoreableLiteral(Expr* e) {
  switch (e->nodeKind()) {
    case AST_IntLiteral:
    case AST_FloatLiteral:
    case AST_BooleanLiteral:
      return true;
    default:
      return false;
  }
}

static void compileStoredExpr(ScriptType* type, Expr* expr, CompilerContext& ctx, const uint64 off, const bool local) {
  BytecodeWriter& writer = ctx.getWriter();

  if (type->kind() == TK_PRIMITIVE && expr && isStoreableLiteral(expr)) {
    PrimitiveScriptType* prim = static_cast<PrimitiveScriptType*>(type);

    if (local) {
      BYTEWIDTH_OPCODE(type->stackSizeBytes(), writer, OP_STORECONST)
    } else {
      BYTEWIDTH_OPCODE(type->stackSizeBytes(), writer, OP_GSTORECONST)
    }

    writer.appendU32(off);

    switch (expr->nodeKind()) {
      case AST_IntLiteral: {
        IntLiteral* il = static_cast<IntLiteral*>(expr);
        switch (prim->stackSizeBytes()) {
          case 1:
            writer.appendU8(il->value);
            break;
          case 2:
            writer.appendU16(il->value);
            break;
          case 4:
            writer.appendU32(il->value);
            break;
          default:
            writer.appendU64(il->value);
            break;
        }
        break;
      }
      case AST_FloatLiteral: {
        FloatLiteral* fl = static_cast<FloatLiteral*>(expr);
        if (prim->stackSizeBytes() == 4) {
          writer.appendF32(fl->value);
        } else {
          writer.appendF64(fl->value);
        }
        break;
      }
      case AST_BooleanLiteral: {
        BooleanLiteral* bl = static_cast<BooleanLiteral*>(expr);
        writer.appendU8(bl->value ? 1 : 0);
        break;
      }
      default:
        break;
    }

    writer.endInstr();
    return;
  }

  const registerid valueReg = ctx.acquireRegister();
  compileRValue(type, expr, ctx, valueReg);

  if (local) {
    BYTEWIDTH_OPCODE(type->stackSizeBytes(), writer, OP_SWRITE)
  } else {
    BYTEWIDTH_OPCODE(type->stackSizeBytes(), writer, OP_GWRITE)
  }

  writer.appendU8(valueReg);
  writer.appendU64(off);
  writer.endInstr();

  ctx.freeRegister(valueReg);
}

static void createTypeTable(BytecodeFile& out, CompilerContext& ctx) {
  const TypeTable& types = ctx.getSemantics().getTypes();
  const uint64 size = types.size() - (LAST_RESERVED_TYPE_INDEX + 1);

  if (size == 0) {
    out.typeTable = nullptr;
    out.typeTableSize = 0;
    return;
  }

  TypeTableEntry** entries = static_cast<TypeTableEntry**>(malloc(size * sizeof(TypeTableEntry*)));
  ConstStringPoolWriter& stringPool = ctx.getStringPool();

  for (uint32 i = 0; i < size; i++) {
    const typeindex typeIdx = i + LAST_RESERVED_TYPE_INDEX + 1;
    ScriptType* type = types.lookupByIndex(typeIdx);

    switch (type->kind()) {
      case TK_STRUCT: {
        ScriptStructType* structType = static_cast<ScriptStructType*>(type);
        const std::string& name = structType->getNameString();
        const uint32 propCount = structType->getPropertyCount();

        TypeTableStruct* ttStruct = new TypeTableStruct();
        ttStruct->type = TYPE_TABLE_STRUCT;
        ttStruct->index = typeIdx;
        ttStruct->nameOffset = stringPool.emplaceString(name.data(), name.length());
        ttStruct->propertyCount = propCount;

        LocalFuncSymbol* lfs = ctx.getSemantics().getConstructors()[structType];
        const uint32 fIdx = ctx.findFunctionIndex(lfs);
        ttStruct->constructorFuncIndex = fIdx;

        TypeTableStructProperty* props = new TypeTableStructProperty[propCount];
        uint64 off = 0;

        for (uint32 j = 0; j < propCount; j++) {
          const StructProperty* prop = structType->getProperty(j);
          const typeindex propType = types.findIndex(prop->type);
          props[j] = {
            .nameOffset = stringPool.emplaceString(prop->name.data(), prop->name.length()),
            .valueOffset = off,
            .type = propType
          };
          off += prop->type->stackSizeBytes();
        }

        ttStruct->properties = props;
        entries[i] = ttStruct;
        break;
      }
      case TK_ARRAY: {
        ScriptArrayType* arrType = static_cast<ScriptArrayType*>(type);
        const ScriptType* componentType = arrType->getComponentType();
        const typeindex cTypeIdx = types.findIndex(componentType);

        TypeTableArray* arr = new TypeTableArray();
        arr->type = TYPE_TABLE_ARRAY;
        arr->index = typeIdx;
        arr->componentType = cTypeIdx;

        entries[i] = arr;
        break;
      }
      case TK_FUNC: {
        FunctionSignature* sign = static_cast<FunctionSignature*>(type);
        TypeTableFuncSign* tableSign = new TypeTableFuncSign();
        tableSign->type = TYPE_TABLE_SIGNATURE;
        tableSign->index = typeIdx;

        const typeindex retType = types.findIndex(sign->getReturnType());
        const uint32 argCount = sign->getArgumentsLength();

        typeindex* argIndexes = new typeindex[argCount];
        for (uint32 j = 0; j < argCount; j++) {
          const typeindex argIndex = types.findIndex(sign->getArgumentType(j));
          argIndexes[j] = argIndex;
        }

        tableSign->varargs = sign->isVariadic();
        tableSign->returnType = retType;
        tableSign->argTypes = argIndexes;
        tableSign->argumentCount = argCount;

        entries[i] = tableSign;
        break;
      }

      default:
        break;
    }
  }

  out.typeTableSize = size;
  out.typeTable = entries;
}

static void createFunctionTable(BytecodeFile& out, CompilerContext& ctx) {
  std::vector<CompiledFunction>& compiledFuncs = ctx.getCompiledFunctions();
  const TypeTable& types = ctx.getSemantics().getTypes();

  FunctionTableEntry* table = static_cast<FunctionTableEntry*>(malloc(sizeof(FunctionTableEntry) * compiledFuncs.size()));

  for (uint32 i = 0; i < compiledFuncs.size(); i++) {
    CompiledFunction& cfunc = compiledFuncs[i];
    const uint64 tIndex = types.findIndex(cfunc.functionSymbol->getFunction()->getSignature());

    table[i] = {
      .nameOffset = cfunc.poolId,
      .signatureIndex = tIndex,
      .startingInstruction = cfunc.bodyStart
    };
  }

  out.funcTable = table;
  out.funcTableEntries = compiledFuncs.size();
}

BytecodeFile compile(SemanticContext& ctx) {
  uint64 registerBitSet = 0;
  CompilerContext cctx = CompilerContext(ctx, &registerBitSet);

  constexpr uint64 initcap = LENGTH_INSTRUCTION * 1024;
  cctx.getWriter().reserveSpace(initcap);

  for (LocalFunction* lf : ctx.getLocalFunctions()) {
    compileLocalFunction(lf, cctx);
  }

#ifdef RUNTIME_CHECKS
  if (registerBitSet != 0) {
    throw std::runtime_error("Registers not freed after being compiled");
  }
#endif

  BytecodeFile file;

  createTypeTable(file, cctx);
  createFunctionTable(file, cctx);

  file.instructionBuf = cctx.getWriter().getBuffer();
  file.instructionsSize = cctx.getWriter().getLength();
  file.constStringPool = cctx.getStringPool().getData();
  file.stringPoolSize = cctx.getStringPool().getLength();
  file.globalScopeSize = ctx.getGlobalScope()->getStackSize();

  return file;
}
