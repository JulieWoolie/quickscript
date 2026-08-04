import {Instruction, INSTRUCTION_LENGTH, OpCodeGenResult, writeToFile} from "./common";

export async function generateMarkdownSpec(res: OpCodeGenResult): Promise<void> {
  let out = `# OP Code specification Table
| Metadata Property | Value |
|--|--|
|Size of an instruction (in bytes)|${INSTRUCTION_LENGTH}|
|Bytes used for an opcode|${res.opcodeSize}|
|Arguments length (bytes)|${INSTRUCTION_LENGTH - res.opcodeSize}|

| OP Code | Value | Padding | Arguments |
|--|--|--|--|`

  for (const code of res.codes) {
    out += `\n|\`${code.opcode}\`|\`${code.value}\`|${code.padding}|`
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

export async function printHtmlSpec(res: OpCodeGenResult): Promise<void> {
  let out = `

<table>
  <caption>Metadata Parameters</caption>
  <thead>
  <tr>
    <th>Property</th>
    <th>Value</th>
  </tr>
  </thead>
  <tbody>
  <tr>
    <th>Size of an instruction (in bytes)</th>
    <th>${INSTRUCTION_LENGTH}</th>
  </tr>
  <tr>
    <th>Bytes used for an opcode</th>
    <th>${res.opcodeSize}</th>
  </tr>
  <tr>
    <th>Arguments length (bytes)</th>
    <th>${INSTRUCTION_LENGTH - res.opcodeSize}</th>
  </tr>
  </tbody>
</table>

<table>
  <caption>OP Codes</caption>
  <thead><tr>
    <th>OP Code</th>
    <th>Value</th>
    <th>Padding</th>
    <th>Arguments</th>
  </tr></thead>
  <tbody>`

  for (const code of res.codes) {
    let paramStr = ""
    code.params.forEach((v, i) => {
      if (i != 0) {
        paramStr += ", "
      }
      paramStr += `${v.name}: ${v.typename}`
    })

    out += `
    <tr>
      <td><code>${code.opcode}</code></td>
      <td><code>${code.value}</code></td>
      <td>${code.padding}</td>
      <td><code>${paramStr}</code></td>
    </tr>`
  }

  out += `\n  </tbody>\n</table>`

  await writeToFile(out, "./opcode-table.html")
}