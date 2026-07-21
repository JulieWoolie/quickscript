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

let counter = 0
let opcodes: string[] = []

let out = `
#ifndef QUICKSCRIPT_OPCODES_H
#define QUICKSCRIPT_OPCODES_H

#include "../common.h"

#define LENGTH_OPCODE 2
#define LENGTH_ARGS 9
#define LENGTH_INSTRUCTION (LENGTH_OPCODE+LENGTH_ARGS)`

out += `\n\n//`
out += `\n// Section numbers that you see here are references to this:`
out += `\n// https://juliewoolie.com/blog/quickscript`
out += `\n// I wrote a bunch of the specification and planning there`
out += `\n//`

out += `\n\n// 4.3.1 General Purpose OP Codes`
appendCode("NOP")

appendCode("PUSHLINE")
appendCode("RET")
appendCode("JMP")
appendCode("JMPI0")
appendCode("JMPN0")

byteSizedOpCode("LOADCONST")

out += `\n\n// 4.3.2 Stack Memory OP Codes`
appendCode("STACKALLOC")
appendCode("STACKFREE")
byteSizedOpCode("RSREAD")
byteSizedOpCode("RSWRITE")
byteSizedOpCode("ASREAD")
byteSizedOpCode("ASWRITE")

out += `\n\n// 4.3.3 Heap Memory OP Codes`
appendCode("HEAPALLOC")
appendCode("HEAPFREE")
byteSizedOpCode("READOBJ")
byteSizedOpCode("WRITEOBJ")

out += `\n\n// 4.3.4 Function Call Instructions`
appendCode("PUSHARG")
appendCode("SETRV")
appendCode("INVOKE")

out += `\n\n// 4.3.5 Conversion Instructions`
conversionCodes()

out += `\n\n// 4.3.6 Unary Operations`
appendCode("BNEGATE")
appendCode("LNEGATE")

out += `\n\n// 4.3.7.1 Integer-only Binary Operations`
byteSizedOpCode("LSHIFT")
byteSizedOpCode("URSHIFT")

out += `\n\n// 4.3.7.2 Boolean-only Binary Operations`
appendCode("BAND")
appendCode("BOR")
appendCode("BXOR")
appendCode("LAND")
appendCode("LOR")
appendCode("LXOR")

out += `\n\n// 4.3.7.3 General Number Binary Operations`
mathOpCodes()

function nextOpCode(): string {
  return `0x${(counter++).toString(16).toUpperCase().padStart(4, "0")}`
}

function appendCode(name) {
  out += `\n#define OP_${name.padEnd(15, " ")} ${nextOpCode()}`
  opcodes.push(name)
}

function byteSizedOpCode(name: string): void {
  appendCode(`${name}8`)
  appendCode(`${name}16`)
  appendCode(`${name}32`)
  appendCode(`${name}64`)
}

function conversionCodes() {
  for (const f of typeShorthands) {
    for (const t of typeShorthands) {
      if (f == t) {
        continue
      }
      appendCode(`${f}T${t}`)
    }
  }
}

function mathOpCodes() {
  for (const mathOp of mathOperations) {
    for (const type of typeShorthands) {
      appendCode(`${mathOp.toUpperCase()}_${type}`)
    }
  }
}

out += `\n\ntypedef uint16 opcode;\n\nconststring opcode_name(opcode code);`
out += `\n\n#endif //QUICKSCRIPT_OPCODES_H`

await writeToFile("../src/interpreter/opcodes.h")

out = `#include "opcodes.h"`

out += `\n\n\nconststring opcode_name(opcode code) {\n  switch (code) {`
for (const code of opcodes) {
  if (code == "NOP") {
    continue
  }
  out += `\n    case OP_${code}: return "${code}";`
}
out += `\n    default: return "NOP";`
out += `\n  }\n}\n`

await writeToFile("../src/interpreter/opcodes.cc")

async function writeToFile(fname: string): Promise<void> {
  await Deno.writeTextFile(fname, out)
}