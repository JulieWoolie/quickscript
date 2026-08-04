# OP Code specification Table
| Metadata Property | Value |
|--|--|
|Size of an instruction (in bytes)|16|
|Bytes used for an opcode|2|
|Arguments length (bytes)|14|

| OP Code | Value | Padding | Arguments |
|--|--|--|--|
|`NOP`|`0x0000`|14||
|`PUSHLINE`|`0x0001`|10|`lineno: uint32`|
|`RET`|`0x0002`|14||
|`JMP`|`0x0003`|10|`to: uint32`|
|`JMPI0`|`0x0004`|9|`to: uint32`, `condition: register`|
|`JMPN0`|`0x0005`|9|`to: uint32`, `condition: register`|
|`MOV`|`0x0006`|12|`from: register`, `to: register`|
|`LOADCONST8`|`0x0007`|12|`out: register`, `val: uint8`|
|`LOADCONST16`|`0x0008`|11|`out: register`, `val: uint16`|
|`LOADCONST32`|`0x0009`|9|`out: register`, `val: uint32`|
|`LOADCONST64`|`0x000A`|7|`out: register`, `val: uint64`|
|`LOADCONSTSTR`|`0x000B`|5|`out: register`, `straddr: uint64`|
|`STACKALLOC`|`0x000C`|6|`bytes: uint64`|
|`STACKFREE`|`0x000D`|6|`bytes: uint64`|
|`RSREAD8`|`0x000E`|5|`out: register`, `offset: uint64`|
|`RSREAD16`|`0x000F`|5|`out: register`, `offset: uint64`|
|`RSREAD32`|`0x0010`|5|`out: register`, `offset: uint64`|
|`RSREAD64`|`0x0011`|5|`out: register`, `offset: uint64`|
|`RSWRITE8`|`0x0012`|5|`val: register`, `offset: uint64`|
|`RSWRITE16`|`0x0013`|5|`val: register`, `offset: uint64`|
|`RSWRITE32`|`0x0014`|5|`val: register`, `offset: uint64`|
|`RSWRITE64`|`0x0015`|5|`val: register`, `offset: uint64`|
|`ASREAD8`|`0x0016`|5|`out: register`, `offset: uint64`|
|`ASREAD16`|`0x0017`|5|`out: register`, `offset: uint64`|
|`ASREAD32`|`0x0018`|5|`out: register`, `offset: uint64`|
|`ASREAD64`|`0x0019`|5|`out: register`, `offset: uint64`|
|`ASWRITE8`|`0x001A`|5|`val: register`, `offset: uint64`|
|`ASWRITE16`|`0x001B`|5|`val: register`, `offset: uint64`|
|`ASWRITE32`|`0x001C`|5|`val: register`, `offset: uint64`|
|`ASWRITE64`|`0x001D`|5|`val: register`, `offset: uint64`|
|`HEAPALLOC`|`0x001E`|5|`out: register`, `bytes: uint64`|
|`HEAPFREE`|`0x001F`|5|`addr: register`, `bytes: uint64`|
|`READOBJ8`|`0x0020`|8|`obj: register`, `out: register`, `off: uint32`|
|`READOBJ16`|`0x0021`|8|`obj: register`, `out: register`, `off: uint32`|
|`READOBJ32`|`0x0022`|8|`obj: register`, `out: register`, `off: uint32`|
|`READOBJ64`|`0x0023`|8|`obj: register`, `out: register`, `off: uint32`|
|`WRITEOBJ8`|`0x0024`|8|`obj: register`, `val: register`, `off: uint32`|
|`WRITEOBJ16`|`0x0025`|8|`obj: register`, `val: register`, `off: uint32`|
|`WRITEOBJ32`|`0x0026`|8|`obj: register`, `val: register`, `off: uint32`|
|`WRITEOBJ64`|`0x0027`|8|`obj: register`, `val: register`, `off: uint32`|
|`READIDX8`|`0x0028`|11|`obj: register`, `out: register`, `idx: register`|
|`READIDX16`|`0x0029`|11|`obj: register`, `out: register`, `idx: register`|
|`READIDX32`|`0x002A`|11|`obj: register`, `out: register`, `idx: register`|
|`READIDX64`|`0x002B`|11|`obj: register`, `out: register`, `idx: register`|
|`WRITEIDX8`|`0x002C`|11|`obj: register`, `val: register`, `idx: register`|
|`WRITEIDX16`|`0x002D`|11|`obj: register`, `val: register`, `idx: register`|
|`WRITEIDX32`|`0x002E`|11|`obj: register`, `val: register`, `idx: register`|
|`WRITEIDX64`|`0x002F`|11|`obj: register`, `val: register`, `idx: register`|
|`PUSHARG`|`0x0030`|13|`val: register`|
|`SETRV`|`0x0031`|13|`val: register`|
|`INVOKE`|`0x0032`|12|`func: register`, `out: register`|
|`FUNCLOOKUP`|`0x0033`|9|`out: register`, `index: uint32`|
|`I8TU8`|`0x0034`|12|`in: register`, `out: register`|
|`I8TI16`|`0x0035`|12|`in: register`, `out: register`|
|`I8TU16`|`0x0036`|12|`in: register`, `out: register`|
|`I8TI32`|`0x0037`|12|`in: register`, `out: register`|
|`I8TU32`|`0x0038`|12|`in: register`, `out: register`|
|`I8TI64`|`0x0039`|12|`in: register`, `out: register`|
|`I8TU64`|`0x003A`|12|`in: register`, `out: register`|
|`I8TF32`|`0x003B`|12|`in: register`, `out: register`|
|`I8TF64`|`0x003C`|12|`in: register`, `out: register`|
|`U8TI8`|`0x003D`|12|`in: register`, `out: register`|
|`U8TI16`|`0x003E`|12|`in: register`, `out: register`|
|`U8TI32`|`0x003F`|12|`in: register`, `out: register`|
|`U8TI64`|`0x0040`|12|`in: register`, `out: register`|
|`U8TF32`|`0x0041`|12|`in: register`, `out: register`|
|`U8TF64`|`0x0042`|12|`in: register`, `out: register`|
|`I16TI8`|`0x0043`|12|`in: register`, `out: register`|
|`I16TU8`|`0x0044`|12|`in: register`, `out: register`|
|`I16TU16`|`0x0045`|12|`in: register`, `out: register`|
|`I16TI32`|`0x0046`|12|`in: register`, `out: register`|
|`I16TU32`|`0x0047`|12|`in: register`, `out: register`|
|`I16TI64`|`0x0048`|12|`in: register`, `out: register`|
|`I16TU64`|`0x0049`|12|`in: register`, `out: register`|
|`I16TF32`|`0x004A`|12|`in: register`, `out: register`|
|`I16TF64`|`0x004B`|12|`in: register`, `out: register`|
|`U16TI8`|`0x004C`|12|`in: register`, `out: register`|
|`U16TI16`|`0x004D`|12|`in: register`, `out: register`|
|`U16TI32`|`0x004E`|12|`in: register`, `out: register`|
|`U16TI64`|`0x004F`|12|`in: register`, `out: register`|
|`U16TF32`|`0x0050`|12|`in: register`, `out: register`|
|`U16TF64`|`0x0051`|12|`in: register`, `out: register`|
|`I32TI8`|`0x0052`|12|`in: register`, `out: register`|
|`I32TU8`|`0x0053`|12|`in: register`, `out: register`|
|`I32TI16`|`0x0054`|12|`in: register`, `out: register`|
|`I32TU16`|`0x0055`|12|`in: register`, `out: register`|
|`I32TU32`|`0x0056`|12|`in: register`, `out: register`|
|`I32TI64`|`0x0057`|12|`in: register`, `out: register`|
|`I32TU64`|`0x0058`|12|`in: register`, `out: register`|
|`I32TF32`|`0x0059`|12|`in: register`, `out: register`|
|`I32TF64`|`0x005A`|12|`in: register`, `out: register`|
|`U32TI8`|`0x005B`|12|`in: register`, `out: register`|
|`U32TI16`|`0x005C`|12|`in: register`, `out: register`|
|`U32TI32`|`0x005D`|12|`in: register`, `out: register`|
|`U32TI64`|`0x005E`|12|`in: register`, `out: register`|
|`U32TF32`|`0x005F`|12|`in: register`, `out: register`|
|`U32TF64`|`0x0060`|12|`in: register`, `out: register`|
|`I64TI8`|`0x0061`|12|`in: register`, `out: register`|
|`I64TU8`|`0x0062`|12|`in: register`, `out: register`|
|`I64TI16`|`0x0063`|12|`in: register`, `out: register`|
|`I64TU16`|`0x0064`|12|`in: register`, `out: register`|
|`I64TI32`|`0x0065`|12|`in: register`, `out: register`|
|`I64TU32`|`0x0066`|12|`in: register`, `out: register`|
|`I64TU64`|`0x0067`|12|`in: register`, `out: register`|
|`I64TF32`|`0x0068`|12|`in: register`, `out: register`|
|`I64TF64`|`0x0069`|12|`in: register`, `out: register`|
|`U64TI8`|`0x006A`|12|`in: register`, `out: register`|
|`U64TI16`|`0x006B`|12|`in: register`, `out: register`|
|`U64TI32`|`0x006C`|12|`in: register`, `out: register`|
|`U64TI64`|`0x006D`|12|`in: register`, `out: register`|
|`U64TF32`|`0x006E`|12|`in: register`, `out: register`|
|`U64TF64`|`0x006F`|12|`in: register`, `out: register`|
|`F32TI8`|`0x0070`|12|`in: register`, `out: register`|
|`F32TU8`|`0x0071`|12|`in: register`, `out: register`|
|`F32TI16`|`0x0072`|12|`in: register`, `out: register`|
|`F32TU16`|`0x0073`|12|`in: register`, `out: register`|
|`F32TI32`|`0x0074`|12|`in: register`, `out: register`|
|`F32TU32`|`0x0075`|12|`in: register`, `out: register`|
|`F32TI64`|`0x0076`|12|`in: register`, `out: register`|
|`F32TU64`|`0x0077`|12|`in: register`, `out: register`|
|`F32TF64`|`0x0078`|12|`in: register`, `out: register`|
|`F64TI8`|`0x0079`|12|`in: register`, `out: register`|
|`F64TU8`|`0x007A`|12|`in: register`, `out: register`|
|`F64TI16`|`0x007B`|12|`in: register`, `out: register`|
|`F64TU16`|`0x007C`|12|`in: register`, `out: register`|
|`F64TI32`|`0x007D`|12|`in: register`, `out: register`|
|`F64TU32`|`0x007E`|12|`in: register`, `out: register`|
|`F64TI64`|`0x007F`|12|`in: register`, `out: register`|
|`F64TU64`|`0x0080`|12|`in: register`, `out: register`|
|`F64TF32`|`0x0081`|12|`in: register`, `out: register`|
|`BNEGATE`|`0x0082`|12|`in: register`, `out: register`|
|`LNEGATE`|`0x0083`|12|`in: register`, `out: register`|
|`NEGI8`|`0x0084`|12|`in: register`, `out: register`|
|`NEGU8`|`0x0085`|12|`in: register`, `out: register`|
|`NEGI16`|`0x0086`|12|`in: register`, `out: register`|
|`NEGU16`|`0x0087`|12|`in: register`, `out: register`|
|`NEGI32`|`0x0088`|12|`in: register`, `out: register`|
|`NEGU32`|`0x0089`|12|`in: register`, `out: register`|
|`NEGI64`|`0x008A`|12|`in: register`, `out: register`|
|`NEGU64`|`0x008B`|12|`in: register`, `out: register`|
|`NEGF32`|`0x008C`|12|`in: register`, `out: register`|
|`NEGF64`|`0x008D`|12|`in: register`, `out: register`|
|`INCI8`|`0x008E`|12|`in: register`, `out: register`|
|`INCU8`|`0x008F`|12|`in: register`, `out: register`|
|`INCI16`|`0x0090`|12|`in: register`, `out: register`|
|`INCU16`|`0x0091`|12|`in: register`, `out: register`|
|`INCI32`|`0x0092`|12|`in: register`, `out: register`|
|`INCU32`|`0x0093`|12|`in: register`, `out: register`|
|`INCI64`|`0x0094`|12|`in: register`, `out: register`|
|`INCU64`|`0x0095`|12|`in: register`, `out: register`|
|`INCF32`|`0x0096`|12|`in: register`, `out: register`|
|`INCF64`|`0x0097`|12|`in: register`, `out: register`|
|`DECI8`|`0x0098`|12|`in: register`, `out: register`|
|`DECU8`|`0x0099`|12|`in: register`, `out: register`|
|`DECI16`|`0x009A`|12|`in: register`, `out: register`|
|`DECU16`|`0x009B`|12|`in: register`, `out: register`|
|`DECI32`|`0x009C`|12|`in: register`, `out: register`|
|`DECU32`|`0x009D`|12|`in: register`, `out: register`|
|`DECI64`|`0x009E`|12|`in: register`, `out: register`|
|`DECU64`|`0x009F`|12|`in: register`, `out: register`|
|`DECF32`|`0x00A0`|12|`in: register`, `out: register`|
|`DECF64`|`0x00A1`|12|`in: register`, `out: register`|
|`LSHIFT8`|`0x00A2`|11|`lhs: register`, `rhs: register`, `out: register`|
|`LSHIFT16`|`0x00A3`|11|`lhs: register`, `rhs: register`, `out: register`|
|`LSHIFT32`|`0x00A4`|11|`lhs: register`, `rhs: register`, `out: register`|
|`LSHIFT64`|`0x00A5`|11|`lhs: register`, `rhs: register`, `out: register`|
|`RSHIFT8`|`0x00A6`|11|`lhs: register`, `rhs: register`, `out: register`|
|`RSHIFT16`|`0x00A7`|11|`lhs: register`, `rhs: register`, `out: register`|
|`RSHIFT32`|`0x00A8`|11|`lhs: register`, `rhs: register`, `out: register`|
|`RSHIFT64`|`0x00A9`|11|`lhs: register`, `rhs: register`, `out: register`|
|`URSHIFT8`|`0x00AA`|11|`lhs: register`, `rhs: register`, `out: register`|
|`URSHIFT16`|`0x00AB`|11|`lhs: register`, `rhs: register`, `out: register`|
|`URSHIFT32`|`0x00AC`|11|`lhs: register`, `rhs: register`, `out: register`|
|`URSHIFT64`|`0x00AD`|11|`lhs: register`, `rhs: register`, `out: register`|
|`BAND`|`0x00AE`|11|`lhs: register`, `rhs: register`, `out: register`|
|`BOR`|`0x00AF`|11|`lhs: register`, `rhs: register`, `out: register`|
|`BXOR`|`0x00B0`|11|`lhs: register`, `rhs: register`, `out: register`|
|`LAND`|`0x00B1`|11|`lhs: register`, `rhs: register`, `out: register`|
|`LOR`|`0x00B2`|11|`lhs: register`, `rhs: register`, `out: register`|
|`LXOR`|`0x00B3`|11|`lhs: register`, `rhs: register`, `out: register`|
|`ADDI8`|`0x00B4`|11|`lhs: register`, `rhs: register`, `out: register`|
|`ADDU8`|`0x00B5`|11|`lhs: register`, `rhs: register`, `out: register`|
|`ADDI16`|`0x00B6`|11|`lhs: register`, `rhs: register`, `out: register`|
|`ADDU16`|`0x00B7`|11|`lhs: register`, `rhs: register`, `out: register`|
|`ADDI32`|`0x00B8`|11|`lhs: register`, `rhs: register`, `out: register`|
|`ADDU32`|`0x00B9`|11|`lhs: register`, `rhs: register`, `out: register`|
|`ADDI64`|`0x00BA`|11|`lhs: register`, `rhs: register`, `out: register`|
|`ADDU64`|`0x00BB`|11|`lhs: register`, `rhs: register`, `out: register`|
|`ADDF32`|`0x00BC`|11|`lhs: register`, `rhs: register`, `out: register`|
|`ADDF64`|`0x00BD`|11|`lhs: register`, `rhs: register`, `out: register`|
|`SUBI8`|`0x00BE`|11|`lhs: register`, `rhs: register`, `out: register`|
|`SUBU8`|`0x00BF`|11|`lhs: register`, `rhs: register`, `out: register`|
|`SUBI16`|`0x00C0`|11|`lhs: register`, `rhs: register`, `out: register`|
|`SUBU16`|`0x00C1`|11|`lhs: register`, `rhs: register`, `out: register`|
|`SUBI32`|`0x00C2`|11|`lhs: register`, `rhs: register`, `out: register`|
|`SUBU32`|`0x00C3`|11|`lhs: register`, `rhs: register`, `out: register`|
|`SUBI64`|`0x00C4`|11|`lhs: register`, `rhs: register`, `out: register`|
|`SUBU64`|`0x00C5`|11|`lhs: register`, `rhs: register`, `out: register`|
|`SUBF32`|`0x00C6`|11|`lhs: register`, `rhs: register`, `out: register`|
|`SUBF64`|`0x00C7`|11|`lhs: register`, `rhs: register`, `out: register`|
|`DIVI8`|`0x00C8`|11|`lhs: register`, `rhs: register`, `out: register`|
|`DIVU8`|`0x00C9`|11|`lhs: register`, `rhs: register`, `out: register`|
|`DIVI16`|`0x00CA`|11|`lhs: register`, `rhs: register`, `out: register`|
|`DIVU16`|`0x00CB`|11|`lhs: register`, `rhs: register`, `out: register`|
|`DIVI32`|`0x00CC`|11|`lhs: register`, `rhs: register`, `out: register`|
|`DIVU32`|`0x00CD`|11|`lhs: register`, `rhs: register`, `out: register`|
|`DIVI64`|`0x00CE`|11|`lhs: register`, `rhs: register`, `out: register`|
|`DIVU64`|`0x00CF`|11|`lhs: register`, `rhs: register`, `out: register`|
|`DIVF32`|`0x00D0`|11|`lhs: register`, `rhs: register`, `out: register`|
|`DIVF64`|`0x00D1`|11|`lhs: register`, `rhs: register`, `out: register`|
|`MULI8`|`0x00D2`|11|`lhs: register`, `rhs: register`, `out: register`|
|`MULU8`|`0x00D3`|11|`lhs: register`, `rhs: register`, `out: register`|
|`MULI16`|`0x00D4`|11|`lhs: register`, `rhs: register`, `out: register`|
|`MULU16`|`0x00D5`|11|`lhs: register`, `rhs: register`, `out: register`|
|`MULI32`|`0x00D6`|11|`lhs: register`, `rhs: register`, `out: register`|
|`MULU32`|`0x00D7`|11|`lhs: register`, `rhs: register`, `out: register`|
|`MULI64`|`0x00D8`|11|`lhs: register`, `rhs: register`, `out: register`|
|`MULU64`|`0x00D9`|11|`lhs: register`, `rhs: register`, `out: register`|
|`MULF32`|`0x00DA`|11|`lhs: register`, `rhs: register`, `out: register`|
|`MULF64`|`0x00DB`|11|`lhs: register`, `rhs: register`, `out: register`|
|`MODI8`|`0x00DC`|11|`lhs: register`, `rhs: register`, `out: register`|
|`MODU8`|`0x00DD`|11|`lhs: register`, `rhs: register`, `out: register`|
|`MODI16`|`0x00DE`|11|`lhs: register`, `rhs: register`, `out: register`|
|`MODU16`|`0x00DF`|11|`lhs: register`, `rhs: register`, `out: register`|
|`MODI32`|`0x00E0`|11|`lhs: register`, `rhs: register`, `out: register`|
|`MODU32`|`0x00E1`|11|`lhs: register`, `rhs: register`, `out: register`|
|`MODI64`|`0x00E2`|11|`lhs: register`, `rhs: register`, `out: register`|
|`MODU64`|`0x00E3`|11|`lhs: register`, `rhs: register`, `out: register`|
|`MODF32`|`0x00E4`|11|`lhs: register`, `rhs: register`, `out: register`|
|`MODF64`|`0x00E5`|11|`lhs: register`, `rhs: register`, `out: register`|
|`POWI8`|`0x00E6`|11|`lhs: register`, `rhs: register`, `out: register`|
|`POWU8`|`0x00E7`|11|`lhs: register`, `rhs: register`, `out: register`|
|`POWI16`|`0x00E8`|11|`lhs: register`, `rhs: register`, `out: register`|
|`POWU16`|`0x00E9`|11|`lhs: register`, `rhs: register`, `out: register`|
|`POWI32`|`0x00EA`|11|`lhs: register`, `rhs: register`, `out: register`|
|`POWU32`|`0x00EB`|11|`lhs: register`, `rhs: register`, `out: register`|
|`POWI64`|`0x00EC`|11|`lhs: register`, `rhs: register`, `out: register`|
|`POWU64`|`0x00ED`|11|`lhs: register`, `rhs: register`, `out: register`|
|`POWF32`|`0x00EE`|11|`lhs: register`, `rhs: register`, `out: register`|
|`POWF64`|`0x00EF`|11|`lhs: register`, `rhs: register`, `out: register`|
|`EQ8`|`0x00F0`|11|`lhs: register`, `rhs: register`, `out: register`|
|`EQ16`|`0x00F1`|11|`lhs: register`, `rhs: register`, `out: register`|
|`EQ32`|`0x00F2`|11|`lhs: register`, `rhs: register`, `out: register`|
|`EQ64`|`0x00F3`|11|`lhs: register`, `rhs: register`, `out: register`|
|`EQARR`|`0x00F4`|11|`lhs: register`, `rhs: register`, `out: register`|
|`EQSTRUCT`|`0x00F5`|11|`lhs: register`, `rhs: register`, `out: register`|
|`NEQ8`|`0x00F6`|11|`lhs: register`, `rhs: register`, `out: register`|
|`NEQ16`|`0x00F7`|11|`lhs: register`, `rhs: register`, `out: register`|
|`NEQ32`|`0x00F8`|11|`lhs: register`, `rhs: register`, `out: register`|
|`NEQ64`|`0x00F9`|11|`lhs: register`, `rhs: register`, `out: register`|
|`NEQARR`|`0x00FA`|11|`lhs: register`, `rhs: register`, `out: register`|
|`NEQSTRUCT`|`0x00FB`|11|`lhs: register`, `rhs: register`, `out: register`|
|`GTI8`|`0x00FC`|11|`lhs: register`, `rhs: register`, `out: register`|
|`GTU8`|`0x00FD`|11|`lhs: register`, `rhs: register`, `out: register`|
|`GTI16`|`0x00FE`|11|`lhs: register`, `rhs: register`, `out: register`|
|`GTU16`|`0x00FF`|11|`lhs: register`, `rhs: register`, `out: register`|
|`GTI32`|`0x0100`|11|`lhs: register`, `rhs: register`, `out: register`|
|`GTU32`|`0x0101`|11|`lhs: register`, `rhs: register`, `out: register`|
|`GTI64`|`0x0102`|11|`lhs: register`, `rhs: register`, `out: register`|
|`GTU64`|`0x0103`|11|`lhs: register`, `rhs: register`, `out: register`|
|`GTF32`|`0x0104`|11|`lhs: register`, `rhs: register`, `out: register`|
|`GTF64`|`0x0105`|11|`lhs: register`, `rhs: register`, `out: register`|
|`GTARR`|`0x0106`|11|`lhs: register`, `rhs: register`, `out: register`|
|`GTEI8`|`0x0107`|11|`lhs: register`, `rhs: register`, `out: register`|
|`GTEU8`|`0x0108`|11|`lhs: register`, `rhs: register`, `out: register`|
|`GTEI16`|`0x0109`|11|`lhs: register`, `rhs: register`, `out: register`|
|`GTEU16`|`0x010A`|11|`lhs: register`, `rhs: register`, `out: register`|
|`GTEI32`|`0x010B`|11|`lhs: register`, `rhs: register`, `out: register`|
|`GTEU32`|`0x010C`|11|`lhs: register`, `rhs: register`, `out: register`|
|`GTEI64`|`0x010D`|11|`lhs: register`, `rhs: register`, `out: register`|
|`GTEU64`|`0x010E`|11|`lhs: register`, `rhs: register`, `out: register`|
|`GTEF32`|`0x010F`|11|`lhs: register`, `rhs: register`, `out: register`|
|`GTEF64`|`0x0110`|11|`lhs: register`, `rhs: register`, `out: register`|
|`GTEARR`|`0x0111`|11|`lhs: register`, `rhs: register`, `out: register`|
|`LTI8`|`0x0112`|11|`lhs: register`, `rhs: register`, `out: register`|
|`LTU8`|`0x0113`|11|`lhs: register`, `rhs: register`, `out: register`|
|`LTI16`|`0x0114`|11|`lhs: register`, `rhs: register`, `out: register`|
|`LTU16`|`0x0115`|11|`lhs: register`, `rhs: register`, `out: register`|
|`LTI32`|`0x0116`|11|`lhs: register`, `rhs: register`, `out: register`|
|`LTU32`|`0x0117`|11|`lhs: register`, `rhs: register`, `out: register`|
|`LTI64`|`0x0118`|11|`lhs: register`, `rhs: register`, `out: register`|
|`LTU64`|`0x0119`|11|`lhs: register`, `rhs: register`, `out: register`|
|`LTF32`|`0x011A`|11|`lhs: register`, `rhs: register`, `out: register`|
|`LTF64`|`0x011B`|11|`lhs: register`, `rhs: register`, `out: register`|
|`LTARR`|`0x011C`|11|`lhs: register`, `rhs: register`, `out: register`|
|`LTEI8`|`0x011D`|11|`lhs: register`, `rhs: register`, `out: register`|
|`LTEU8`|`0x011E`|11|`lhs: register`, `rhs: register`, `out: register`|
|`LTEI16`|`0x011F`|11|`lhs: register`, `rhs: register`, `out: register`|
|`LTEU16`|`0x0120`|11|`lhs: register`, `rhs: register`, `out: register`|
|`LTEI32`|`0x0121`|11|`lhs: register`, `rhs: register`, `out: register`|
|`LTEU32`|`0x0122`|11|`lhs: register`, `rhs: register`, `out: register`|
|`LTEI64`|`0x0123`|11|`lhs: register`, `rhs: register`, `out: register`|
|`LTEU64`|`0x0124`|11|`lhs: register`, `rhs: register`, `out: register`|
|`LTEF32`|`0x0125`|11|`lhs: register`, `rhs: register`, `out: register`|
|`LTEF64`|`0x0126`|11|`lhs: register`, `rhs: register`, `out: register`|
|`LTEARR`|`0x0127`|11|`lhs: register`, `rhs: register`, `out: register`|
|`STRCONCAT`|`0x0128`|11|`lhs: register`, `rhs: register`, `out: register`|
|`STRREP8`|`0x0129`|11|`lhs: register`, `rhs: register`, `out: register`|
|`STRREP16`|`0x012A`|11|`lhs: register`, `rhs: register`, `out: register`|
|`STRREP32`|`0x012B`|11|`lhs: register`, `rhs: register`, `out: register`|
|`STRREP64`|`0x012C`|11|`lhs: register`, `rhs: register`, `out: register`|