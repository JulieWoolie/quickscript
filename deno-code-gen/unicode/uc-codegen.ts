import {
  BinaryPropertyPages,
  CodepointContext,
  CodepointProperty,
  PAGE_MASK,
  PAGE_SHIFT
} from "./unicode";
import {FILE_HEADER, getFittingType, writeToFile} from "../common";

function titlecase(str: string): string {
  return `${str.substring(0, 1).toUpperCase()}${str.substring(1)}`
}

function getFuncName(cn: string): string {
  const l = cn.toLowerCase()

  if (l.startsWith("changes")) {
    return titlecase(cn)
  }
  if (l.startsWith("expands")) {
    return titlecase(cn)
  }

  return `Is${titlecase(cn)}`
}

interface CodepointRange {
  min: number,
  max: number
}

interface OptimizedNonPagedValues {
  codepoints: number[]
  ranges: CodepointRange[]
}

function isNext(a: number, b: number): boolean {
  return b == (a + 1)
}

function optimizeNonPagedProperty(set: Set<number>): OptimizedNonPagedValues {
  let result: OptimizedNonPagedValues = {
    codepoints: [],
    ranges: []
  }

  const nums = [...set]
  nums.sort((a, b) => a - b)

  let i = 0
  while (i < nums.length) {
    const c = nums[i++]

    if (i < nums.length && isNext(c, nums[i])) {
      let range: CodepointRange = {
        min: c,
        max: c
      }

      const start = i

      while (i < nums.length && isNext(range.max, nums[i])) {
        range.max = nums[i++]
      }

      if (range.max - range.min >= 3) {
        result.ranges.push(range)
        continue
      }

      i = start
    }

    result.codepoints.push(c)
  }

  return result
}

function char(num: number): string {
  if (num >= 0x20 && num <= 0x7f) {
    let str = String.fromCodePoint(num)
    if (str == "'") {
      str = `\\'`
    }
    return `'${str}'`
  }
  return `0x${num.toString(16)}`
}

type StringMap = {[name: string]: string}

async function generateBinaryPropHeader(binDefs: CodepointProperty[], funcNames: StringMap) {
  let out = `#ifndef UNICODE_BINARY_PROPS_H
#define UNICODE_BINARY_PROPS_H

#include "../common.h"
#include "utf8.h"

${FILE_HEADER}

typedef const uint8* UnicodeBitSet;

`

  for (const bd of binDefs) {
    out += `#define BPROP_${bd.name} ${bd.typeLocalIndex}\n`
  }

  const baseType = getFittingType(binDefs.length)!
  out += `typedef ${baseType.type} binaryprop;\n\n`

  for (const bd of binDefs) {
    out += `${funcNames[bd.name]}(utf32char ch);\n\n`
  }

  out += `#endif // UNICODE_BINARY_PROPS_H`

  await writeToFile(out, "../src/strings/unicode_binary_props.h");
}

function generateNonPagedPropMethods(funcNames: StringMap, ctx: CodepointContext): string {
  let out = ""
  for (const bd of ctx.nonPagedBinary) {
    out += `\n${funcNames[bd.name]}(const utf32char ch) {`

    const values = ctx.binaryValues[bd.name]!
    const opt = optimizeNonPagedProperty(values)

    if (opt.codepoints.length == 0) {
      opt.ranges.forEach((r, i) => {
        out += `\n  `

        if (i == 0) {
          out += `return `
        } else {
          out += `    || `
        }

        out += `(ch >= ${char(r.min)} && ch <= ${char(r.max)})`
      })

      out += `;\n}`

      continue
    }

    if (opt.ranges.length != 0) {
      opt.ranges.forEach((r, i) => {
        out += `\n  `
        if (i == 0) {
          out += `if `
          if (opt.ranges.length > 1) {
            out += "("
          }
        } else {
          out += ` || `
        }

        out += `(ch >= ${char(r.min)} && ch <= ${char(r.max)})`
      })

      if (opt.ranges.length > 1) {
        out += `\n  )`
      }

      out += ` {\n    return true;\n  }`
    }

    out += `\n  switch (ch) {`
    for (const v of opt.codepoints) {
      out += `\n    case ${char(v)}:`
    }

    out += `\n      return true;\n    default:\n      return false;\n  }\n  return false;\n}`
  }

  return out
}

async function generateBinaryPropertySourceFile(pages: BinaryPropertyPages, funcNames: StringMap, ctx: CodepointContext) {
  let out = `#include "unicode_binary_props.h"

${FILE_HEADER}

static const uint32 PAGE_STARTS[${pages.indexMap.length}] = {`

  for (let i = 0; i < pages.indexMap.length; i++) {
    if (i % 8 == 0) {
      out += `\n  `
    }

    const mappedIndex = pages.indexMap[i]
    const dataStart = mappedIndex * pages.pageSizeBytes

    const idxStr = `0x${dataStart.toString(16).padStart(8, '0')}`

    out += `${idxStr}, `
  }

  const pageCount = pages.uniquePages.length
  const pageSize = pages.pageSizeBytes

  out += `\n};

#define PAGES_COUNT ${pageCount}
#define PAGE_MEMSIZE ${pageSize}
#define BITSET_SIZE ${pages.codepointBytes}

static const uint8 PAGES[${pageCount * pageSize}] = {`

  for (let i = 0; i < pages.uniquePages.length; i++) {
    const buf = pages.uniquePages[i]

    out += `\n  // page ${i} offset: ${i * pageSize}`

    for (let j = 0; j < pages.pageSizeBytes; j++) {
      if (j % pages.codepointBytes == 0) {
        const idxStr = `${(j / pages.codepointBytes).toString().padStart(3, ' ')}`
        out += `\n  /* ${idxStr} */ `
      }

      const byte = buf[j]
      out += `0x${byte.toString(16).padStart(2, '0')},`
    }
  }

  const pageMaskString = `(ch & 0x${PAGE_MASK.toString(16)})`

  out += `\n};\n`
  out += generateNonPagedPropMethods(funcNames, ctx)

  for (const bd of ctx.pagedBinary) {
    out += `\n${funcNames[bd.name]}(const utf32char ch) {`

    const byteIndex = Math.floor(bd.typeLocalIndex / 8)
    const bitIndex = bd.typeLocalIndex % 8
    const mask = 1 << bitIndex

    out += `\n  return PAGES[((PAGE_STARTS[ch >> ${PAGE_SHIFT}]) + (${pageMaskString} * ${pages.codepointBytes})) + ${byteIndex}] & 0x${mask.toString(16)};`
    out += `\n}\n`
  }

  await writeToFile(out, "../src/strings/unicode_binary_props.cc");
}

export async function generateBinaryPropertyLookups(ctx: CodepointContext, pages: BinaryPropertyPages) {
  const binDefs: CodepointProperty[] = [...ctx.pagedBinary, ...ctx.nonPagedBinary]
  const funcNames: {[name: string]: string} = {}

  for (const def of binDefs) {
    const tn = def.codeName
    funcNames[def.name] = `bool uc${getFuncName(tn)}`
  }

  await generateBinaryPropHeader(binDefs, funcNames)
  await generateBinaryPropertySourceFile(pages, funcNames, ctx)
}