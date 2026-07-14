#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "common.h"
#include "errors.h"
#include "lexer.h"
#include "syntaxtree.h"
#include "parser.h"

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
  NodePool pool = NodePool();

  Parser p = Parser(&tokens, &pool, &errors, &table);

  NodeRef<Identifier> idref = p.id();
  Identifier* ptr = pool.get<Identifier>(idref);

  char idval[512];
  table.getchars(ptr->value, idval, 512);

  printf("id: loc=%i valId=%i val='%s'\n", ptr->location.index, ptr->value, idval);

  return EXIT_SUCCESS;
}

