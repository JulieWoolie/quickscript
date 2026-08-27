import {generateCharacterPropertiesDefinitions, loadProps} from "./unicode-properties";
import {paginateBinaryProperties} from "./paging";
import {generateBinaryPropertyLookups} from "./uc-codegen";
import {
  BinaryPropertyValueMap,
  CodepointContext,
  CodepointProperties,
  CodepointProperty, MIN_COUNT_FOR_PAGED_DATA
} from "./unicode";

function createContext(props: CodepointProperties, values: BinaryPropertyValueMap): CodepointContext {
  const pagedBinary: CodepointProperty[] = []
  const nonPagedBinary: CodepointProperty[] = []

  for (const p of props.properties) {
    if (p.type != "binary") {
      continue
    }

    const codepoints = values[p.name]?.size ?? 0
    p.codepointCount = codepoints

    if (codepoints == 0) {
      continue
    }

    if (codepoints < MIN_COUNT_FOR_PAGED_DATA) {
      nonPagedBinary.push(p)
      continue
    }

    pagedBinary.push(p)
  }

  pagedBinary.forEach((v, i) => v.typeLocalIndex = i)

  return {
    binaryValues: values,
    pagedBinary,
    nonPagedBinary,
    allProperties: props
  }
}

export async function generateUnicodeCode(): Promise<void> {
  const properties = await generateCharacterPropertiesDefinitions()
  const propMap: BinaryPropertyValueMap = {}

  await loadProps(propMap, properties.lookup)

  const ctx = createContext(properties, propMap)
  const pages = paginateBinaryProperties(ctx)

  console.table(properties.properties)

  await generateBinaryPropertyLookups(ctx, pages)
}