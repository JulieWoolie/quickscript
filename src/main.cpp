#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "allocator.h"
#include "common.h"
#include "analysis/TypeResolver.h"
#include "interpreter/nativeinterface.h"
#include "parse/errors.h"
#include "parse/lexer.h"
#include "parse/syntaxtree.h"
#include "parse/parser.h"
#include "parse/print-visitor.h"

void native_printfn(NativeCall* call) {
  // nop;
}

Bindings createStandardBindings(TypeLookup* lookup) {
  Bindings bindings;

  FunctionSignature sign;
  sign.paramCount = 1;
  sign.returnType = lookup->getVoidType();

  FunctionSignatureParam param;
  param.type = lookup->getStringType();
  param.varargs = false;
  sign.params = &param;

  FunctionSignature* emplaced = lookup->emplaceFunctionType(&sign);

  bindings.putFunction("print", emplaced, native_printfn);

  return bindings;
}

int32 main(int32 argc, cstring argv[]) {
  if (argc < 2) {
    printf("Enter file name to execute!\n");
    return EXIT_FAILURE;
  }

  cstring fname = argv[argc - 1];
  std::ifstream file(fname);

  if (!file) {
    printf("File '%s' doesn't exist or can't be read.\n", fname);
    return EXIT_FAILURE;
  }

  std::string file_contents { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };

  TokenList tokens = TokenList();
  StringTable table = StringTable();

  Lexer l = Lexer(file_contents, &tokens, &table);
  l.lex();

  CompilerErrors errors = CompilerErrors(&file_contents, fname);
  NoFreeAllocator pool = NoFreeAllocator();

  Parser p = Parser(&tokens, &pool, &errors, &table);

  ScriptFileStatement* sfs = p.parse();

  TypeLookup lookup = TypeLookup(&pool);
  Bindings bindings = createStandardBindings(&lookup);

  TypeResolver resolver = TypeResolver(&lookup, &table, &errors, &bindings);

  resolver.acceptScriptFileStatement(sfs);

  PrintingVisitor pv = PrintingVisitor(&table, fname);
  sfs->acceptVisit(&pv);

  return EXIT_SUCCESS;
}

