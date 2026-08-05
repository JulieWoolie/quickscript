import {
  FILE_HEADER,
  Instruction,
  INSTRUCTION_LENGTH,
  InstructionParam,
  NUMBER_TYPES, OpCodeGenResult,
  writeToFile
} from "./common";
import {generateOpCodes} from "./opcode-gen";
import {generateEvaluatorSwitchStatement} from "./evaluator-gen";
import {generateDocs} from "./docs";

async function createOpCodesHeader(res: OpCodeGenResult): Promise<void> {
  const opcodes = res.codes
  let out = `#ifndef QUICKSCRIPT_OPCODES_H
#define QUICKSCRIPT_OPCODES_H

#include "../common.h"

#define LENGTH_OPCODE ${res.opcodeSize}
#define LENGTH_ARGS ${INSTRUCTION_LENGTH - res.opcodeSize}
#define LENGTH_INSTRUCTION ${INSTRUCTION_LENGTH}`

  out += "\n\n"
  out += FILE_HEADER
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

  out += `

#define LAST_OPCODE OP_${opcodes[opcodes.length - 1].opcode}
#define OPCODE_COUNT ${opcodes.length}

typedef ${res.baseType} opcode;

conststring opcode_name(opcode code);

#endif //QUICKSCRIPT_OPCODES_H`

  await writeToFile(out, "../src/interpreter/opcodes.h")
}

async function createOpCodesSourceFile(opcodes: Instruction[]): Promise<void> {
  let out = `#include "opcodes.h"

${FILE_HEADER}

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

async function generateConversionCompileMethod(): Promise<void> {
  let out = `#ifndef QUICKSCRIPT_TYPE_CONV_H
#define QUICKSCRIPT_TYPE_CONV_H

${FILE_HEADER}

#include "../interpreter/opcodes.h"
#include "../types/types.h"

opcode conversionOpCode(primitivekind from, primitivekind to);

#endif // QUICKSCRIPT_TYPE_CONV_H`

  await writeToFile(out, "../src/codegen/type_conv.h")

  out = `
#include "type_conv.h"
  
${FILE_HEADER}

opcode conversionOpCode(primitivekind from, primitivekind to) {
  switch (from) {`

  for (const fromtype of NUMBER_TYPES) {
    if (fromtype.shorthand == "U8") {
      out += `\n    case PK_BOOL:`
    }
    out += `\n    case PK_${fromtype.fullname.toUpperCase()}:\n      switch(to) {`
    for (const totype of NUMBER_TYPES) {
      if (fromtype == totype) {
        continue
      }
      if (fromtype.shorthand.startsWith("U")
          && totype.shorthand.startsWith("U")
      ) {
        continue
      }

      const opcode = `${fromtype.shorthand}T${totype.shorthand}`

      out += `\n        case PK_${totype.fullname.toUpperCase()}: return OP_${opcode};`
    }
    out += `\n        default: return OP_NOP;`
    out += `\n      }`
    out += `\n      return OP_NOP;`
  }

  out += `
    default: return OP_NOP;
  }
}`

  await writeToFile(out, "../src/codegen/type_conv.cc")
}

async function main(): Promise<void> {
  const res = generateOpCodes()
  const codes = res.codes

  console.log(`Generated ${codes.length} OP Codes, using ${res.baseType} (${res.opcodeSize} byte unsigned integer) as opcode type`)

  await createOpCodesHeader(res)
  await createOpCodesSourceFile(codes)
  await generateDocs(res)
  await generateConversionCompileMethod()
  await generateEvaluatorSwitchStatement(res)
}

// @ts-ignore
await main()