#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "common.h"
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

  return EXIT_SUCCESS;
}

