#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "common.h"
#include "errors.h"
#include "lexer.h"

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

  Lexer l(file_contents);
  CompilerErrors errors(&file_contents, fname);

  Token* first = l.nextToken();

  errors.error(first->start, "Something happened here! int=%i ttype=%s", 34, tokentype_name(first->ttype));

  return EXIT_SUCCESS;
}

