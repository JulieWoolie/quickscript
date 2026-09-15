#include "qs/bytecode/bytecode_symbol.hpp"

BytecodeSymbol::BytecodeSymbol(bfsymtype symtype): type(symtype) {

}

BytecodeFuncSymbol::BytecodeFuncSymbol(): BytecodeSymbol(BFSYM_FUNC) {

}

BytecodeFuncSymbol* BytecodeFuncSymbol::create() {
  return new BytecodeFuncSymbol();
}

void BytecodeFuncSymbol::destroy(const BytecodeFuncSymbol* sym) {
  delete sym;
}

BytecodeTypeSymbol::BytecodeTypeSymbol(): BytecodeSymbol(BFSYM_STRUCT) {

}

BytecodeTypeSymbol* BytecodeTypeSymbol::create() {
  return new BytecodeTypeSymbol();
}

void BytecodeTypeSymbol::destroy(const BytecodeTypeSymbol* sym) {
  delete sym;
}

BytecodeVarSymbol::BytecodeVarSymbol(): BytecodeSymbol(BFSYM_VARIABLE) {

}

BytecodeVarSymbol* BytecodeVarSymbol::create() {
  return new BytecodeVarSymbol();
}

void BytecodeVarSymbol::destroy(const BytecodeVarSymbol* sym) {
  delete sym;
}
