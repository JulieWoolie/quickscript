import {
  BINARY_ARGS,
  Instruction,
  INSTRUCTION_LENGTH,
  InstructionParam,
  NUMBER_TYPES,
  OpCodeGenResult, UNARY_ARGS
} from "./common";

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
  heapOperations()
  functionCallOperations()
  conversionOperations()
  unaryOperations()
  integerBinaryOperations()
  booleanBinaryOperations()
  mathOperations()
  comparisonOperators()
  stringArrayOperations()

  const potentialSizes = [1, 2, 4]
  let memSize = 0
  let baseType = ""

  for (const sz of potentialSizes) {
    let max = parseInt("FF".repeat(sz), 16)

    if (opcodes.length > max) {
      continue
    }

    memSize = sz
    baseType = `uint${sz * 8}`

    break
  }

  if (baseType == "") {
    throw "Too many OP Codes, no valid number type can represent all op codes"
  }

  for (const code of opcodes) {
    let totalMem = memSize
    for (const arg of code.params) {
      totalMem += arg.size
    }
    code.padding = INSTRUCTION_LENGTH - totalMem
  }

  return {
    codes: opcodes,
    baseType: baseType,
    opcodeSize: memSize
  }
}

function generalOperations() {
  currentCategory = "4.3.1 General Purpose OP Codes"

  opCode("NOP", [], [])

  opCode("PUSHLINE", [uint32("lineno")])
  opCode("RET", [])
  opCode("JMP", [uint32("to")])
  opCode("JMPI0", [uint32("to"), reg("condition")])
  opCode("JMPN0", [uint32("to"), reg("condition")])

  opCode("MOV", [reg("from"), reg("to")], [
      "registers[from] = registers[to];"
  ])

  byteSizedOpCode("LOADCONST", [reg("out"), sd("val")], [
      "registers[out] = val;"
  ])
  opCode("LOADCONSTSTR", [reg("out"), uint64("straddr")])
}

function stackOperations() {
  currentCategory = "4.3.2 Stack Memory OP Codes"
  opCode("STACKALLOC", [uint64("bytes")])
  opCode("STACKFREE", [uint64("bytes")])
  byteSizedOpCode("SREAD", [reg("out"), uint64("offset")])
  byteSizedOpCode("SWRITE", [reg("val"), uint64("offset")])
  byteSizedOpCode("GREAD", [reg("out"), uint64("offset")])
  byteSizedOpCode("GWRITE", [reg("val"), uint64("offset")])

  byteSizedOpCode("STORECONST", [uint32("offset"), sd("value")])
}

function heapOperations() {
  currentCategory = "4.3.3 Heap Memory OP Codes"

  opCode("HEAPALLOC", [reg("out"), uint64("bytes")])
  opCode("HEAPFREE", [reg("addr"), uint64("bytes")])
  byteSizedOpCode("READOBJ", [reg("obj"), reg("out"), uint32("off")])
  byteSizedOpCode("WRITEOBJ", [reg("obj"), reg("val"), uint32("off")])
  byteSizedOpCode("READIDX", [reg("obj"), reg("out"), reg("idx")])
  byteSizedOpCode("WRITEIDX", [reg("obj"), reg("val"), reg("idx")])
}

function functionCallOperations() {
  currentCategory = "4.3.4 Function Call Instructions"

  opCode("SETARGTYPE", [uint32("index"), uint32("typeindex")])
  opCode("INVOKE", [reg("func"), reg("out")])
  opCode("LFUNCLOOKUP", [uint32("index"), reg("out")])
  opCode("NFUNCLOOKUP", [uint32("typeindex"), uint64("funcName"), reg("out")])
}

function conversionOperations() {
  currentCategory = "4.3.5 Conversion Instructions"

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
        `const ${f.fullname} f = *reinterpret_cast<${f.fullname}*>(registers + in);`,
        `registers[out] = static_cast<${t.fullname}>(f);`
      ]

      opCode(`${f.shorthand}T${t.shorthand}`, UNARY_ARGS, source)
    }
  }
}

function unaryOperations() {
  currentCategory = "4.3.6 Unary Operations"

  opCode("BNEGATE", UNARY_ARGS)
  opCode("LNEGATE", UNARY_ARGS)

  const numberUnaryOperations = ["NEG", "INC", "DEC"]

  for (const op of numberUnaryOperations) {
    for (const ts of NUMBER_TYPES) {
      opCode(`${op}${ts.shorthand}`, UNARY_ARGS)
    }
  }
}

function integerBinaryOperations() {
  currentCategory = "4.3.7.1 Integer-only Binary Operations"

  const operations = [
    {code: "LSHIFT", operator: "<<"},
    {code: "RSHIFT", operator: ">>"}
  ]

  for (const op of operations) {
    opCode(op.code, BINARY_ARGS, [
      `registers[out] = REGREAD(lhs, uint64) ${op.operator} REGREAD(rhs, uint64);`
    ])
  }
}

function booleanBinaryOperations() {
  currentCategory = "4.3.7.2 Boolean-only Binary Operations"

  opCode("BAND", BINARY_ARGS)
  opCode("BOR", BINARY_ARGS)
  opCode("BXOR", BINARY_ARGS)
  opCode("LAND", BINARY_ARGS)
  opCode("LOR", BINARY_ARGS)
  opCode("LXOR", BINARY_ARGS)
}

function mathOperations() {
  currentCategory = "4.3.7.3 General Number Binary Operations"

  for (const mathOp of MATH_OPERATIONS) {
    for (const type of NUMBER_TYPES) {
      let source: string[]

      if (mathOp.type == "native") {
        source = [`registers[out] = REGREAD(lhs, ${type.fullname}) ${mathOp.operator} REGREAD(rhs, ${type.fullname});`]
      } else {
        source = [`registers[out] = ${mathOp.functionName}(REGREAD(lhs, ${type.fullname}), REGREAD(rhs, ${type.fullname}));`]
      }

      opCode(`${mathOp.name.toUpperCase()}${type.shorthand}`, BINARY_ARGS, source)
    }
  }
}

function comparisonOperators() {
  currentCategory = "4.3.7.4 Comparison Operations"

  const equalityOperators = [
    {name: "EQ", operator: "=="},
    {name: "NEQ", operator: "!="},
  ]
  for (const eqOp of equalityOperators) {
    byteSizedOpCode(eqOp.name, BINARY_ARGS, [
        `registers[out] = REGREAD(lhs, %UTYPE%) ${eqOp.operator} REGREAD(rhs, %UTYPE%);`
    ])

    opCode(`${eqOp.name}ARR`, BINARY_ARGS)
    opCode(`${eqOp.name}STRUCT`, BINARY_ARGS)
  }

  const comparisonOperators = [
    {name: "GT", operator: ">"},
    {name: "GTE", operator: ">="},
    {name: "LT", operator: "<"},
    {name: "LTE", operator: "<="}
  ]
  for (const cmpOp of comparisonOperators) {
    for (const ts of NUMBER_TYPES) {
      opCode(`${cmpOp.name}${ts.shorthand}`, BINARY_ARGS, [
          `registers[out] = REGREAD(lhs, ${ts.fullname}) ${cmpOp.operator} REGREAD(rhs, ${ts.fullname});`
      ])
    }
    opCode(`${cmpOp.name}ARR`, BINARY_ARGS)
  }
}

function stringArrayOperations() {
  currentCategory = "4.3.7.5 String/Array Operations"

  opCode("STRCONCAT", BINARY_ARGS)
  byteSizedOpCode("STRREP", BINARY_ARGS)
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