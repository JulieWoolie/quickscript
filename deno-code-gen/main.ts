import {opcodeMain} from "./opcodes/opcodes-main";
import {generateStdLibDeclFile} from "./stdlib/stdlib-gen";
import {generateUnicodeCode} from "./unicode/unicode-gen";

async function main() {
  await opcodeMain()
  await generateStdLibDeclFile()
  await generateUnicodeCode()
}

// @ts-ignore
await main()