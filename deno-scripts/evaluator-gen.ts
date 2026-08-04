import {FILE_HEADER, Instruction, NUMBER_TYPES, OpCodeGenResult, writeToFile} from "./common";

interface PlaceholderProcessor {
  placeholderStart: string
  handle(args: string[], ctx: PlaceholderContext): string
}

interface PlaceholderContext {
  opcodeSize: number
  instr: Instruction
}

const PROCESSORS: PlaceholderProcessor[] = [
  {
    placeholderStart: "reg",
    handle(args: string[], ctx: PlaceholderContext): string {
      return `instr.bytes[${args}]`
    }
  }
]

export async function generateEvaluatorSwitchStatement(res: OpCodeGenResult): Promise<void> {
  let out: string = ``

  for (const code of res.codes) {
    if (code.cSourceCode == undefined) {
      continue
    }

    out += `\ncase OP_${code.opcode}: {`

    let pOff = 0
    for (const param of code.params) {
      let tn = param.typename
      if (tn == "register") {
        tn = "uint8"
      }

      out += `\n  const ${tn} ${param.name} = *`

      let mustCast = tn != "uint8"
      if (mustCast) {
        out += `*reinterpret_cast<${tn}*>`
      }
      if (mustCast || pOff > 0) {
        out += "("
      }
      out += `args`
      if (pOff != 0) {
        out += `+ ${pOff}`
      }
      if (mustCast || pOff > 0) {
        out += ")"
      }
      out += ";"

      pOff += param.size
    }

    for (const line of code.cSourceCode) {
      out += `\n  ${replacePlaceholders(line, res.opcodeSize, code)}`
    }

    out += `\n  break;`
    out += `\n}`
  }

  await writeToFile(out, "./evaluator.c")
}

function replacePlaceholders(line: string, opcodeSize: number, instr: Instruction): string {
  const ctx: PlaceholderContext = {instr, opcodeSize}

  for (const prop of PROCESSORS) {
    const str = `%${prop.placeholderStart}`
    while (line.includes(str)) {
      const startIdx = line.indexOf(str)
      const endIdx = line.indexOf('%', startIdx + str.length)

      if (endIdx == -1) {
        break
      }

      let argStr = line.substring(startIdx + str.length, endIdx)
      let args: string[] = []

      if (argStr.length != 0 && argStr[0] == ':') {
        argStr = argStr.substring(1)
        args = argStr.split(",")
      }

      const replacement = prop.handle(args, ctx)
      line = line.substring(0, startIdx) + replacement + line.substring(endIdx + 1)
    }
  }
  return line
}