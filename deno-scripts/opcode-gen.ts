interface InstructionParam {
  typename: string
  size: number
  name: string
}

interface OpCodePadding {
  opcode: string
  padding: number
}

interface Instruction extends OpCodePadding{
  value: string
  params: InstructionParam[]
  category: string
}

interface OpCodeGenResult {
  opcodes: Instruction[]
  paddings: OpCodePadding[]
}

const ARGUMENTS_LENGTH = 9

function generateOpCodes(): OpCodeGenResult {
  let currentCategory: string = ""

  const opcodes: Instruction[] = []
  const paddings: OpCodePadding[] = []

  function getPadding(params: InstructionParam[]): number {
    let pad = ARGUMENTS_LENGTH
    for (const p of params) {
      pad -= p.size
    }
    return pad
  }

  function opCode(name: string, ...params: InstructionParam[]) {
    const numval = `0x${(opcodes.length).toString(16).toUpperCase().padStart(4, "0")}`
    const pad = getPadding(params)

    let instr: Instruction = {
      opcode: name,
      padding: pad,
      params,
      value: numval,
      category: currentCategory
    }

    opcodes.push(instr)
    paddings.push(instr)
  }

  function byteSizedOpCode(name: string, ...params: InstructionParam[]): void {
    const sizes = [8, 16, 32, 64]
    const bytesizes = [1, 2, 4, 6]

    let hasTypeVariance = false
    for (const p of params) {
      if (p.size == -1) {
        hasTypeVariance = true
        break
      }
    }

    if (!hasTypeVariance) {
      paddings.push({
        opcode: name,
        padding: getPadding(params)
      })
    }

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

      opCode(`${name}${size}`, ...p)
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
  function binParams(): InstructionParam[] {
    return [reg("lhs"), reg("rhs"), reg("out")]
  }

  currentCategory = "4.3.1 General Purpose OP Codes"
  opCode("NOP")

  opCode("PUSHLINE", uint32("lineno"))
  opCode("RET")
  opCode("JMP", uint32("to"))
  opCode("JMPI0", uint32("to"), reg("condition"))
  opCode("JMPN0", uint32("to"), reg("condition"))
  opCode("MOV", reg("from"), reg("to"))

  byteSizedOpCode("LOADCONST", reg("out"), sd("val"))
  opCode("LOADCONSTSTR", reg("out"), uint64("straddr"))

  currentCategory = "4.3.2 Stack Memory OP Codes"
  opCode("STACKALLOC", uint64("bytes"))
  opCode("STACKFREE", uint64("bytes"))
  byteSizedOpCode("RSREAD", reg("out"), uint64("offset"))
  byteSizedOpCode("RSWRITE", reg("val"), uint64("offset"))
  byteSizedOpCode("ASREAD", reg("out"), uint64("offset"))
  byteSizedOpCode("ASWRITE", reg("val"), uint64("offset"))

  currentCategory = "4.3.3 Heap Memory OP Codes"
  opCode("HEAPALLOC", reg("out"), uint64("bytes"))
  opCode("HEAPFREE", reg("addr"), uint64("bytes"))
  byteSizedOpCode("READOBJ", reg("obj"), reg("out"), uint32("off"))
  byteSizedOpCode("WRITEOBJ", reg("obj"), reg("val"), uint32("off"))
  byteSizedOpCode("READIDX", reg("obj"), reg("out"), reg("idx"))
  byteSizedOpCode("WRITEIDX", reg("obj"), reg("val"), reg("idx"))

  currentCategory = "4.3.4 Function Call Instructions"
  opCode("PUSHARG", reg("val"))
  opCode("SETRV", reg("val"))
  opCode("INVOKE", reg("val"), reg("out"))
  opCode("VINVOKE", reg("val"))

  const mathOperations = [
    "add",
    "sub",
    "div",
    "mul",
    "mod",
    "pow",
  ]

  const typeShorthands: string[] = [
    "I8",
    "U8",
    "I16",
    "U16",
    "I32",
    "U32",
    "I64",
    "U64",
    "F32",
    "F64"
  ]

  currentCategory = "4.3.5 Conversion Instructions"
  for (const f of typeShorthands) {
    for (const t of typeShorthands) {
      if (f == t) {
        continue
      }
      opCode(`${f}T${t}`, reg("from"), reg("out"))
    }
  }

  currentCategory = "4.3.6 Unary Operations"
  opCode("BNEGATE", reg("in"), reg("out"))
  opCode("LNEGATE", reg("in"), reg("out"))
  for (const ts of typeShorthands) {
    opCode(`NEG${ts}`, reg("in"), reg("out"))
  }
  for (const ts of typeShorthands) {
    opCode(`INC${ts}`, reg("in"), reg("out"))
  }
  for (const ts of typeShorthands) {
    opCode(`DEC${ts}`, reg("in"), reg("out"))
  }

  currentCategory = "4.3.7.1 Integer-only Binary Operations"
  byteSizedOpCode("LSHIFT", ...binParams())
  byteSizedOpCode("URSHIFT", ...binParams())

  currentCategory = "4.3.7.2 Boolean-only Binary Operations"
  opCode("BAND", ...binParams())
  opCode("BOR", ...binParams())
  opCode("BXOR", ...binParams())
  opCode("LAND", ...binParams())
  opCode("LOR", ...binParams())
  opCode("LXOR", ...binParams())

  currentCategory = "4.3.7.3 General Number Binary Operations"
  for (const mathOp of mathOperations) {
    for (const type of typeShorthands) {
      opCode(`${mathOp.toUpperCase()}_${type}`, ...binParams())
    }
  }

  currentCategory = "4.3.7.4 Comparison Operations"
  const equalityOperators = ["EQ", "NEQ"]
  for (const eqOp of equalityOperators) {
    byteSizedOpCode(eqOp, ...binParams())
    opCode(`${eqOp}ARR`, ...binParams())
    opCode(`${eqOp}STRUCT`, ...binParams())
  }

  const comparisonOperators = ["GT", "GTE", "LT", "LTE"]

  for (const cmpOp of comparisonOperators) {
    for (const ts of typeShorthands) {
      opCode(`${cmpOp}${ts}`, ...binParams())
    }
    opCode(`${cmpOp}ARR`, ...binParams())
  }

  return {opcodes, paddings}
}

async function createOpCodesHeader(res: OpCodeGenResult): Promise<void> {
  const opcodes = res.opcodes
  const paddings = res.paddings

  let out = ``

  out += "\n\n"
  out += createGeneratedHeader()
  out += `\n// Section numbers that you see here are references to this:`
  out += `\n// https://juliewoolie.com/blog/quickscript`
  out += `\n// I wrote a bunch of the specification and planning there`
  out += `\n//`

  const groupedCodes: {[cat: string]: Instruction[]} = {}

  opcodes.forEach(v => {
    let arr = groupedCodes[v.category]
    if (!arr) {
      arr = []
      groupedCodes[v.category] = arr
    }
    arr.push(v)
  })

  for (const group in groupedCodes) {
    out += `\n\n// ${group}`
    for (const code of groupedCodes[group]) {
      out += `\n#define OP_${code.opcode.padEnd(15, ' ')} ${code.value}`
    }
  }

  out += `\n`
  const paddingDejavu: {[key: string]: number} = {}
  for (const code of paddings) {
    let opcode = code.opcode;

    if (endsWith(opcode, "16", "32", "64")) {
      opcode = opcode.substring(0, opcode.length - 2)
    } else if (endsWith(opcode, "8")) {
      opcode = opcode.substring(0, opcode.length - 1)
    }

    let printedPad = paddingDejavu[opcode]
    if (printedPad != undefined && printedPad == code.padding) {
      continue
    }

    out += `\n#define PAD_${code.opcode.padEnd(14, ' ')} ${code.padding}`
    paddingDejavu[code.opcode] = code.padding
  }

  let opcodeLen: number
  let basetype: string

  if (opcodes.length < 256) {
    opcodeLen = 1
    basetype = "uint8"
  } else {
    opcodeLen = 2
    basetype = "uint16"
  }

  const instructionLen = opcodeLen + ARGUMENTS_LENGTH

  out = `#ifndef QUICKSCRIPT_OPCODES_H
#define QUICKSCRIPT_OPCODES_H

#include "../common.h"

#define LENGTH_OPCODE ${opcodeLen}
#define LENGTH_ARGS ${ARGUMENTS_LENGTH}
#define LENGTH_INSTRUCTION ${instructionLen}` + out

  out += `\n\ntypedef ${basetype} opcode;\n\nconststring opcode_name(opcode code);`
  out += `\n\n#endif //QUICKSCRIPT_OPCODES_H`

  await writeToFile(out, "../src/interpreter/opcodes.h")
}

async function createOpCodesSourceFile(opcodes: Instruction[]): Promise<void> {
  let out = `#include "opcodes.h"

${createGeneratedHeader()}

conststring opcode_name(opcode code) {
  switch (code) {`

  for (const code of opcodes) {
    if (code.opcode == "NOP") {
      continue
    }
    out += `\n    case OP_${code.opcode}: return "${code.opcode}";`
  }
  out += `
    default: return "NOP";
  }
}
`
  await writeToFile(out, "../src/interpreter/opcodes.cc")
}

async function createSpecHeader(): Promise<void> {
  let out = `#ifndef QUICKSCRIPT_OPCODESPEC_H
#define QUICKSCRIPT_OPCODESPEC_H

${createGeneratedHeader()}

#include <cstdarg>
#include "../interpreter/opcodes.h"

void appendOpCodeData(uint8* buf, opcode code, va_list list);

#endif //QUICKSCRIPT_OPCODESPEC_H`

  await writeToFile(out, "../src/codegen/opcodespec.h")
}

async function createSpecSourceFile(opcodes: Instruction[]): Promise<void> {
  let out = `#include "opcodespec.h"
#include <cstring>

${createGeneratedHeader()}

void appendOpCodeData(uint8* buf, opcode code, va_list list) {
  switch (code) {`

  const opcodeGroups: {[sig: string]: Instruction[]} = {}

  for (const code of opcodes) {
    const params = code.params;
    let sig = ""
    params.forEach((v, i) => {
      if (i != 0) {
        sig += ","
      }
      sig += `${v.name}:${v.typename}`
    })

    let arr: Instruction[] | undefined = opcodeGroups[sig]
    if (!arr) {
      arr = []
      opcodeGroups[sig] = arr
    }

    arr.push(code)
  }

  const castingRequiredTypes = ["uint8", "uint16"]

  for (const sig in opcodeGroups) {
    let arr = opcodeGroups[sig]
    const c = arr[0]

    out += `\n\n    //`
    out += `\n    // Padding: ${c.padding}`
    out += `\n    // Arguments:`

    c.params.forEach((v, i) => {
      out += `\n    //   [${i}] ${v.name}: ${v.typename}`
    })
    out += `\n    //`

    for (const code of arr) {
      out += `\n    case OP_${code.opcode}:`
    }

    let off = 0
    for (const p of c.params) {
      let typename = p.typename
      let mustCast = true
      let mustCastArg = false
      let varname: string = "buf"

      if (off != 0) {
        varname += ` + ${off}`
      }

      if (typename == "register") {
        typename = "uint8"
        mustCast = false
        mustCastArg = true
      } else if (typename == "uint8") {
        mustCast = false
        mustCastArg = true
      }

      out += `\n      *`
      if (mustCast) {
        out += `reinterpret_cast<${typename}*>`
      }
      if (mustCast || off > 0) {
        out += "("
      }
      out += `${varname}`
      if (mustCast || off > 0) {
        out += `)`
      }

      let vaArgCallType: string = typename
      if (castingRequiredTypes.includes(typename)) {
        vaArgCallType = "int32"
      }

      out += ` = `
      if (mustCastArg) {
        out += `static_cast<${typename}>(`
      }
      out += `va_arg(list, ${vaArgCallType})`
      if (mustCastArg) {
        out += `)`
      }
      out += `;`

      off += p.size
    }

    if (c.padding > 0) {
      let varname: string = "buf"
      if (off != 0) {
        varname += ` + ${off}`
      }

      out += `\n      memset(${varname}, 0, ${c.padding});`
    }

    out += `\n      break;`
  }


  out += `\n  }\n}`

  await writeToFile(out, "../src/codegen/opcodespec.cc")
}

async function generateMarkdownSpec(opcodes: Instruction[]): Promise<void> {
  let out = `# OP Code specification Table
| OP Code | Padding | Arguments |
|--|--|--|`
  for (const code of opcodes) {
    out += `\n|\`${code.opcode}\`|${code.padding}|`
    code.params.forEach((v, i) => {
      if (i != 0) {
        out += `, `
      }
      out += `\`${v.name}: ${v.typename}\``
    })
    out += "|"
  }

  await writeToFile(out, "../opcode-spec.md")
}

async function main(): Promise<void> {
  const res = generateOpCodes()
  const codes = res.opcodes

  await createOpCodesHeader(res)
  await createOpCodesSourceFile(codes)

  await createSpecHeader()
  await createSpecSourceFile(codes)

  await generateMarkdownSpec(codes)
}

function endsWith(str: string, ...suffixes: string[]): boolean {
  for (const suffix of suffixes) {
    if (str.endsWith(suffix)) {
      return true
    }
  }
  return false
}

async function writeToFile(out: string, fname: string): Promise<void> {
  // @ts-ignore
  await Deno.writeTextFile(fname, out)
}

function createGeneratedHeader(): string {
  let out = `//`
  out += `\n// This file was automatically generated by a deno script in:`
  out += `\n// deno-scripts/opcode-gen.ts`
  out += `\n// `
  return out
}

// @ts-ignore
await main()