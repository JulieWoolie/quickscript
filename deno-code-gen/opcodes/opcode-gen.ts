import {
  BINARY_ARGS, getFittingType,
  Instruction,
  INSTRUCTION_LENGTH,
  InstructionParam,
  NUMBER_TYPES,
  OpCodeGenResult, UNARY_ARGS
} from "../common";

interface BaseMathOp {
  name: string
}
interface NativeMathOp extends BaseMathOp {
  type: "native",
  operator: string
}
interface FuncMathOp extends BaseMathOp {
  type: "func",
  functionName: string
}

type MathOperation = | FuncMathOp | NativeMathOp

const MATH_OPERATIONS: MathOperation[] = [
  {name: "add", type: "native", operator: "+"},
  {name: "sub", type: "native", operator: "-"},
  {name: "div", type: "native", operator: "/"},
  {name: "mul", type: "native", operator: "*"},
  {name: "mod", type: "native", operator: "%"},
  {name: "pow", type: "func", functionName: "std::pow"},
]

let currentCategory: string = ""
const opcodes: Instruction[] = []

export function generateOpCodes(): OpCodeGenResult {
  generalOperations()
  stackOperations()
  globalStackOperations()
  closureOperations()
  heapOperations()
  functionCallOperations()
  conversionOperations()
  unaryOperations()
  integerBinaryOperations()
  booleanBinaryOperations()
  mathOperations()
  comparisonOperators()
  stringArrayOperations()

  const fittingType = getFittingType(opcodes.length)
  if (fittingType == null) {
    throw "Too many OP Codes, no valid number type can represent all op codes"
  }

  let paddingError = false
  for (const code of opcodes) {
    let totalMem = fittingType.bytes
    for (const arg of code.params) {
      totalMem += arg.size
    }
    code.padding = INSTRUCTION_LENGTH - totalMem

    if (code.padding < 0) {
      console.error(`OP Code ${code.opcode} arguments are too long, left with ${code.padding} padding.`)
      paddingError = true
    }
  }

  if (paddingError) {
    throw `One or more opcodes use too many bytes for their instruction`
  }

  return {
    codes: opcodes,
    baseType: fittingType.type,
    opcodeSize: fittingType.bytes
  }
}

function generalOperations() {
  currentCategory = "2.4.1 General Purpose OP Codes"

  opCode("NOP", [], [])

  opCode("PUSHLINE", [uint32("lineno")])
  opCode("RET", [])
  opCode("JMP", [uint32("to")])
  opCode("JMPI0", [uint32("to"), reg("condition")])
  opCode("JMPN0", [uint32("to"), reg("condition")])
  opCode("ASSERT", [reg("condition"), reg("message")])

  opCode("MOV", [reg("from"), reg("to")], [
      "%REG:to% = %REG:from%;"
  ])

  byteSizedOpCode("LOADCONST", [reg("out"), sd("val")], [
      "%REG:out% = %CONST:val,%SIZE%%;"
  ])
  opCode("LOADCONSTSTR", [reg("out"), uint64("straddr")])
}

function stackOperations() {
  currentCategory = "2.4.2 Stack Memory OP Codes"

  byteSizedOpCode("SREAD", [reg("out"), uint64("offset")], [
      "%REG:out% = READ_U%SIZE%(stack, READ_U%SIZE%ARG(%OFFSET:offset%));"
  ])

  byteSizedOpCode("SWRITE", [reg("val"), uint64("offset")], [
      "WRITE_U%SIZE%(stack, READ_U64ARG(%OFFSET:offset%), %REG:val%);"
  ])

  byteSizedOpCode("STORECONST", [uint32("offset"), sd("value")], [
      "WRITE_U%SIZE%(stack, READ_U32ARG(%OFFSET:offset%), %CONST:value,%SIZE%%);"
  ])
}

function globalStackOperations() {
  currentCategory = "2.4.3 Global Memory OP Codes"

  byteSizedOpCode("GREAD", [reg("out"), uint64("offset")], [
    "%REG:out% = READ_U%SIZE%(global, READ_U%SIZE%ARG(%OFFSET:offset%));"
  ])

  byteSizedOpCode("GWRITE", [reg("val"), uint64("offset")], [
    "WRITE_U%SIZE%(global, READ_U64ARG(%OFFSET:offset%), %REG:val%);"
  ])

  byteSizedOpCode("GSTORECONST", [uint32("offset"), sd("value")], [
    "WRITE_U%SIZE%(global, READ_U32ARG(%OFFSET:offset%), %CONST:value,%SIZE%%);"
  ])
}

function closureOperations() {
  currentCategory = "2.4.4 Closure OP Codes"

  opCode("GETSTACKPTR", [reg("out")], [
      "%REG:out% = reinterpret_cast<uint64>(stack);"
  ])

  byteSizedOpCode("CREAD", [reg("closure"), uint64("off"), reg("out")], [
    "%REG:out% = *reinterpret_cast<%UTYPE%*>(%REG:closure% + %CONST:off,32%);"
  ])
  byteSizedOpCode("CWRITE", [reg("closure"), uint64("off"), reg("val")], [
    "*reinterpret_cast<%UTYPE%*>(%REG:closure% + %CONST:off,32%) = %REG:val,%UTYPE%%;"
  ])
}

function heapOperations() {
  currentCategory = "2.4.5 Heap Memory OP Codes"

  opCode("OBJALLOC", [reg("out"), uint32("typeindex")])
  opCode("ARRAYALLOC", [reg("out"), uint32("count"), uint32("typeindex")])

  opCode("INCREFC", [reg("obj")])
  opCode("DECREFC", [reg("obj")])

  byteSizedOpCode("READOBJ", [reg("obj"), reg("out"), uint32("off")], [
      "%REG:out% = *reinterpret_cast<%UTYPE%*>(%REG:obj% + %CONST:off,32% + REFCOUNT_PREFIX_SIZE);"
  ])

  byteSizedOpCode("WRITEOBJ", [reg("obj"), reg("val"), uint32("off")], [
      "*reinterpret_cast<%UTYPE%*>(%REG:obj% + %CONST:off,32% + REFCOUNT_PREFIX_SIZE) = %REG:val,%UTYPE%%;"
  ])

  byteSizedOpCode("READIDX", [reg("obj"), reg("out"), reg("idx")], [
      "{",
      "QsArray arr = castToQsArray(%REG:obj,void*%);",
      "const uint32 idx = %REG:idx,uint32%",
      "",
      "if (idx >= arr.length) {",
    `  throwScriptError("Index out of bounds");`,
      "}",
      "",
      "%REG:out% = arr.get%SHORTHAND%(idx);",
      "}"
  ])

  byteSizedOpCode("WRITEIDX", [reg("obj"), reg("val"), reg("idx")], [
    "{",
    "QsArray arr = castToQsArray(%REG:obj,void*%);",
    "const uint32 idx = %REG:idx,uint32%",
    "",
    "if (idx >= arr.length) {",
    `  throwScriptError("Index out of bounds");`,
    "}",
    "",
    "const %UTYPE% value = %REG:val,%UTYPE%%;",
    "arr.set%SHORTHAND%(idx, value);",
    "}"
  ])

  opCode("ARRLEN", [reg("obj"), reg("out")], [
      "%REG:out% = readQsArrayLength(%REG:obj,void*%);"
  ])
}

function functionCallOperations() {
  currentCategory = "2.4.6 Function Call Instructions"

  // opCode("SETARG", [uint32("index"), uint32("typeindex"), uint32("stackoffset")])
  opCode("LFUNCLOOKUP", [uint32("index"), reg("out")])
  opCode("NFUNCLOOKUP", [uint32("typeindex"), uint64("funcName"), reg("out")])

  opCode("INVOKE", [reg("func")])
}

function conversionOperations() {
  currentCategory = "2.4.7 Conversion Instructions"

  for (const f of NUMBER_TYPES) {
    for (const t of NUMBER_TYPES) {
      if (f == t) {
        continue
      }

      if (f.shorthand.startsWith("U") && t.shorthand.startsWith("U")) {
        // No converting between unsigned types, just
        // truncate bits or add extra leading bits
        continue
      }

      const source: string[] = [
        `%REG:out,${t.fullname}% = static_cast<${t.fullname}>(%REG:in,${f.fullname}%);`
      ]

      opCode(`${f.shorthand}T${t.shorthand}`, UNARY_ARGS, source)
    }
  }
}

function unaryOperations() {
  currentCategory = "2.4.8 Unary Operations"

  opCode("BNEGATE", UNARY_ARGS, ["%REG:out% = ~%REG:in%;"])
  opCode("LNEGATE", UNARY_ARGS, ["%REG:out% = %REG:in% ? 0 : 1;"])

  const numberUnaryOperations = ["NEG", "INC", "DEC"]

  for (const op of numberUnaryOperations) {
    for (const ts of NUMBER_TYPES) {
      let source = ""
      if (op == "NEG") {
        let typeName
        if (!ts.signed) {
          typeName = ts.fullname.substring(1)
        } else {
          typeName = ts.fullname
        }

        source = `%REG:out,${typeName}% = -%REG:in,${typeName}%;`
      } else {
        source = `%REG:out,${ts.fullname}% = %REG:in,${ts.fullname}% ${op == 'INC' ? '+' : '-'} 1;`
      }

      opCode(`${op}${ts.shorthand}`, UNARY_ARGS, [source])
    }
  }
}

function integerBinaryOperations() {
  currentCategory = "2.4.9.1 Integer-only Binary Operations"

  const operations = [
    {code: "LSHIFT", operator: "<<"},
    {code: "RSHIFT", operator: ">>"}
  ]

  for (const op of operations) {
    opCode(op.code, BINARY_ARGS, [
      `%REG:out% = %REG:lhs% ${op.operator} %REG:rhs%;`
    ])
  }
}

function booleanBinaryOperations() {
  currentCategory = "2.4.9.2 Boolean-only Binary Operations"

  const operators = [
    {name: "AND", bitwiseOp: "&", logicOp: "&&"},
    {name: "OR", bitwiseOp: "|", logicOp: "||"},
    {name: "XOR", bitwiseOp: "^", logicOp: ""},
  ]

  for (const {name, bitwiseOp, logicOp} of operators) {
    opCode(`B${name}`, BINARY_ARGS, [`%REG:out% = %REG:lhs% ${bitwiseOp} %REG:rhs%;`])

    if (logicOp.length != 0) {
      opCode(`L${name}`, BINARY_ARGS, [`%REG:out% = %REG:lhs% ${logicOp} %REG:rhs%;`])
    } else {
      opCode(`L${name}`, BINARY_ARGS, [`%REG:out% = !%REG:lhs% != !%REG:rhs%;`])
    }
  }
}

function mathOperations() {
  currentCategory = "2.4.9.3 General Number Binary Operations"

  for (const mathOp of MATH_OPERATIONS) {
    for (const type of NUMBER_TYPES) {
      let source: string
      const tn = type.fullname

      if (mathOp.name == "mod" && !type.integral) {
        source = `%REG:out,${tn}% = fmod(%REG:lhs,${tn}%, %REG:rhs,${tn}%);`
      } else if (mathOp.type == "native") {
        source = `%REG:out,${tn}% = %REG:lhs,${tn}% ${mathOp.operator} %REG:rhs,${tn}%;`
      } else {
        source = `%REG:out,${tn}% = ${mathOp.functionName}(%REG:lhs,${tn}%, %REG:rhs,${tn}%);`
      }

      opCode(`${mathOp.name.toUpperCase()}${type.shorthand}`, BINARY_ARGS, [source])
    }
  }
}

function comparisonOperators() {
  currentCategory = "2.4.9.4 Comparison Operations"

  const equalityOperators = [
    {name: "EQ", operator: "=="},
    {name: "NEQ", operator: "!="},
  ]

  const typedComparisonParams = [...BINARY_ARGS, uint32("typeindex")]

  for (const eqOp of equalityOperators) {
    byteSizedOpCode(eqOp.name, BINARY_ARGS, [
        `%REG:out% = %REG:lhs,%UTYPE%% ${eqOp.operator} %REG:rhs,%UTYPE%%;`
    ])

    const p = eqOp.name == "NEQ" ? "!" : ""

    opCode(`${eqOp.name}ARR`, typedComparisonParams, [
        `%REG:out% = ${p}doArrayEqualityCheck(args[%OFFSET:lhs%], args[%OFFSET:rhs%], %CONST:typeindex,32%);`
    ])

    opCode(`${eqOp.name}STRUCT`, typedComparisonParams, [
      `%REG:out% = ${p}doStructEqualityCheck(args[%OFFSET:lhs%], args[%OFFSET:rhs%], %CONST:typeindex,32%);`
    ])
  }

  const comparisonOperators = [
    {name: "GT", operator: ">", arraySuffix: "== LEFT_GT_RIGHT"},
    {name: "GTE", operator: ">=", arraySuffix: "!= LEFT_LT_RIGHT"},
    {name: "LT", operator: "<", arraySuffix: "== LEFT_LT_RIGHT"},
    {name: "LTE", operator: "<=", arraySuffix: "!= LEFT_GT_RIGHT"},
  ]
  for (const cmpOp of comparisonOperators) {
    for (const ts of NUMBER_TYPES) {
      const tn = ts.fullname

      opCode(`${cmpOp.name}${ts.shorthand}`, BINARY_ARGS, [
          `%REG:out% = %REG:lhs,${tn}% ${cmpOp.operator} %REG:rhs,${tn}%;`
      ])
    }

    opCode(`${cmpOp.name}ARR`, typedComparisonParams, [
        `%REG:out% = doArrayComparison(args[%OFFSET:lhs%], args[%OFFSET:rhs%], %CONST:typeindex,32%) ${cmpOp.arraySuffix};`
    ])
  }
}

function stringArrayOperations() {
  currentCategory = "2.4.9.5 String/Array Operations"

  opCode("STRCONCAT", [reg("lhs"), reg("rhs"), uint32("typeindex"), reg("out")])
  byteSizedOpCode("STRREP", BINARY_ARGS, [
      "%REG:out% = stringRepeat(%REG:lhs,void*%, %REG:rhs,%UTYPE%%, m_vm.getHeap());"
  ])
}

function opCode(name: string, params: InstructionParam[], source?: string[]) {
  const numval = `0x${(opcodes.length).toString(16).toUpperCase().padStart(4, "0")}`

  let instr: Instruction = {
    opcode: name,
    padding: -1,
    params,
    value: numval,
    category: currentCategory,
  }

  if (source != undefined) {
    instr.cSourceCode = source
  }

  opcodes.push(instr)
}

function byteSizedOpCode(name: string, params: InstructionParam[], source?: string[]): void {
  const sizes = [8, 16, 32, 64]
  const bytesizes = [1, 2, 4, 6]

  for (let i = 0; i < sizes.length; i++) {
    const p: InstructionParam[] = []

    const size: number = sizes[i]
    const bsize: number = bytesizes[i]

    for (const param of params) {
      if (param.size == -1) {
        p.push({
          name: param.name,
          size: bsize,
          typename: `uint${size}`
        })
        continue
      }

      p.push({...param})
    }

    let src: string[] | undefined = undefined
    if (source != undefined) {
      src = []
      source.forEach((v, i) => {
        src!![i] = v
          .replaceAll("%UTYPE%", `uint${size}`)
          .replaceAll("%TYPE%", `int${size}`)
          .replaceAll("%SIZE%", `${size}`)
          .replaceAll("%BYTES%", `${bsize}`)
          .replaceAll("%SHORTHAND%", `U${size}`)
          .replaceAll("%LSHORTHAND%", `u${size}`)
      })
    }

    opCode(`${name}${size}`, p, src)
  }
}

function reg(name: string): InstructionParam {
  return {size: 1, typename: "register", name}
}
function uint32(name: string): InstructionParam {
  return {size: 4, typename: "uint32", name}
}
function uint64(name: string): InstructionParam {
  return {size: 8, typename: "uint64", name}
}
function sd(name: string): InstructionParam {
  return {name, size: -1, typename: ""}
}