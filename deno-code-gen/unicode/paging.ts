import {
  BinaryPropertyPages, CodepointContext,
  CodepointProperty, PAGE_MASK, PAGE_SHIFT, PAGE_SIZE, PAGECOUNT
} from "./unicode";

function buffersEqual(b1: Uint8Array, b2: Uint8Array, pageSize: number): boolean {
  for (let i = 0; i < pageSize; i++) {
    if (b1[i] != b2[i]) {
      return false
    }
  }
  return true
}

function fillUniquePages(p: BinaryPropertyPages, nonUniquePages: Uint8Array[]) {
  const pageSizeBytes = p.pageSizeBytes

  outer: for (let nu = 0; nu < nonUniquePages.length; nu++) {
    const nonUnique = nonUniquePages[nu]

    for (let ui = 0; ui < p.uniquePages.length; ui++) {
      const unique = p.uniquePages[ui]
      if (unique == undefined) {
        continue
      }

      if (!buffersEqual(nonUnique, unique, pageSizeBytes)) {
        continue
      }

      p.indexMap[nu] = ui
      continue outer
    }

    const ui = p.uniquePages.length
    p.uniquePages.push(nonUnique)
    p.indexMap[nu] = ui
  }
}

export function paginateBinaryProperties(ctx: CodepointContext): BinaryPropertyPages {
  const paged: CodepointProperty[] = ctx.pagedBinary

  const binaryCount = paged.length
  const bytes = Math.ceil(binaryCount / 8)
  const pageSize = bytes * PAGE_SIZE

  const result: BinaryPropertyPages = {
    codepointBytes: bytes,
    pageSizeBytes: pageSize,
    indexMap: Array(PAGECOUNT),
    uniquePages: []
  }

  const nonUniquePages: Uint8Array[] = Array(PAGECOUNT)
  for (let i = 0; i < PAGECOUNT; i++) {
    nonUniquePages[i] = new Uint8Array(pageSize)
  }

  for (const propName in ctx.binaryValues) {
    const codepoints = ctx.binaryValues[propName]!
    const def = ctx.allProperties.lookup[propName]

    if (def == undefined) {
      throw `property definition for ${propName} not found`
    }

    for (const ch of codepoints) {
      const pageIndex = ch >> PAGE_SHIFT
      const charIndex = ch & PAGE_MASK
      const page = nonUniquePages[pageIndex]

      const propByteIndex = Math.floor(def.typeLocalIndex / 8)
      const propBitIndex = def.typeLocalIndex % 8

      const index = (charIndex * bytes) + propByteIndex

      page[index] |= (1 << propBitIndex)
    }
  }

  fillUniquePages(result, nonUniquePages)

  const nonUniqueCount = nonUniquePages.length
  const uniqueCount = result.uniquePages.length
  const ratio = 100.0 - ((uniqueCount / nonUniqueCount) * 100.0)
  const totalMemSize = uniqueCount * pageSize
  const memSizeKb = totalMemSize / 1024

  console.log("Post unicode page processing info:")
  console.log(`  Non unique pages: ${nonUniqueCount}`)
  console.log(`  Unique pages: ${uniqueCount}`)
  console.log(`  Drop Rate: ${ratio.toFixed(2)}%`)
  console.log(`  Required bytes to represent unique pages: ${totalMemSize} (Or ${memSizeKb} KiB)`)

  return result
}