import {FILE_HEADER, Instruction, NUMBER_TYPES, OpCodeGenResult, writeToFile} from "./common";

interface PlaceholderProcessor {
  placeholderStart: string
  handle(args: string[], ctx: PlaceholderContext): string
}

interface PlaceholderContext {
  opcodeSize: number
  instr: Instruction

  getArgOffset(name: string): number
}

const PROCESSORS: PlaceholderProcessor[] = [
  {
    placeholderStart: "reg",
    handle(args: string[], ctx: PlaceholderContext): string {
      const off = ctx.getArgOffset(args[0])
      let regExpr = `m_registers[args[${ctx.getArgOffset(args[0])}]]`
      if (args.length == 1) {
        return regExpr
      }
      return `REG_AS(args[${off}], ${args[1]})`
    }
  },
  {
    placeholderStart: "const",
    handle(args: string[], ctx: PlaceholderContext): string {
      const off = ctx.getArgOffset(args[0])
      let size = "8"

      if (args.length > 1) {
        size = args[1]
      }

      return `READ_U${size}ARG(${off})`
    }
  },
  {
    placeholderStart: "offset",
    handle(args: string[], ctx: PlaceholderContext): string {
      return `${ctx.getArgOffset(args[0])}`
    }
  }
]

export async function generateEvaluatorSwitchStatement(res: OpCodeGenResult): Promise<void> {
  let out: string = `
${FILE_HEADER}
// This file exists soley to be included into the interpreter 
// file so the switch statement isn't log as hell
// 
// DO NOT ADD THIS FILE TO CMakeLists.txt
// 
  `

  for (const code of res.codes) {
    if (code.cSourceCode == undefined) {
      continue
    }

    out += `\n    case OP_${code.opcode}:`

    let start = 0
    let acLen = code.cSourceCode.length
    let braced = false

    if (code.cSourceCode.length > 0 && code.cSourceCode[start] == "{") {
      out += ` {`
      start++
      acLen -= 1
      braced = true
    }

    for (let i = start; i < acLen; i++) {
      const line = code.cSourceCode[i]
      const formattedLine = replacePlaceholders(line, res.opcodeSize, code)

      out += `\n      ${formattedLine}`
      if (formattedLine.length != 0 && !formattedLine.endsWith(";")) {
        out += ";"
      }
    }

    out += `\n      break;`
    if (braced) {
      out += `\n    }`
    }
  }

  await writeToFile(out, "../src/interpreter/evaluator.cc")
}

function replacePlaceholders(line: string, opcodeSize: number, instr: Instruction): string {
  const ctx: PlaceholderContext = {
    instr,
    opcodeSize,

    getArgOffset(name: string): number {
      let off = 0
      for (const p of instr.params) {
        if (p.name != name) {
          off += p.size
          continue
        }
        return off
      }
      return 0
    }
  }

  for (const prop of PROCESSORS) {
    const str = `%${prop.placeholderStart.toUpperCase()}`
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