declare const Deno: any

interface UnicodePropertyDefs {
  binaryProps: string[]
}

interface FetchParams {
  skipComments: boolean
  skipSeparators: boolean
  skipHeader: boolean
}

async function fetchUnicodeFile(fname: string, opts?: FetchParams): Promise<string[]> {
  const cachePath = `.cache/${fname}`
  let strContent = ""

  try {
    strContent = await Deno.readTextFile(cachePath)
    console.log(`Found cached data file ${cachePath}`)
  } catch (err) {
    if (!(err instanceof Deno.errors.NotFound)) {
      throw err
    }

    const fUrl = `https://www.unicode.org/Public/UCD/latest/ucd/${fname}`
    console.log(`Fetching requested data from ${fUrl}`)

    const res = await fetch(fUrl)
    if (!res.ok) {
      throw `Failed to fetch file ${fname} (status: ${res.status})`
    }

    try {
      await Deno.mkdir(`.cache/`)
    } catch (ignored) {}

    const file = await Deno.open(cachePath, {
      write: true,
      create: true,
      truncate: true
    })

    await res.body!!.pipeTo(file.writable)

    strContent = await Deno.readTextFile(cachePath)
  }

  const stripComments = opts?.skipComments ?? true
  const skipHeader = opts?.skipHeader ?? true
  const skipSeparators = opts?.skipSeparators ?? true

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

  if (stripComments) {
    lines = lines.map(v => {
      if (!v.includes("#")) {
        return v
      }
      return v.substring(0, v.indexOf('#'))
    })
  }

  return lines.map(v => v.trim())
}

function separateValues(line: string): string[] {
  return line.split(";").map(v => v.trim())
}

async function generateProps(defs: UnicodePropertyDefs) {
  const lines = await fetchUnicodeFile("PropertyAliases.txt", {
    skipSeparators: true,
    skipHeader: true,
    skipComments: false
  })

  let start: number | undefined

  for (let i = 0; i < lines.length; i++) {
    let line = lines[i]

    if (line == "# Binary Properties") {
      start = i + 1
      break
    }
  }

  if (start == undefined) {
    throw "idk, no start"
  }

  for (let i = start; i < lines.length; i++) {
    const line = lines[i]
    if (line.startsWith("#")) {
      break
    }

    const values = separateValues(lines[i])
    defs.binaryProps.push(values[1])
  }
}

async function generateHeaderFile(defs: UnicodePropertyDefs) {
  let out = `#ifndef UNICODE_PROPERTIES_H_
#define UNICODE_PROPERTIES_H_

#include "../common.h"
`

  out += `\n#define BIN_PROP_INVALID 0`
  defs.binaryProps.forEach((v, i) => {
    out += `\n#define BIN_PROP_${v} static_cast<binprop>(${i + 1})`
  })

  out += `\ntypedef uint8 binprop;
typedef uint32* BinaryProps;

bool hasBinaryProp(u32char ch, binprop prop);

BinaryProps getCharacterProperties(u32char ch);

#endif`

  await Deno.writeTextFile("../src/strings/unicode_properties.h", out)
}

async function main() {
  const props: UnicodePropertyDefs = {
    binaryProps: []
  }

  await generateProps(props)
  await generateHeaderFile(props)
}

// @ts-ignore
await main()