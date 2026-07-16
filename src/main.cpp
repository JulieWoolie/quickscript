#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "allocator.h"
#include "common.h"
#include "parse/errors.h"
#include "parse/lexer.h"
#include "parse/syntaxtree.h"
#include "parse/parser.h"
#include "parse/print-visitor.h"

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
  PrintingVisitor pv = PrintingVisitor(&table, fname);

  sfs->acceptVisit(&pv);

  return EXIT_SUCCESS;
}

