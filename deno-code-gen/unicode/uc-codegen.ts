import {
  BinaryPropertyPages,
  CodepointContext,
  CodepointProperty,
  PAGE_MASK,
  PAGE_SHIFT
} from "./unicode";
import {FILE_HEADER, getFittingType, writeToFile} from "../common";

function getFuncName(cn: string): string {
  const l = cn.toLowerCase()

  if (l.startsWith("changes")) {
    return cn
  }
  if (l.startsWith("expands")) {
    return cn
  }

  return `is${cn.substring(0, 1).toUpperCase()}${cn.substring(1)}`
}

export async function generateBinaryPropertyLookups(ctx: CodepointContext, pages: BinaryPropertyPages) {
  let out = `#ifndef UNICODE_BINARY_PROPS_H
#define UNICODE_BINARY_PROPS_H

#include "../common.h"
#include "utf8.h"

${FILE_HEADER}

typedef const uint8* UnicodeBitSet;

`

  const binDefs: CodepointProperty[] = [...ctx.pagedBinary, ...ctx.nonPagedBinary]
  const funcNames: {[name: string]: string} = {}

  for (const def of binDefs) {
    const tn = def.codeName
    funcNames[def.name] = `bool char_${getFuncName(tn)}`
  }

  for (const bd of binDefs) {
    out += `#define BPROP_${bd.name} ${bd.typeLocalIndex}\n`
  }

  const baseType = getFittingType(binDefs.length)!
  out += `typedef ${baseType.type} binaryprop;\n\n`

  out += `UnicodeBitSet getProperties(utf32char ch);\n\n`

  for (const bd of binDefs) {
    out += `${funcNames[bd.name]}(utf32char ch);\n\n`
  }

  out += `#endif // UNICODE_BINARY_PROPS_H`

  await writeToFile(out, "../src/strings/unicode_binary_props.h");

  out = `#include "unicode_bianry_props.h"

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

  out += `\n};

UnicodeBitSet getProperties(const utf32char ch) {
  uint32 pageIndex = ch >> ${PAGE_SHIFT};
  uint32 dataOffset = INDEX_LOOKUP[pageIndex];
  return &PAGES[dataOffset + ${pageMaskString}];
}
`

  for (const bd of ctx.nonPagedBinary) {
    out += `\n${funcNames[bd.name]}(const utf32char ch) {
  switch (ch) {`

    const values = ctx.binaryValues[bd.name]!
    for (const v of values) {
      out += `\n    case 0x${v.toString(16)}:`
    }

    out += `\n      return true;\n    default:\n      return false;\n  }\n  return false;\n}`
  }

  for (const bd of ctx.pagedBinary) {
    out += `\n${funcNames[bd.name]}(const utf32char ch) {`

    if (bd.codepointCount == 0) {
      out += `\n  return false;\n}\n`
      continue
    }

    const byteIndex = Math.floor(bd.typeLocalIndex / 8)
    const bitIndex = Math.floor(bd.typeLocalIndex % 8)
    const mask = 1 << bitIndex

    let byteIndexStr = byteIndex == 0 ? "" : ` + ${byteIndex}`

    out += `\n  return PAGES[PAGE_STARTS[ch >> ${PAGE_SHIFT}] + ${pageMaskString}${byteIndexStr}] & 0x${mask.toString(16)};`
    out += `\n}\n`
  }

  await writeToFile(out, "../src/strings/unicode_binary_props.cc");
}