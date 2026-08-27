import {
  Instruction,
  INSTRUCTION_LENGTH,
  InstructionParam,
  OpCodeGenResult,
  writeToFile
} from "../common";

interface Table {
  caption?: string
  headers: string[]
  rows: string[][]
}

function printTableToHtml(table: Table): string {
  let out = "<table>"

  if (table.caption) {
    out += `\n  <caption>${table.caption}</caption>`
  }

  out += "\n  <thead>\n    <tr>"
  table.headers.forEach(v => {
    out += `<th>${v}</th>`
  })
  out += `\n  </tr>\n  </thead>`

  out += `\n  <tbody>`
  for (const row of table.rows) {
    out += `\n    <tr>`
    for (const cell of row) {
      out += `<td>${cell}</td>`
    }
    out += `</tr>`
  }
  out += `\n  </tbody>\n</table>`

  return out
}

function printTableToMarkdown(table: Table): string {
  let allRows: string[][] = [
    table.headers,

    ...table.rows.map(r => r.map(function(c) {
      let n = c.replaceAll("<code>", "`").replaceAll("</code>", "`")
      if (n == "``") {
        return ""
      }
      return n
    }))
  ]

  allRows = allRows.map(r => r.map(c => ` ${c} `))

  const biggestRowLengths: number[] = []

  for (const row of allRows) {
    for (let i = 0; i < row.length; i++) {
      biggestRowLengths[i] = Math.max(row[i].length, biggestRowLengths[i] ?? 0)
    }
  }

  let row2: string[] = []
  for (const big of biggestRowLengths) {
    row2.push("-".repeat(big))
  }
  allRows.splice(1, 0, row2)

  let out = ""
  for (let i = 0; i < allRows.length; i++) {
    if (i != 0) {
      out += `\n`
    }

    let row = allRows[i]

    out += `|`
    for (let j = 0; j < row.length; j++) {
      if (j != 0) {
        out += "|"
      }
      out += row[j].padEnd(biggestRowLengths[j], ' ')
    }
    out += `|`
  }

  return out
}

function generateMetadataTable(res: OpCodeGenResult): Table {
  return {
    headers: ["Metadata Property", "Value"],
    rows: [
        ["Size of an instruction (in bytes)", `${INSTRUCTION_LENGTH}`],
        ["Bytes used for an opcode", `${res.opcodeSize}`],
        ["Arguments length (in bytes)", `${INSTRUCTION_LENGTH - res.opcodeSize}`],
        ["OP Code count", `${res.codes.length}`]
    ]
  }
}

function joinParams(p: InstructionParam[]): string {
  return p.map(v => `${v.name}: ${v.typename}`).join(", ")
}

function code(str: string): string {
  return `<code>${str}</code>`
}

function generateOpCodeTable(codes: Instruction[]): Table {
  let rows: string[][] = []

  for (const opcode of codes) {
    rows.push([code(opcode.opcode), code(opcode.value), `${opcode.padding}`, code(joinParams(opcode.params))])
  }

  return {
    caption: "OP Codes",
    headers: ["OP Code", "Value", "Padding", "Arguments"],
    rows
  }
}

export async function generateDocs(res: OpCodeGenResult): Promise<void> {
  const metaTable = generateMetadataTable(res)
  const opcodeTable = generateOpCodeTable(res.codes)

  const mdStr = `# OP Code Specification Tables\n\n${printTableToMarkdown(metaTable)}\n\n${printTableToMarkdown(opcodeTable)}`
  const htmlStr = `<h2 id="opcodes-spec">OP Code Table</h2>\n\n${printTableToHtml(metaTable)}\n\n${printTableToHtml(opcodeTable)}`

  await writeToFile(mdStr, "../opcode-spec.md")
  await writeToFile(htmlStr, "./opcode-table.html")
}