#ifndef QUICKSCRIPT_OPCODES_H
#define QUICKSCRIPT_OPCODES_H

#include "../common.h"

#define LENGTH_OPCODE 2
#define LENGTH_ARGS 9
#define LENGTH_INSTRUCTION (LENGTH_OPCODE+LENGTH_ARGS)

//
// Section numbers that you see here are references to this:
// https://juliewoolie.com/blog/quickscript
// I wrote a bunch of the specification and planning there
//

// 4.3.1 General Purpose OP Codes
#define OP_NOP             0x0000
#define OP_PUSHLINE        0x0001
#define OP_RET             0x0002
#define OP_JMP             0x0003
#define OP_JMPI0           0x0004
#define OP_JMPN0           0x0005
#define OP_STACKALLOC      0x0006
#define OP_STACKFREE       0x0007
#define OP_SREAD           0x0008
#define OP_SWRITE          0x0009
#define OP_LOADCONST8      0x000A
#define OP_LOADCONST16     0x000B
#define OP_LOADCONST32     0x000C
#define OP_LOADCONST64     0x000D
#define OP_READOBJ         0x000E
#define OP_WRITEOBJ        0x000F
#define OP_HEAPALLOC       0x0010
#define OP_HEAPFREE        0x0011

// 4.3.2 Function Call Instructions
#define OP_PUSHARG         0x0012
#define OP_SETRV           0x0013
#define OP_INVOKE          0x0014

// 4.3.3 Conversion Instructions
#define OP_I8TU8           0x0015
#define OP_I8TI16          0x0016
#define OP_I8TU16          0x0017
#define OP_I8TI32          0x0018
#define OP_I8TU32          0x0019
#define OP_I8TI64          0x001A
#define OP_I8TU64          0x001B
#define OP_I8TF32          0x001C
#define OP_I8TF64          0x001D
#define OP_U8TI8           0x001E
#define OP_U8TI16          0x001F
#define OP_U8TU16          0x0020
#define OP_U8TI32          0x0021
#define OP_U8TU32          0x0022
#define OP_U8TI64          0x0023
#define OP_U8TU64          0x0024
#define OP_U8TF32          0x0025
#define OP_U8TF64          0x0026
#define OP_I16TI8          0x0027
#define OP_I16TU8          0x0028
#define OP_I16TU16         0x0029
#define OP_I16TI32         0x002A
#define OP_I16TU32         0x002B
#define OP_I16TI64         0x002C
#define OP_I16TU64         0x002D
#define OP_I16TF32         0x002E
#define OP_I16TF64         0x002F
#define OP_U16TI8          0x0030
#define OP_U16TU8          0x0031
#define OP_U16TI16         0x0032
#define OP_U16TI32         0x0033
#define OP_U16TU32         0x0034
#define OP_U16TI64         0x0035
#define OP_U16TU64         0x0036
#define OP_U16TF32         0x0037
#define OP_U16TF64         0x0038
#define OP_I32TI8          0x0039
#define OP_I32TU8          0x003A
#define OP_I32TI16         0x003B
#define OP_I32TU16         0x003C
#define OP_I32TU32         0x003D
#define OP_I32TI64         0x003E
#define OP_I32TU64         0x003F
#define OP_I32TF32         0x0040
#define OP_I32TF64         0x0041
#define OP_U32TI8          0x0042
#define OP_U32TU8          0x0043
#define OP_U32TI16         0x0044
#define OP_U32TU16         0x0045
#define OP_U32TI32         0x0046
#define OP_U32TI64         0x0047
#define OP_U32TU64         0x0048
#define OP_U32TF32         0x0049
#define OP_U32TF64         0x004A
#define OP_I64TI8          0x004B
#define OP_I64TU8          0x004C
#define OP_I64TI16         0x004D
#define OP_I64TU16         0x004E
#define OP_I64TI32         0x004F
#define OP_I64TU32         0x0050
#define OP_I64TU64         0x0051
#define OP_I64TF32         0x0052
#define OP_I64TF64         0x0053
#define OP_U64TI8          0x0054
#define OP_U64TU8          0x0055
#define OP_U64TI16         0x0056
#define OP_U64TU16         0x0057
#define OP_U64TI32         0x0058
#define OP_U64TU32         0x0059
#define OP_U64TI64         0x005A
#define OP_U64TF32         0x005B
#define OP_U64TF64         0x005C
#define OP_F32TI8          0x005D
#define OP_F32TU8          0x005E
#define OP_F32TI16         0x005F
#define OP_F32TU16         0x0060
#define OP_F32TI32         0x0061
#define OP_F32TU32         0x0062
#define OP_F32TI64         0x0063
#define OP_F32TU64         0x0064
#define OP_F32TF64         0x0065
#define OP_F64TI8          0x0066
#define OP_F64TU8          0x0067
#define OP_F64TI16         0x0068
#define OP_F64TU16         0x0069
#define OP_F64TI32         0x006A
#define OP_F64TU32         0x006B
#define OP_F64TI64         0x006C
#define OP_F64TU64         0x006D
#define OP_F64TF32         0x006E

// 4.3.4 Unary Operations
#define OP_BNEGATE         0x006F
#define OP_LNEGATE         0x0070

// 4.3.5.1 Integer-only Binary Operations
#define OP_LSHIFT8         0x0071
#define OP_LSHIFT16        0x0072
#define OP_LSHIFT32        0x0073
#define OP_LSHIFT64        0x0074
#define OP_URSHIFT8        0x0075
#define OP_URSHIFT16       0x0076
#define OP_URSHIFT32       0x0077
#define OP_URSHIFT64       0x0078

// 4.3.5.2 Boolean-only Binary Operations
#define OP_BAND            0x0079
#define OP_BOR             0x007A
#define OP_BXOR            0x007B
#define OP_LAND            0x007C
#define OP_LOR             0x007D
#define OP_LXOR            0x007E

// 4.3.5.3 General Number Binary Operations
#define OP_ADD_I8          0x007F
#define OP_ADD_U8          0x0080
#define OP_ADD_I16         0x0081
#define OP_ADD_U16         0x0082
#define OP_ADD_I32         0x0083
#define OP_ADD_U32         0x0084
#define OP_ADD_I64         0x0085
#define OP_ADD_U64         0x0086
#define OP_ADD_F32         0x0087
#define OP_ADD_F64         0x0088
#define OP_SUB_I8          0x0089
#define OP_SUB_U8          0x008A
#define OP_SUB_I16         0x008B
#define OP_SUB_U16         0x008C
#define OP_SUB_I32         0x008D
#define OP_SUB_U32         0x008E
#define OP_SUB_I64         0x008F
#define OP_SUB_U64         0x0090
#define OP_SUB_F32         0x0091
#define OP_SUB_F64         0x0092
#define OP_DIV_I8          0x0093
#define OP_DIV_U8          0x0094
#define OP_DIV_I16         0x0095
#define OP_DIV_U16         0x0096
#define OP_DIV_I32         0x0097
#define OP_DIV_U32         0x0098
#define OP_DIV_I64         0x0099
#define OP_DIV_U64         0x009A
#define OP_DIV_F32         0x009B
#define OP_DIV_F64         0x009C
#define OP_MUL_I8          0x009D
#define OP_MUL_U8          0x009E
#define OP_MUL_I16         0x009F
#define OP_MUL_U16         0x00A0
#define OP_MUL_I32         0x00A1
#define OP_MUL_U32         0x00A2
#define OP_MUL_I64         0x00A3
#define OP_MUL_U64         0x00A4
#define OP_MUL_F32         0x00A5
#define OP_MUL_F64         0x00A6
#define OP_MOD_I8          0x00A7
#define OP_MOD_U8          0x00A8
#define OP_MOD_I16         0x00A9
#define OP_MOD_U16         0x00AA
#define OP_MOD_I32         0x00AB
#define OP_MOD_U32         0x00AC
#define OP_MOD_I64         0x00AD
#define OP_MOD_U64         0x00AE
#define OP_MOD_F32         0x00AF
#define OP_MOD_F64         0x00B0
#define OP_EXP_I8          0x00B1
#define OP_EXP_U8          0x00B2
#define OP_EXP_I16         0x00B3
#define OP_EXP_U16         0x00B4
#define OP_EXP_I32         0x00B5
#define OP_EXP_U32         0x00B6
#define OP_EXP_I64         0x00B7
#define OP_EXP_U64         0x00B8
#define OP_EXP_F32         0x00B9
#define OP_EXP_F64         0x00BA

typedef uint16 opcode;

conststring opcode_name(opcode code);

#endif //QUICKSCRIPT_OPCODES_H