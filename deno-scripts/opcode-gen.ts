const numberTypes: string[] = [
  "int8",
  "uint8",
  "int16",
  "uint16",
  "int32",
  "uint32",
  "int64",
  "uint64",
  "float32",
  "float64"
]

const mathOperations = [
  "add",
  "sub",
  "div",
  "mul",
  "mod",
  "exp",
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

interface InstructionParam {
  typename: string
  size: number
  name: string
}

interface Instruction {
  opcode: string
  value: string
  params: InstructionParam[]
  padding: number
}

interface OpCodePadding {
  opcode: string
  padding: number
}

let counter = 0
let opcodes: Instruction[] = []
let paddings: OpCodePadding[] = []

const argsLen = 9

let out = ``

appendGeneratedHeader()
out += `\n// Section numbers that you see here are references to this:`
out += `\n// https://juliewoolie.com/blog/quickscript`
out += `\n// I wrote a bunch of the specification and planning there`
out += `\n//`

out += `\n\n// 4.3.1 General Purpose OP Codes`
appendCode("NOP")

appendCode("PUSHLINE", uint32("lineno"))
appendCode("RET")
appendCode("JMP", uint32("to"))
appendCode("JMPI0", uint32("to"), reg("condition"))
appendCode("JMPN0", uint32("to"), reg("condition"))
appendCode("MOV", reg("from"), reg("to"))

byteSizedOpCode("LOADCONST", reg("out"), sd("val"))
appendCode("LOADCONSTSTR", reg("out"), uint64("straddr"))

out += `\n\n// 4.3.2 Stack Memory OP Codes`
appendCode("STACKALLOC", uint64("bytes"))
appendCode("STACKFREE", uint64("bytes"))
byteSizedOpCode("RSREAD", reg("out"), uint64("offset"))
byteSizedOpCode("RSWRITE", reg("val"), uint64("offset"))
byteSizedOpCode("ASREAD", reg("out"), uint64("offset"))
byteSizedOpCode("ASWRITE", reg("val"), uint64("offset"))

out += `\n\n// 4.3.3 Heap Memory OP Codes`
appendCode("HEAPALLOC", reg("out"), uint64("bytes"))
appendCode("HEAPFREE", reg("addr"), uint64("bytes"))
byteSizedOpCode("READOBJ", reg("obj"), reg("out"), uint32("off"))
byteSizedOpCode("WRITEOBJ", reg("obj"), reg("val"), uint32("off"))
byteSizedOpCode("READIDX", reg("obj"), reg("out"), reg("idx"))
byteSizedOpCode("WRITEIDX", reg("obj"), reg("val"), reg("idx"))

out += `\n\n// 4.3.4 Function Call Instructions`
appendCode("PUSHARG", reg("val"))
appendCode("SETRV", reg("val"))
appendCode("INVOKE", reg("val"), reg("out"))
appendCode("VINVOKE", reg("val"))

out += `\n\n// 4.3.5 Conversion Instructions`
conversionCodes()

out += `\n\n// 4.3.6 Unary Operations`
appendCode("BNEGATE", reg("in"), reg("out"))
appendCode("LNEGATE", reg("in"), reg("out"))
for (const ts of typeShorthands) {
  appendCode(`NEG${ts}`, reg("in"), reg("out"))
}
for (const ts of typeShorthands) {
  appendCode(`INC${ts}`, reg("in"), reg("out"))
}
for (const ts of typeShorthands) {
  appendCode(`DEC${ts}`, reg("in"), reg("out"))
}

out += `\n\n// 4.3.7.1 Integer-only Binary Operations`
byteSizedOpCode("LSHIFT", ...binParams())
byteSizedOpCode("URSHIFT", ...binParams())

out += `\n\n// 4.3.7.2 Boolean-only Binary Operations`
appendCode("BAND", ...binParams())
appendCode("BOR", ...binParams())
appendCode("BXOR", ...binParams())
appendCode("LAND", ...binParams())
appendCode("LOR", ...binParams())
appendCode("LXOR", ...binParams())

out += `\n\n// 4.3.7.3 General Number Binary Operations`
mathOpCodes()

function endsWith(str: string, ...suffixes: string[]): boolean {
  for (const suffix of suffixes) {
    if (str.endsWith(suffix)) {
      return true
    }
  }
  return false
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

  // @ts-ignore
  out += `\n#define PAD_${code.opcode.padEnd(14, ' ')} ${code.padding}`
  paddingDejavu[code.opcode] = code.padding
}

let opcodeLen: number
let basetype: string

if (counter < 256) {
  opcodeLen = 1
  basetype = "uint8"
} else {
  opcodeLen = 2
  basetype = "uint16"
}

const instructionLen = opcodeLen + argsLen

out = `#ifndef QUICKSCRIPT_OPCODES_H
#define QUICKSCRIPT_OPCODES_H

#include "../common.h"

#define LENGTH_OPCODE ${opcodeLen}
#define LENGTH_ARGS ${argsLen}
#define LENGTH_INSTRUCTION ${instructionLen}` + out

function nextOpCode(): string {
  // @ts-ignore
  return `0x${(counter++).toString(16).toUpperCase().padStart(4, "0")}`
}

function getPadding(params: InstructionParam[]): number {
  let pad = argsLen
  for (const p of params) {
    pad -= p.size
  }
  return pad
}

function appendCode(name: string, ...params: InstructionParam[]) {
  const numval = nextOpCode()

  const pad = getPadding(params)

  let instr: Instruction = {
    opcode: name,
    padding: pad,
    params,
    value: numval
  }

  opcodes.push(instr)

  // @ts-ignore
  out += `\n#define OP_${name.padEnd(15, " ")} ${numval}`

  paddings.push({padding: pad, opcode: name})
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

    appendCode(`${name}${size}`, ...p)
  }
}

function conversionCodes() {
  for (const f of typeShorthands) {
    for (const t of typeShorthands) {
      if (f == t) {
        continue
      }
      appendCode(`${f}T${t}`, reg("from"), reg("out"))
    }
  }
}

function mathOpCodes() {
  for (const mathOp of mathOperations) {
    for (const type of typeShorthands) {
      appendCode(`${mathOp.toUpperCase()}_${type}`, ...binParams())
    }
  }
}

out += `\n\ntypedef ${basetype} opcode;\n\nconststring opcode_name(opcode code);`
out += `\n\n#endif //QUICKSCRIPT_OPCODES_H`

// @ts-ignore
await writeToFile("../src/interpreter/opcodes.h")

out = `#include "opcodes.h"`
appendGeneratedHeader()
out += `

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

// @ts-ignore
await writeToFile("../src/interpreter/opcodes.cc")

out = `#ifndef QUICKSCRIPT_OPCODESPEC_H
#define QUICKSCRIPT_OPCODESPEC_H`
appendGeneratedHeader()

out += `

#include <cstdarg>
#include "../interpreter/opcodes.h"

void appendOpCodeData(uint8* buf, opcode code, va_list list);

#endif //QUICKSCRIPT_OPCODESPEC_H`

// @ts-ignore
await writeToFile("../src/codegen/opcodespec.h")

out = `#include "opcodespec.h"
#include <cstring>`

appendGeneratedHeader()

out += `

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
    out += `${getOffsetVar(off)}`
    if (mustCast || off > 0) {
      out += `)`
    }

    out += ` = `
    if (mustCastArg) {
      out += `static_cast<${typename}>(`
    }
    out += `va_arg(list, ${typename})`
    if (mustCastArg) {
      out += `)`
    }
    out += `;`

    off += p.size
  }

  if (c.padding > 0) {
    out += `\n      memset(${getOffsetVar(off)}, 0, ${c.padding});`
  }

  out += `\n      break;`
}


out += `\n  }\n}`

function getOffsetVar(off: number): string {
  if (off == 0) {
    return `buf`
  }
  return `buf + ${off}`
}

// @ts-ignore
await writeToFile("../src/codegen/opcodespec.cc")

out = `# OP Code specification Table
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

// @ts-ignore
await writeToFile("../opcode-spec.md")

// @ts-ignore
async function writeToFile(fname: string): Promise<void> {
  // @ts-ignore
  await Deno.writeTextFile(fname, out)
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

function appendGeneratedHeader() {
  out += `\n\n//`
  out += `\n// This file was automatically generated by a deno script in:`
  out += `\n// deno-scripts/opcode-gen.ts`
  out += `\n// `
}