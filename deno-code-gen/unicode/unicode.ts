import {fetchCached} from "../cached-requests";

export const MIN_COUNT_FOR_PAGED_DATA = 50
export const MAX_CODEPOINT = 0x10ffff

export const PAGE_SHIFT = 8
export const PAGE_MASK = 0xff
export const PAGE_SIZE = PAGE_MASK + 1

export const PAGECOUNT = Math.ceil(MAX_CODEPOINT / PAGE_SIZE)

export interface FetchParams {
  skipComments: boolean
  skipSeparators: boolean
  skipHeader: boolean,
  stripSuffixComments: boolean
}

export type PropertyType = "numeric" | "string" | "miscellaneous" | "catalog" | "enumerated" | "binary"

export interface CodepointProperty {
  name: string
  alias: string
  codeName: string
  type: PropertyType
  typeLocalIndex: number
  codepointCount: number
}

export type CodepointPropertyLookup = {[name: string]: CodepointProperty | undefined}

export interface CodepointProperties {
  properties: CodepointProperty[]
  lookup: CodepointPropertyLookup
}

export type BinaryPropertyValueMap = {[property: string]: Set<number> | undefined}

export interface CodepointContext {
  allProperties: CodepointProperties
  pagedBinary: CodepointProperty[]
  nonPagedBinary: CodepointProperty[]
  binaryValues: BinaryPropertyValueMap
}

export interface BinaryPropertyPages {
  codepointBytes: number
  pageSizeBytes: number
  uniquePages: Uint8Array[]
  indexMap: number[]
}

export function splitValues(line: string): string[] {
  return line.split(";").map(v => v.trim())
}

export async function fetchUnicodeFile(fname: string, opts?: Partial<FetchParams>): Promise<string[]> {
  const strContent = await fetchCached(`https://www.unicode.org/Public/UCD/latest/ucd/${fname}`, fname)

  const stripComments = opts?.skipComments ?? true
  const skipHeader = opts?.skipHeader ?? true
  const skipSeparators = opts?.skipSeparators ?? true
  const stripSuffixComments = opts?.stripSuffixComments ?? stripComments

  let lines = strContent.split("\n")

  if (skipHeader) {
    for (let i = 0; i < lines.length; i++) {
      if (lines[i].startsWith("#")) {
        continue
      }

      lines = lines.slice(i, lines.length)
      break
    }
  }

  lines = lines.filter(v => {
    const t = v.trim()
    if (t.length == 0) {
      return false
    }

    if (stripComments && t.startsWith("#")) {
      return false
    }
    if (skipSeparators && t.startsWith("#") && t.endsWith("===")) {
      return false
    }

    return true
  })

  if (stripSuffixComments) {
    lines = lines.map(v => {
      if (v.startsWith("#")) {
        return v
      }
      if (v.includes("#")) {
        return v.substring(0, v.indexOf("#"))
      }
      return v
    })
  }

  return lines.map(v => v.trim())
}