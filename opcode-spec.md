# OP Code specification Table
| OP Code | Value | Padding | Arguments |
|--|--|--|--|
|`NOP`|`0x0000`|9||
|`PUSHLINE`|`0x0001`|5|`lineno: uint32`|
|`RET`|`0x0002`|9||
|`JMP`|`0x0003`|5|`to: uint32`|
|`JMPI0`|`0x0004`|4|`to: uint32`, `condition: register`|
|`JMPN0`|`0x0005`|4|`to: uint32`, `condition: register`|
|`MOV`|`0x0006`|7|`from: register`, `to: register`|
|`LOADCONST8`|`0x0007`|7|`out: register`, `val: uint8`|
|`LOADCONST16`|`0x0008`|6|`out: register`, `val: uint16`|
|`LOADCONST32`|`0x0009`|4|`out: register`, `val: uint32`|
|`LOADCONST64`|`0x000A`|2|`out: register`, `val: uint64`|
|`LOADCONSTSTR`|`0x000B`|0|`out: register`, `straddr: uint64`|
|`STACKALLOC`|`0x000C`|1|`bytes: uint64`|
|`STACKFREE`|`0x000D`|1|`bytes: uint64`|
|`RSREAD8`|`0x000E`|0|`out: register`, `offset: uint64`|
|`RSREAD16`|`0x000F`|0|`out: register`, `offset: uint64`|
|`RSREAD32`|`0x0010`|0|`out: register`, `offset: uint64`|
|`RSREAD64`|`0x0011`|0|`out: register`, `offset: uint64`|
|`RSWRITE8`|`0x0012`|0|`val: register`, `offset: uint64`|
|`RSWRITE16`|`0x0013`|0|`val: register`, `offset: uint64`|
|`RSWRITE32`|`0x0014`|0|`val: register`, `offset: uint64`|
|`RSWRITE64`|`0x0015`|0|`val: register`, `offset: uint64`|
|`ASREAD8`|`0x0016`|0|`out: register`, `offset: uint64`|
|`ASREAD16`|`0x0017`|0|`out: register`, `offset: uint64`|
|`ASREAD32`|`0x0018`|0|`out: register`, `offset: uint64`|
|`ASREAD64`|`0x0019`|0|`out: register`, `offset: uint64`|
|`ASWRITE8`|`0x001A`|0|`val: register`, `offset: uint64`|
|`ASWRITE16`|`0x001B`|0|`val: register`, `offset: uint64`|
|`ASWRITE32`|`0x001C`|0|`val: register`, `offset: uint64`|
|`ASWRITE64`|`0x001D`|0|`val: register`, `offset: uint64`|
|`HEAPALLOC`|`0x001E`|0|`out: register`, `bytes: uint64`|
|`HEAPFREE`|`0x001F`|0|`addr: register`, `bytes: uint64`|
|`READOBJ8`|`0x0020`|3|`obj: register`, `out: register`, `off: uint32`|
|`READOBJ16`|`0x0021`|3|`obj: register`, `out: register`, `off: uint32`|
|`READOBJ32`|`0x0022`|3|`obj: register`, `out: register`, `off: uint32`|
|`READOBJ64`|`0x0023`|3|`obj: register`, `out: register`, `off: uint32`|
|`WRITEOBJ8`|`0x0024`|3|`obj: register`, `val: register`, `off: uint32`|
|`WRITEOBJ16`|`0x0025`|3|`obj: register`, `val: register`, `off: uint32`|
|`WRITEOBJ32`|`0x0026`|3|`obj: register`, `val: register`, `off: uint32`|
|`WRITEOBJ64`|`0x0027`|3|`obj: register`, `val: register`, `off: uint32`|
|`READIDX8`|`0x0028`|6|`obj: register`, `out: register`, `idx: register`|
|`READIDX16`|`0x0029`|6|`obj: register`, `out: register`, `idx: register`|
|`READIDX32`|`0x002A`|6|`obj: register`, `out: register`, `idx: register`|
|`READIDX64`|`0x002B`|6|`obj: register`, `out: register`, `idx: register`|
|`WRITEIDX8`|`0x002C`|6|`obj: register`, `val: register`, `idx: register`|
|`WRITEIDX16`|`0x002D`|6|`obj: register`, `val: register`, `idx: register`|
|`WRITEIDX32`|`0x002E`|6|`obj: register`, `val: register`, `idx: register`|
|`WRITEIDX64`|`0x002F`|6|`obj: register`, `val: register`, `idx: register`|
|`PUSHARG`|`0x0030`|8|`val: register`|
|`SETRV`|`0x0031`|8|`val: register`|
|`INVOKE`|`0x0032`|7|`func: register`, `out: register`|
|`I8TU8`|`0x0033`|7|`in: register`, `out: register`|
|`I8TI16`|`0x0034`|7|`in: register`, `out: register`|
|`I8TU16`|`0x0035`|7|`in: register`, `out: register`|
|`I8TI32`|`0x0036`|7|`in: register`, `out: register`|
|`I8TU32`|`0x0037`|7|`in: register`, `out: register`|
|`I8TI64`|`0x0038`|7|`in: register`, `out: register`|
|`I8TU64`|`0x0039`|7|`in: register`, `out: register`|
|`I8TF32`|`0x003A`|7|`in: register`, `out: register`|
|`I8TF64`|`0x003B`|7|`in: register`, `out: register`|
|`U8TI8`|`0x003C`|7|`in: register`, `out: register`|
|`U8TI16`|`0x003D`|7|`in: register`, `out: register`|
|`U8TI32`|`0x003E`|7|`in: register`, `out: register`|
|`U8TI64`|`0x003F`|7|`in: register`, `out: register`|
|`U8TF32`|`0x0040`|7|`in: register`, `out: register`|
|`U8TF64`|`0x0041`|7|`in: register`, `out: register`|
|`I16TI8`|`0x0042`|7|`in: register`, `out: register`|
|`I16TU8`|`0x0043`|7|`in: register`, `out: register`|
|`I16TU16`|`0x0044`|7|`in: register`, `out: register`|
|`I16TI32`|`0x0045`|7|`in: register`, `out: register`|
|`I16TU32`|`0x0046`|7|`in: register`, `out: register`|
|`I16TI64`|`0x0047`|7|`in: register`, `out: register`|
|`I16TU64`|`0x0048`|7|`in: register`, `out: register`|
|`I16TF32`|`0x0049`|7|`in: register`, `out: register`|
|`I16TF64`|`0x004A`|7|`in: register`, `out: register`|
|`U16TI8`|`0x004B`|7|`in: register`, `out: register`|
|`U16TI16`|`0x004C`|7|`in: register`, `out: register`|
|`U16TI32`|`0x004D`|7|`in: register`, `out: register`|
|`U16TI64`|`0x004E`|7|`in: register`, `out: register`|
|`U16TF32`|`0x004F`|7|`in: register`, `out: register`|
|`U16TF64`|`0x0050`|7|`in: register`, `out: register`|
|`I32TI8`|`0x0051`|7|`in: register`, `out: register`|
|`I32TU8`|`0x0052`|7|`in: register`, `out: register`|
|`I32TI16`|`0x0053`|7|`in: register`, `out: register`|
|`I32TU16`|`0x0054`|7|`in: register`, `out: register`|
|`I32TU32`|`0x0055`|7|`in: register`, `out: register`|
|`I32TI64`|`0x0056`|7|`in: register`, `out: register`|
|`I32TU64`|`0x0057`|7|`in: register`, `out: register`|
|`I32TF32`|`0x0058`|7|`in: register`, `out: register`|
|`I32TF64`|`0x0059`|7|`in: register`, `out: register`|
|`U32TI8`|`0x005A`|7|`in: register`, `out: register`|
|`U32TI16`|`0x005B`|7|`in: register`, `out: register`|
|`U32TI32`|`0x005C`|7|`in: register`, `out: register`|
|`U32TI64`|`0x005D`|7|`in: register`, `out: register`|
|`U32TF32`|`0x005E`|7|`in: register`, `out: register`|
|`U32TF64`|`0x005F`|7|`in: register`, `out: register`|
|`I64TI8`|`0x0060`|7|`in: register`, `out: register`|
|`I64TU8`|`0x0061`|7|`in: register`, `out: register`|
|`I64TI16`|`0x0062`|7|`in: register`, `out: register`|
|`I64TU16`|`0x0063`|7|`in: register`, `out: register`|
|`I64TI32`|`0x0064`|7|`in: register`, `out: register`|
|`I64TU32`|`0x0065`|7|`in: register`, `out: register`|
|`I64TU64`|`0x0066`|7|`in: register`, `out: register`|
|`I64TF32`|`0x0067`|7|`in: register`, `out: register`|
|`I64TF64`|`0x0068`|7|`in: register`, `out: register`|
|`U64TI8`|`0x0069`|7|`in: register`, `out: register`|
|`U64TI16`|`0x006A`|7|`in: register`, `out: register`|
|`U64TI32`|`0x006B`|7|`in: register`, `out: register`|
|`U64TI64`|`0x006C`|7|`in: register`, `out: register`|
|`U64TF32`|`0x006D`|7|`in: register`, `out: register`|
|`U64TF64`|`0x006E`|7|`in: register`, `out: register`|
|`F32TI8`|`0x006F`|7|`in: register`, `out: register`|
|`F32TU8`|`0x0070`|7|`in: register`, `out: register`|
|`F32TI16`|`0x0071`|7|`in: register`, `out: register`|
|`F32TU16`|`0x0072`|7|`in: register`, `out: register`|
|`F32TI32`|`0x0073`|7|`in: register`, `out: register`|
|`F32TU32`|`0x0074`|7|`in: register`, `out: register`|
|`F32TI64`|`0x0075`|7|`in: register`, `out: register`|
|`F32TU64`|`0x0076`|7|`in: register`, `out: register`|
|`F32TF64`|`0x0077`|7|`in: register`, `out: register`|
|`F64TI8`|`0x0078`|7|`in: register`, `out: register`|
|`F64TU8`|`0x0079`|7|`in: register`, `out: register`|
|`F64TI16`|`0x007A`|7|`in: register`, `out: register`|
|`F64TU16`|`0x007B`|7|`in: register`, `out: register`|
|`F64TI32`|`0x007C`|7|`in: register`, `out: register`|
|`F64TU32`|`0x007D`|7|`in: register`, `out: register`|
|`F64TI64`|`0x007E`|7|`in: register`, `out: register`|
|`F64TU64`|`0x007F`|7|`in: register`, `out: register`|
|`F64TF32`|`0x0080`|7|`in: register`, `out: register`|
|`BNEGATE`|`0x0081`|7|`in: register`, `out: register`|
|`LNEGATE`|`0x0082`|7|`in: register`, `out: register`|
|`NEGI8`|`0x0083`|7|`in: register`, `out: register`|
|`NEGU8`|`0x0084`|7|`in: register`, `out: register`|
|`NEGI16`|`0x0085`|7|`in: register`, `out: register`|
|`NEGU16`|`0x0086`|7|`in: register`, `out: register`|
|`NEGI32`|`0x0087`|7|`in: register`, `out: register`|
|`NEGU32`|`0x0088`|7|`in: register`, `out: register`|
|`NEGI64`|`0x0089`|7|`in: register`, `out: register`|
|`NEGU64`|`0x008A`|7|`in: register`, `out: register`|
|`NEGF32`|`0x008B`|7|`in: register`, `out: register`|
|`NEGF64`|`0x008C`|7|`in: register`, `out: register`|
|`INCI8`|`0x008D`|7|`in: register`, `out: register`|
|`INCU8`|`0x008E`|7|`in: register`, `out: register`|
|`INCI16`|`0x008F`|7|`in: register`, `out: register`|
|`INCU16`|`0x0090`|7|`in: register`, `out: register`|
|`INCI32`|`0x0091`|7|`in: register`, `out: register`|
|`INCU32`|`0x0092`|7|`in: register`, `out: register`|
|`INCI64`|`0x0093`|7|`in: register`, `out: register`|
|`INCU64`|`0x0094`|7|`in: register`, `out: register`|
|`INCF32`|`0x0095`|7|`in: register`, `out: register`|
|`INCF64`|`0x0096`|7|`in: register`, `out: register`|
|`DECI8`|`0x0097`|7|`in: register`, `out: register`|
|`DECU8`|`0x0098`|7|`in: register`, `out: register`|
|`DECI16`|`0x0099`|7|`in: register`, `out: register`|
|`DECU16`|`0x009A`|7|`in: register`, `out: register`|
|`DECI32`|`0x009B`|7|`in: register`, `out: register`|
|`DECU32`|`0x009C`|7|`in: register`, `out: register`|
|`DECI64`|`0x009D`|7|`in: register`, `out: register`|
|`DECU64`|`0x009E`|7|`in: register`, `out: register`|
|`DECF32`|`0x009F`|7|`in: register`, `out: register`|
|`DECF64`|`0x00A0`|7|`in: register`, `out: register`|
|`LSHIFT8`|`0x00A1`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LSHIFT16`|`0x00A2`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LSHIFT32`|`0x00A3`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LSHIFT64`|`0x00A4`|6|`lhs: register`, `rhs: register`, `out: register`|
|`RSHIFT8`|`0x00A5`|6|`lhs: register`, `rhs: register`, `out: register`|
|`RSHIFT16`|`0x00A6`|6|`lhs: register`, `rhs: register`, `out: register`|
|`RSHIFT32`|`0x00A7`|6|`lhs: register`, `rhs: register`, `out: register`|
|`RSHIFT64`|`0x00A8`|6|`lhs: register`, `rhs: register`, `out: register`|
|`URSHIFT8`|`0x00A9`|6|`lhs: register`, `rhs: register`, `out: register`|
|`URSHIFT16`|`0x00AA`|6|`lhs: register`, `rhs: register`, `out: register`|
|`URSHIFT32`|`0x00AB`|6|`lhs: register`, `rhs: register`, `out: register`|
|`URSHIFT64`|`0x00AC`|6|`lhs: register`, `rhs: register`, `out: register`|
|`BAND`|`0x00AD`|6|`lhs: register`, `rhs: register`, `out: register`|
|`BOR`|`0x00AE`|6|`lhs: register`, `rhs: register`, `out: register`|
|`BXOR`|`0x00AF`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LAND`|`0x00B0`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LOR`|`0x00B1`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LXOR`|`0x00B2`|6|`lhs: register`, `rhs: register`, `out: register`|
|`ADDI8`|`0x00B3`|6|`lhs: register`, `rhs: register`, `out: register`|
|`ADDU8`|`0x00B4`|6|`lhs: register`, `rhs: register`, `out: register`|
|`ADDI16`|`0x00B5`|6|`lhs: register`, `rhs: register`, `out: register`|
|`ADDU16`|`0x00B6`|6|`lhs: register`, `rhs: register`, `out: register`|
|`ADDI32`|`0x00B7`|6|`lhs: register`, `rhs: register`, `out: register`|
|`ADDU32`|`0x00B8`|6|`lhs: register`, `rhs: register`, `out: register`|
|`ADDI64`|`0x00B9`|6|`lhs: register`, `rhs: register`, `out: register`|
|`ADDU64`|`0x00BA`|6|`lhs: register`, `rhs: register`, `out: register`|
|`ADDF32`|`0x00BB`|6|`lhs: register`, `rhs: register`, `out: register`|
|`ADDF64`|`0x00BC`|6|`lhs: register`, `rhs: register`, `out: register`|
|`SUBI8`|`0x00BD`|6|`lhs: register`, `rhs: register`, `out: register`|
|`SUBU8`|`0x00BE`|6|`lhs: register`, `rhs: register`, `out: register`|
|`SUBI16`|`0x00BF`|6|`lhs: register`, `rhs: register`, `out: register`|
|`SUBU16`|`0x00C0`|6|`lhs: register`, `rhs: register`, `out: register`|
|`SUBI32`|`0x00C1`|6|`lhs: register`, `rhs: register`, `out: register`|
|`SUBU32`|`0x00C2`|6|`lhs: register`, `rhs: register`, `out: register`|
|`SUBI64`|`0x00C3`|6|`lhs: register`, `rhs: register`, `out: register`|
|`SUBU64`|`0x00C4`|6|`lhs: register`, `rhs: register`, `out: register`|
|`SUBF32`|`0x00C5`|6|`lhs: register`, `rhs: register`, `out: register`|
|`SUBF64`|`0x00C6`|6|`lhs: register`, `rhs: register`, `out: register`|
|`DIVI8`|`0x00C7`|6|`lhs: register`, `rhs: register`, `out: register`|
|`DIVU8`|`0x00C8`|6|`lhs: register`, `rhs: register`, `out: register`|
|`DIVI16`|`0x00C9`|6|`lhs: register`, `rhs: register`, `out: register`|
|`DIVU16`|`0x00CA`|6|`lhs: register`, `rhs: register`, `out: register`|
|`DIVI32`|`0x00CB`|6|`lhs: register`, `rhs: register`, `out: register`|
|`DIVU32`|`0x00CC`|6|`lhs: register`, `rhs: register`, `out: register`|
|`DIVI64`|`0x00CD`|6|`lhs: register`, `rhs: register`, `out: register`|
|`DIVU64`|`0x00CE`|6|`lhs: register`, `rhs: register`, `out: register`|
|`DIVF32`|`0x00CF`|6|`lhs: register`, `rhs: register`, `out: register`|
|`DIVF64`|`0x00D0`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MULI8`|`0x00D1`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MULU8`|`0x00D2`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MULI16`|`0x00D3`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MULU16`|`0x00D4`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MULI32`|`0x00D5`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MULU32`|`0x00D6`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MULI64`|`0x00D7`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MULU64`|`0x00D8`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MULF32`|`0x00D9`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MULF64`|`0x00DA`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MODI8`|`0x00DB`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MODU8`|`0x00DC`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MODI16`|`0x00DD`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MODU16`|`0x00DE`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MODI32`|`0x00DF`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MODU32`|`0x00E0`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MODI64`|`0x00E1`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MODU64`|`0x00E2`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MODF32`|`0x00E3`|6|`lhs: register`, `rhs: register`, `out: register`|
|`MODF64`|`0x00E4`|6|`lhs: register`, `rhs: register`, `out: register`|
|`POWI8`|`0x00E5`|6|`lhs: register`, `rhs: register`, `out: register`|
|`POWU8`|`0x00E6`|6|`lhs: register`, `rhs: register`, `out: register`|
|`POWI16`|`0x00E7`|6|`lhs: register`, `rhs: register`, `out: register`|
|`POWU16`|`0x00E8`|6|`lhs: register`, `rhs: register`, `out: register`|
|`POWI32`|`0x00E9`|6|`lhs: register`, `rhs: register`, `out: register`|
|`POWU32`|`0x00EA`|6|`lhs: register`, `rhs: register`, `out: register`|
|`POWI64`|`0x00EB`|6|`lhs: register`, `rhs: register`, `out: register`|
|`POWU64`|`0x00EC`|6|`lhs: register`, `rhs: register`, `out: register`|
|`POWF32`|`0x00ED`|6|`lhs: register`, `rhs: register`, `out: register`|
|`POWF64`|`0x00EE`|6|`lhs: register`, `rhs: register`, `out: register`|
|`EQ8`|`0x00EF`|6|`lhs: register`, `rhs: register`, `out: register`|
|`EQ16`|`0x00F0`|6|`lhs: register`, `rhs: register`, `out: register`|
|`EQ32`|`0x00F1`|6|`lhs: register`, `rhs: register`, `out: register`|
|`EQ64`|`0x00F2`|6|`lhs: register`, `rhs: register`, `out: register`|
|`EQARR`|`0x00F3`|6|`lhs: register`, `rhs: register`, `out: register`|
|`EQSTRUCT`|`0x00F4`|6|`lhs: register`, `rhs: register`, `out: register`|
|`NEQ8`|`0x00F5`|6|`lhs: register`, `rhs: register`, `out: register`|
|`NEQ16`|`0x00F6`|6|`lhs: register`, `rhs: register`, `out: register`|
|`NEQ32`|`0x00F7`|6|`lhs: register`, `rhs: register`, `out: register`|
|`NEQ64`|`0x00F8`|6|`lhs: register`, `rhs: register`, `out: register`|
|`NEQARR`|`0x00F9`|6|`lhs: register`, `rhs: register`, `out: register`|
|`NEQSTRUCT`|`0x00FA`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTI8`|`0x00FB`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTU8`|`0x00FC`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTI16`|`0x00FD`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTU16`|`0x00FE`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTI32`|`0x00FF`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTU32`|`0x0100`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTI64`|`0x0101`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTU64`|`0x0102`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTF32`|`0x0103`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTF64`|`0x0104`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTARR`|`0x0105`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTEI8`|`0x0106`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTEU8`|`0x0107`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTEI16`|`0x0108`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTEU16`|`0x0109`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTEI32`|`0x010A`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTEU32`|`0x010B`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTEI64`|`0x010C`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTEU64`|`0x010D`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTEF32`|`0x010E`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTEF64`|`0x010F`|6|`lhs: register`, `rhs: register`, `out: register`|
|`GTEARR`|`0x0110`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTI8`|`0x0111`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTU8`|`0x0112`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTI16`|`0x0113`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTU16`|`0x0114`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTI32`|`0x0115`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTU32`|`0x0116`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTI64`|`0x0117`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTU64`|`0x0118`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTF32`|`0x0119`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTF64`|`0x011A`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTARR`|`0x011B`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTEI8`|`0x011C`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTEU8`|`0x011D`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTEI16`|`0x011E`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTEU16`|`0x011F`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTEI32`|`0x0120`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTEU32`|`0x0121`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTEI64`|`0x0122`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTEU64`|`0x0123`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTEF32`|`0x0124`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTEF64`|`0x0125`|6|`lhs: register`, `rhs: register`, `out: register`|
|`LTEARR`|`0x0126`|6|`lhs: register`, `rhs: register`, `out: register`|
|`STRCONCAT`|`0x0127`|6|`lhs: register`, `rhs: register`, `out: register`|
|`STRREP8`|`0x0128`|6|`lhs: register`, `rhs: register`, `out: register`|
|`STRREP16`|`0x0129`|6|`lhs: register`, `rhs: register`, `out: register`|
|`STRREP32`|`0x012A`|6|`lhs: register`, `rhs: register`, `out: register`|
|`STRREP64`|`0x012B`|6|`lhs: register`, `rhs: register`, `out: register`|