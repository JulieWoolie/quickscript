# OP Code specification Table
| OP Code | Padding | Arguments |
|--|--|--|
|`NOP`|9||
|`PUSHLINE`|5|`lineno: uint32`|
|`RET`|9||
|`JMP`|5|`to: uint32`|
|`JMPI0`|4|`to: uint32`, `condition: register`|
|`JMPN0`|4|`to: uint32`, `condition: register`|
|`MOV`|7|`from: register`, `to: register`|
|`LOADCONST8`|7|`out: register`, `val: uint8`|
|`LOADCONST16`|6|`out: register`, `val: uint16`|
|`LOADCONST32`|4|`out: register`, `val: uint32`|
|`LOADCONST64`|2|`out: register`, `val: uint64`|
|`LOADCONSTSTR`|0|`out: register`, `straddr: uint64`|
|`STACKALLOC`|1|`bytes: uint64`|
|`STACKFREE`|1|`bytes: uint64`|
|`RSREAD8`|0|`out: register`, `offset: uint64`|
|`RSREAD16`|0|`out: register`, `offset: uint64`|
|`RSREAD32`|0|`out: register`, `offset: uint64`|
|`RSREAD64`|0|`out: register`, `offset: uint64`|
|`RSWRITE8`|0|`val: register`, `offset: uint64`|
|`RSWRITE16`|0|`val: register`, `offset: uint64`|
|`RSWRITE32`|0|`val: register`, `offset: uint64`|
|`RSWRITE64`|0|`val: register`, `offset: uint64`|
|`ASREAD8`|0|`out: register`, `offset: uint64`|
|`ASREAD16`|0|`out: register`, `offset: uint64`|
|`ASREAD32`|0|`out: register`, `offset: uint64`|
|`ASREAD64`|0|`out: register`, `offset: uint64`|
|`ASWRITE8`|0|`val: register`, `offset: uint64`|
|`ASWRITE16`|0|`val: register`, `offset: uint64`|
|`ASWRITE32`|0|`val: register`, `offset: uint64`|
|`ASWRITE64`|0|`val: register`, `offset: uint64`|
|`HEAPALLOC`|0|`out: register`, `bytes: uint64`|
|`HEAPFREE`|0|`addr: register`, `bytes: uint64`|
|`READOBJ8`|3|`obj: register`, `out: register`, `off: uint32`|
|`READOBJ16`|3|`obj: register`, `out: register`, `off: uint32`|
|`READOBJ32`|3|`obj: register`, `out: register`, `off: uint32`|
|`READOBJ64`|3|`obj: register`, `out: register`, `off: uint32`|
|`WRITEOBJ8`|3|`obj: register`, `val: register`, `off: uint32`|
|`WRITEOBJ16`|3|`obj: register`, `val: register`, `off: uint32`|
|`WRITEOBJ32`|3|`obj: register`, `val: register`, `off: uint32`|
|`WRITEOBJ64`|3|`obj: register`, `val: register`, `off: uint32`|
|`READIDX8`|6|`obj: register`, `out: register`, `idx: register`|
|`READIDX16`|6|`obj: register`, `out: register`, `idx: register`|
|`READIDX32`|6|`obj: register`, `out: register`, `idx: register`|
|`READIDX64`|6|`obj: register`, `out: register`, `idx: register`|
|`WRITEIDX8`|6|`obj: register`, `val: register`, `idx: register`|
|`WRITEIDX16`|6|`obj: register`, `val: register`, `idx: register`|
|`WRITEIDX32`|6|`obj: register`, `val: register`, `idx: register`|
|`WRITEIDX64`|6|`obj: register`, `val: register`, `idx: register`|
|`PUSHARG`|8|`val: register`|
|`SETRV`|8|`val: register`|
|`INVOKE`|7|`val: register`, `out: register`|
|`VINVOKE`|8|`val: register`|
|`I8TU8`|7|`from: register`, `out: register`|
|`I8TI16`|7|`from: register`, `out: register`|
|`I8TU16`|7|`from: register`, `out: register`|
|`I8TI32`|7|`from: register`, `out: register`|
|`I8TU32`|7|`from: register`, `out: register`|
|`I8TI64`|7|`from: register`, `out: register`|
|`I8TU64`|7|`from: register`, `out: register`|
|`I8TF32`|7|`from: register`, `out: register`|
|`I8TF64`|7|`from: register`, `out: register`|
|`U8TI8`|7|`from: register`, `out: register`|
|`U8TI16`|7|`from: register`, `out: register`|
|`U8TU16`|7|`from: register`, `out: register`|
|`U8TI32`|7|`from: register`, `out: register`|
|`U8TU32`|7|`from: register`, `out: register`|
|`U8TI64`|7|`from: register`, `out: register`|
|`U8TU64`|7|`from: register`, `out: register`|
|`U8TF32`|7|`from: register`, `out: register`|
|`U8TF64`|7|`from: register`, `out: register`|
|`I16TI8`|7|`from: register`, `out: register`|
|`I16TU8`|7|`from: register`, `out: register`|
|`I16TU16`|7|`from: register`, `out: register`|
|`I16TI32`|7|`from: register`, `out: register`|
|`I16TU32`|7|`from: register`, `out: register`|
|`I16TI64`|7|`from: register`, `out: register`|
|`I16TU64`|7|`from: register`, `out: register`|
|`I16TF32`|7|`from: register`, `out: register`|
|`I16TF64`|7|`from: register`, `out: register`|
|`U16TI8`|7|`from: register`, `out: register`|
|`U16TU8`|7|`from: register`, `out: register`|
|`U16TI16`|7|`from: register`, `out: register`|
|`U16TI32`|7|`from: register`, `out: register`|
|`U16TU32`|7|`from: register`, `out: register`|
|`U16TI64`|7|`from: register`, `out: register`|
|`U16TU64`|7|`from: register`, `out: register`|
|`U16TF32`|7|`from: register`, `out: register`|
|`U16TF64`|7|`from: register`, `out: register`|
|`I32TI8`|7|`from: register`, `out: register`|
|`I32TU8`|7|`from: register`, `out: register`|
|`I32TI16`|7|`from: register`, `out: register`|
|`I32TU16`|7|`from: register`, `out: register`|
|`I32TU32`|7|`from: register`, `out: register`|
|`I32TI64`|7|`from: register`, `out: register`|
|`I32TU64`|7|`from: register`, `out: register`|
|`I32TF32`|7|`from: register`, `out: register`|
|`I32TF64`|7|`from: register`, `out: register`|
|`U32TI8`|7|`from: register`, `out: register`|
|`U32TU8`|7|`from: register`, `out: register`|
|`U32TI16`|7|`from: register`, `out: register`|
|`U32TU16`|7|`from: register`, `out: register`|
|`U32TI32`|7|`from: register`, `out: register`|
|`U32TI64`|7|`from: register`, `out: register`|
|`U32TU64`|7|`from: register`, `out: register`|
|`U32TF32`|7|`from: register`, `out: register`|
|`U32TF64`|7|`from: register`, `out: register`|
|`I64TI8`|7|`from: register`, `out: register`|
|`I64TU8`|7|`from: register`, `out: register`|
|`I64TI16`|7|`from: register`, `out: register`|
|`I64TU16`|7|`from: register`, `out: register`|
|`I64TI32`|7|`from: register`, `out: register`|
|`I64TU32`|7|`from: register`, `out: register`|
|`I64TU64`|7|`from: register`, `out: register`|
|`I64TF32`|7|`from: register`, `out: register`|
|`I64TF64`|7|`from: register`, `out: register`|
|`U64TI8`|7|`from: register`, `out: register`|
|`U64TU8`|7|`from: register`, `out: register`|
|`U64TI16`|7|`from: register`, `out: register`|
|`U64TU16`|7|`from: register`, `out: register`|
|`U64TI32`|7|`from: register`, `out: register`|
|`U64TU32`|7|`from: register`, `out: register`|
|`U64TI64`|7|`from: register`, `out: register`|
|`U64TF32`|7|`from: register`, `out: register`|
|`U64TF64`|7|`from: register`, `out: register`|
|`F32TI8`|7|`from: register`, `out: register`|
|`F32TU8`|7|`from: register`, `out: register`|
|`F32TI16`|7|`from: register`, `out: register`|
|`F32TU16`|7|`from: register`, `out: register`|
|`F32TI32`|7|`from: register`, `out: register`|
|`F32TU32`|7|`from: register`, `out: register`|
|`F32TI64`|7|`from: register`, `out: register`|
|`F32TU64`|7|`from: register`, `out: register`|
|`F32TF64`|7|`from: register`, `out: register`|
|`F64TI8`|7|`from: register`, `out: register`|
|`F64TU8`|7|`from: register`, `out: register`|
|`F64TI16`|7|`from: register`, `out: register`|
|`F64TU16`|7|`from: register`, `out: register`|
|`F64TI32`|7|`from: register`, `out: register`|
|`F64TU32`|7|`from: register`, `out: register`|
|`F64TI64`|7|`from: register`, `out: register`|
|`F64TU64`|7|`from: register`, `out: register`|
|`F64TF32`|7|`from: register`, `out: register`|
|`BNEGATE`|7|`in: register`, `out: register`|
|`LNEGATE`|7|`in: register`, `out: register`|
|`NEGI8`|7|`in: register`, `out: register`|
|`NEGU8`|7|`in: register`, `out: register`|
|`NEGI16`|7|`in: register`, `out: register`|
|`NEGU16`|7|`in: register`, `out: register`|
|`NEGI32`|7|`in: register`, `out: register`|
|`NEGU32`|7|`in: register`, `out: register`|
|`NEGI64`|7|`in: register`, `out: register`|
|`NEGU64`|7|`in: register`, `out: register`|
|`NEGF32`|7|`in: register`, `out: register`|
|`NEGF64`|7|`in: register`, `out: register`|
|`INCI8`|7|`in: register`, `out: register`|
|`INCU8`|7|`in: register`, `out: register`|
|`INCI16`|7|`in: register`, `out: register`|
|`INCU16`|7|`in: register`, `out: register`|
|`INCI32`|7|`in: register`, `out: register`|
|`INCU32`|7|`in: register`, `out: register`|
|`INCI64`|7|`in: register`, `out: register`|
|`INCU64`|7|`in: register`, `out: register`|
|`INCF32`|7|`in: register`, `out: register`|
|`INCF64`|7|`in: register`, `out: register`|
|`DECI8`|7|`in: register`, `out: register`|
|`DECU8`|7|`in: register`, `out: register`|
|`DECI16`|7|`in: register`, `out: register`|
|`DECU16`|7|`in: register`, `out: register`|
|`DECI32`|7|`in: register`, `out: register`|
|`DECU32`|7|`in: register`, `out: register`|
|`DECI64`|7|`in: register`, `out: register`|
|`DECU64`|7|`in: register`, `out: register`|
|`DECF32`|7|`in: register`, `out: register`|
|`DECF64`|7|`in: register`, `out: register`|
|`LSHIFT8`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LSHIFT16`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LSHIFT32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LSHIFT64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`URSHIFT8`|6|`lhs: register`, `rhs: register`, `out: register`|
|`URSHIFT16`|6|`lhs: register`, `rhs: register`, `out: register`|
|`URSHIFT32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`URSHIFT64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`BAND`|6|`lhs: register`, `rhs: register`, `out: register`|
|`BOR`|6|`lhs: register`, `rhs: register`, `out: register`|
|`BXOR`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LAND`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LOR`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LXOR`|6|`lhs: register`, `rhs: register`, `out: register`|
|`ADD_I8`|6|`lhs: register`, `rhs: register`, `out: register`|
|`ADD_U8`|6|`lhs: register`, `rhs: register`, `out: register`|
|`ADD_I16`|6|`lhs: register`, `rhs: register`, `out: register`|
|`ADD_U16`|6|`lhs: register`, `rhs: register`, `out: register`|
|`ADD_I32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`ADD_U32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`ADD_I64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`ADD_U64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`ADD_F32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`ADD_F64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`SUB_I8`|6|`lhs: register`, `rhs: register`, `out: register`|
|`SUB_U8`|6|`lhs: register`, `rhs: register`, `out: register`|
|`SUB_I16`|6|`lhs: register`, `rhs: register`, `out: register`|
|`SUB_U16`|6|`lhs: register`, `rhs: register`, `out: register`|
|`SUB_I32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`SUB_U32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`SUB_I64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`SUB_U64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`SUB_F32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`SUB_F64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`DIV_I8`|6|`lhs: register`, `rhs: register`, `out: register`|
|`DIV_U8`|6|`lhs: register`, `rhs: register`, `out: register`|
|`DIV_I16`|6|`lhs: register`, `rhs: register`, `out: register`|
|`DIV_U16`|6|`lhs: register`, `rhs: register`, `out: register`|
|`DIV_I32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`DIV_U32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`DIV_I64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`DIV_U64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`DIV_F32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`DIV_F64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MUL_I8`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MUL_U8`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MUL_I16`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MUL_U16`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MUL_I32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MUL_U32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MUL_I64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MUL_U64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MUL_F32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MUL_F64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MOD_I8`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MOD_U8`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MOD_I16`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MOD_U16`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MOD_I32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MOD_U32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MOD_I64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MOD_U64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MOD_F32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MOD_F64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`POW_I8`|6|`lhs: register`, `rhs: register`, `out: register`|
|`POW_U8`|6|`lhs: register`, `rhs: register`, `out: register`|
|`POW_I16`|6|`lhs: register`, `rhs: register`, `out: register`|
|`POW_U16`|6|`lhs: register`, `rhs: register`, `out: register`|
|`POW_I32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`POW_U32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`POW_I64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`POW_U64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`POW_F32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`POW_F64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`EQ8`|6|`lhs: register`, `rhs: register`, `out: register`|
|`EQ16`|6|`lhs: register`, `rhs: register`, `out: register`|
|`EQ32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`EQ64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`EQARR`|6|`lhs: register`, `rhs: register`, `out: register`|
|`EQSTRUCT`|6|`lhs: register`, `rhs: register`, `out: register`|
|`NEQ8`|6|`lhs: register`, `rhs: register`, `out: register`|
|`NEQ16`|6|`lhs: register`, `rhs: register`, `out: register`|
|`NEQ32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`NEQ64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`NEQARR`|6|`lhs: register`, `rhs: register`, `out: register`|
|`NEQSTRUCT`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTI8`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTU8`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTI16`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTU16`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTI32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTU32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTI64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTU64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTF32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTF64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTARR`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTEI8`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTEU8`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTEI16`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTEU16`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTEI32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTEU32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTEI64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTEU64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTEF32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTEF64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTEARR`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTI8`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTU8`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTI16`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTU16`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTI32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTU32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTI64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTU64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTF32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTF64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTARR`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTEI8`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTEU8`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTEI16`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTEU16`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTEI32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTEU32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTEI64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTEU64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTEF32`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTEF64`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTEARR`|6|`lhs: register`, `rhs: register`, `out: register`|