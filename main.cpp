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

  TokenList tokens = TokenList();
  StringTable table = StringTable();

  Lexer l = Lexer(file_contents, &tokens, &table);
  CompilerErrors errors = CompilerErrors(&file_contents, fname);

  while (true) {
    Token* tok = l.nextToken();
    if (tok->ttype == TT_EOF) {
      break;
    }

    errors.info(tok->start, "Token = %s value=%i", tokentype_name(tok->ttype), tok->valueId);

    if (tok->valueId == EMPTY_STRING) {
      continue;
    }

    uint32 len = table.getlen(tok->valueId);
    char content[len + 1];

    table.getchars(tok->valueId, content, len);
    content[len] = '\0';

    printf("token content: '%s'\n", content);
  }

  return EXIT_SUCCESS;
}

