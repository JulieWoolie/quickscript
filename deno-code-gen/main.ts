import {opcodeMain} from "./opcodes/opcodes-main";
import {generateStdLib} from "./stdlib/stdlib-gen";
import {generateUnicodeCode} from "./unicode/unicode-gen";

async function main() {
  await opcodeMain()
  // await generateStdLib()
  await generateUnicodeCode()
}

// @ts-ignore
await main()