import {opcodeMain} from "./opcodes/opcodes-main";
import {generateStdLibDeclFile} from "./stdlib/stdlib-gen";

async function main() {
  await opcodeMain()
  await generateStdLibDeclFile()
}

// @ts-ignore
await main()