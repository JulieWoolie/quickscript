import {
  BinaryPropertyValueMap, CodepointProperties,
  CodepointProperty, CodepointPropertyLookup,
  fetchUnicodeFile, PropertyType,
  splitValues
} from "./unicode";

async function loadPropsFrom(fileName: string, map: BinaryPropertyValueMap, lookup: CodepointPropertyLookup) {
  const lines = await fetchUnicodeFile(fileName)

  for (const line of lines) {
    const lineValues = splitValues(line)

    if (lineValues.length != 2) {
      continue
    }

    const codepoints = lineValues[0]
    const property = lineValues[1]

    const definedProp = lookup[property]
    if (definedProp == undefined || definedProp.type != "binary") {
      continue
    }

    let values = map[property]

    if (values == undefined) {
      values = new Set<number>()
      map[property] = values
    }

    if (codepoints.includes("..")) {
      const split = codepoints.split("..")
      const min = parseInt(split[0], 16)
      const max = parseInt(split[1], 16)

      for (let i = min; i <= max; i++) {
        values.add(i)
      }

      continue
    }

    values.add(parseInt(codepoints, 16))
  }
}

export async function loadProps(map: BinaryPropertyValueMap, lookup: CodepointPropertyLookup) {
  await loadPropsFrom("PropList.txt", map, lookup)
  await loadPropsFrom("DerivedCoreProperties.txt", map, lookup)
}

function toCodeName(name: string) {
  if (!name.includes("_")) {
    return `${name.substring(0, 1).toLowerCase()}${name.substring(1)}`
  }

  return name
      .split("_")
      .map((v, i) => {
        if (i == 0) {
          return v.toLowerCase()
        }
        return v.substring(0, 1).toUpperCase() + v.substring(1).toLowerCase()
      })
      .join("")
}

type PropertyIndexMapper = {
  [key in PropertyType]: number;
};

export async function generateCharacterPropertiesDefinitions(): Promise<CodepointProperties> {
  const lines = await fetchUnicodeFile("PropertyAliases.txt", {
    skipComments: false,
    skipHeader: true,
    skipSeparators: true,
    stripSuffixComments: true
  })

  let category: PropertyType = "string"

  const indexMap: PropertyIndexMapper = {
    binary: 0,
    string: 0,
    numeric: 0,
    catalog: 0,
    enumerated: 0,
    miscellaneous: 0
  }

  const result: CodepointProperties = {
    lookup: {},
    properties: []
  }

  const catLinePrefix = "# "
  const catLineSuffix = " Properties"

  for (const line of lines) {
    if (line.startsWith(catLinePrefix)) {
      if (line.endsWith(catLineSuffix)) {
        const start = catLinePrefix.length
        const end = line.length - catLineSuffix.length
        category = line.substring(start, end).toLowerCase() as PropertyType
      }
      continue
    }

    const values = splitValues(line);
    const longname = values[1]
    const alias = values[0]

    const definition: CodepointProperty = {
      alias,
      name: longname,
      codeName: toCodeName(longname),
      typeLocalIndex: indexMap[category]++,
      type: category,
      codepointCount: 0
    }

    result.properties.push(definition);
    result.lookup[longname] = definition
  }

  return result
}