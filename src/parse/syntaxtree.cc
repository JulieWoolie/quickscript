
#include "syntaxtree.h"

conststring parsedprimitivetype_name(parsedprimitivetype pt) {
  switch (pt) {
    case PPT_NIL: return "NIL";
    case PPT_BOOL: return "BOOL";
    case PPT_UINT8: return "UINT8";
    case PPT_INT8: return "INT8";
    case PPT_UINT16: return "UINT16";
    case PPT_INT16: return "INT16";
    case PPT_UINT32: return "UINT32";
    case PPT_INT32: return "INT32";
    case PPT_UINT64: return "UINT64";
    case PPT_INT64: return "INT64";
    case PPT_FLOAT32: return "FLOAT32";
    case PPT_FLOAT64: return "FLOAT64";
    case PPT_STRING: return "STRING";
    default: return "UNKNOWN";
  }
}

conststring binaryop_name(binaryop op) {
  switch (op) {
    case BOP_NIL: return "BOP_NIL";
    case BOP_GT: return "BOP_GT";
    case BOP_LT: return "BOP_LT";
    case BOP_GTE: return "BOP_GTE";
    case BOP_LTE: return "BOP_LTE";
    case BOP_EQ: return "BOP_EQ";
    case BOP_NEQ: return "BOP_NEQ";
    case BOP_ADD: return "BOP_ADD";
    case BOP_SUB: return "BOP_SUB";
    case BOP_MUL: return "BOP_MUL";
    case BOP_DIV: return "BOP_DIV";
    case BOP_MOD: return "BOP_MOD";
    case BOP_EXP: return "BOP_EXP";
    case BOP_SHL: return "BOP_SHL";
    case BOP_SHR: return "BOP_SHR";
    case BOP_USHR: return "BOP_USHR";
    case BOP_XOR: return "BOP_XOR";
    case BOP_LOG_OR: return "BOP_LOG_OR";
    case BOP_LOG_AND: return "BOP_LOG_AND";
    case BOP_BIT_OR: return "BOP_BIT_OR";
    case BOP_BIT_AND: return "BOP_BIT_AND";
    case BOP_POW: return "BOP_POW";
    case BOP_ASSIGN_ADD: return "BOP_ASSIGN_ADD";
    case BOP_ASSIGN_SUB: return "BOP_ASSIGN_SUB";
    case BOP_ASSIGN_MUL: return "BOP_ASSIGN_MUL";
    case BOP_ASSIGN_DIV: return "BOP_ASSIGN_DIV";
    case BOP_ASSIGN_MOD: return "BOP_ASSIGN_MOD";
    case BOP_ASSIGN_EXP: return "BOP_ASSIGN_EXP";
    case BOP_ASSIGN_SHL: return "BOP_ASSIGN_SHL";
    case BOP_ASSIGN_SHR: return "BOP_ASSIGN_SHR";
    case BOP_ASSIGN_USHR: return "BOP_ASSIGN_USHR";
    case BOP_ASSIGN_XOR: return "BOP_ASSIGN_XOR";
    case BOP_ASSIGN_LOG_OR: return "BOP_ASSIGN_LOG_OR";
    case BOP_ASSIGN_BIT_OR: return "BOP_ASSIGN_BIT_OR";
    case BOP_ASSIGN_LOG_AND: return "BOP_ASSIGN_LOG_AND";
    case BOP_ASSIGN_BIT_AND: return "BOP_ASSIGN_BIT_AND";

    default:return "BOP_INVALID";
  }
}

conststring unaryop_name(unaryop op) {
  switch (op) {
    case UOP_NIL: return "UOP_NIL";
    case UOP_PREINC: return "UOP_PREINC";
    case UOP_POSTINC: return "UOP_POSTINC";
    case UOP_PREDEC: return "UOP_PREDEC";
    case UOP_POSTDEC: return "UOP_POSTDEC";
    case UOP_POS: return "UOP_POS";
    case UOP_NEG: return "UOP_NEG";
    case UOP_BIT_NOT: return "UOP_BIT_NOT";
    case UOP_LOG_NOT: return "UOP_LOG_NOT";
    default: return "UOP_INVALID";
  }
}

conststring controlflowtype_name(controlflowtype cft) {
  switch (cft) {
    case CFT_NIL: return "NIL";
    case CFT_CONTINUE: return "CONTINUE";
    case CFT_BREAK: return "BREAK";
    default: return "UNKNOWN";
  }
}
