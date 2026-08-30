import {FILE_HEADER, Instruction, InstructionParam, OpCodeGenResult, writeToFile} from "../common";

function getSignatureString(params: InstructionParam[]): string {
  let sig = ""
  params.forEach((v, i) => {
    if (i != 0) {
      sig += ","
    }
    sig += `${v.name}:${v.typename}`
  })
  return sig
}

function generateRegistryStringNameFunc(): string {
  let out = "\n  switch(r) {"
  for (let i = 0; i < 64; i++) {
    let str = i.toString()
    let regName: string

    if (i == 0) {
      regName = `rvr`
    } else if (i == 1) {
      regName = "icr"
    } else {
      regName = `r${str.padStart(2, '0')}`
    }

    out += `\n    case ${i}: return "${regName}";`
  }
  out += `\n    default: return "INVALID_REGISTRY";\n  }\n`
  return out
}

function getLongestName(codes: Instruction[]) {
  let longestName = 0
  for (const code of codes) {
    longestName = Math.max(code.opcode.length, longestName)
  }
  return longestName
}

export async function generatePrinterFunction(res: OpCodeGenResult) {
  let out = `#ifndef OPCODE_PRINTER_H
#define OPCODE_PRINTER_H

#include <cstdio>

#include "opcodes.h"
#include "../types/types.h"

${FILE_HEADER}

void printTypeIndex(FILE* out, typeindex idx);

void printInstructionToString(uint8* buf, FILE* out, uint8* strPool);

#endif // OPCODE_PRINTER_H`

  await writeToFile(out, "../src/interpreter/opcode_printer.h")

  out = `#include "opcode_printer.h"

${FILE_HEADER}

static conststring getRegistryName(const uint8 r) {${generateRegistryStringNameFunc()}}

void printTypeIndex(FILE* out, const typeindex idx) {
  if (idx < LAST_RESERVED_TYPE_INDEX) {
    fprintf(out, "%s", nativeTypeIndexName(idx));
    return;
  }
  fprintf(out, "%llu", idx);
}

void printInstructionToString(uint8* buf, FILE* out, uint8* strPool) {
  const opcode code = *reinterpret_cast<opcode*>(buf);
  fprintf(out, "%-${getLongestName(res.codes)}s", opcode_name(code));
  
  switch (code) {`

  const signatureGroups: {[sig: string]: Instruction[]} = {}
  for (const code of res.codes) {
    const sig = getSignatureString(code.params)
    let group = signatureGroups[sig]

    if (group == null) {
      group = []
      signatureGroups[sig] = group
    }

    group.push(code)
  }

  for (const signature in signatureGroups) {
    const codes = signatureGroups[signature]
    for (const code of codes) {
      out += `\n    case OP_${code.opcode}:`
    }

    const params = codes[0].params
    let off = res.opcodeSize

    for (const p of params) {
      let tn: string = p.typename
      let formatSpec: string = ""
      let valExpr: string = ""

      out += `\n      fprintf(out, " /*${p.name}:*/");`

      if (p.name == "typeindex") {
        out += `\n      fprintf(out, " TYPES[");`
        out += `\n      printTypeIndex(out, *reinterpret_cast<${tn}*>(buf + ${off}));`
        out += `\n      fprintf(out, "]");`
        off += p.size
        continue
      }

      switch (p.typename) {
        case "uint64":
          formatSpec = "%llu"
          break
        case "int64":
          formatSpec = "%lld"
          break
        case "register":
          tn = "uint8"
          formatSpec = "%s"
          valExpr = `getRegistryName(*(buf + ${off}))`
          break
        case "typeindex":
          tn = "uint32"
          formatSpec = "TYPE[%d]"
          break
        default:
          formatSpec = "%d"
          break
      }

      if (p.name == "typeindex") {
        formatSpec = "TYPE[%d]"
      }

      if (valExpr.length == 0) {
        if (tn == "uint8") {
          valExpr = `*(buf + ${off})`
        } else {
          valExpr = `*reinterpret_cast<${tn}*>(buf + ${off})`
        }
      }

      if (p.name == "straddr" || p.name == "funcName") {
        formatSpec = "STRINGS[off=%llu] /* %.*s */"
        valExpr += `, *reinterpret_cast<uint32*>(strPool + ${valExpr}), reinterpret_cast<char*>(strPool + sizeof(uint32) + ${valExpr})`
      }

      out += `\n      fprintf(out, " ${formatSpec}", ${valExpr});`
      off += p.size
    }

    out += `\n      break;`
  }

  out += `\n    default:\n      break;\n  }\n}`

  await writeToFile(out, "../src/interpreter/opcode_printer.cc")
}