#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "allocator.h"
#include "common.h"
#include "analysis/analyzer.h"
#include "interpreter/nativeinterface.h"
#include "errors.h"
#include "tester.h"
#include "analysis/transformer.h"
#include "codegen/compiler.h"
#include "interpreter/ir_file.h"
#include "parse/lexer.h"
#include "parse/syntaxtree.h"
#include "parse/parser.h"
#include "parse/print-visitor.h"

Bindings createStandardBindings() {
  Bindings bindings;
  return bindings;
}

int32 main(int32 argc, cstring argv[]) {
  ProgramSettings settings;
  ParseResult res = parseSettings(settings, argc, argv);

  if (res == RES_FAILED || settings.command == CMD_HELP) {
    showHelpMessage();
    return EXIT_SUCCESS;
  }

  if (settings.command == CMD_TESTS) {
    runTests(settings);
    return EXIT_SUCCESS;
  }

  std::string fname = std::string(settings.inputFile);
  std::ifstream file(fname);

  if (!file.is_open()) {
    printf("File '%s' doesn't exist or can't be read.\n", fname.c_str());
    return EXIT_FAILURE;
  }

  std::string file_contents { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };

  TokenList tokens = TokenList();
  StringTable table = StringTable();

  CompilerErrors errors = CompilerErrors(&file_contents, fname.c_str());

  Lexer l = Lexer(file_contents, &tokens, &table, &errors);
  l.lex();

  NoFreeAllocator pool = NoFreeAllocator();

  Parser p = Parser(&tokens, &pool, &errors, &table);

  ScriptFileStatement* sfs = p.parse();

  if (settings.printAst & PRINTAST_AFTER_PARSE) {
    printf("\n\n ====== AST After parser ======\n\n");
    PrintingVisitor pv = PrintingVisitor(&table, fname.c_str());
    pv.acceptScriptFileStatement(sfs);
  }

  TypeTable lookup = TypeTable();
  Bindings bindings = createStandardBindings();

  SemanticContext ctx = SemanticContext(lookup, table, errors, bindings, pool);
  runSemanticAnalysis(sfs, ctx);

  if (settings.printAst & PRINTAST_AFTER_ANALYSIS) {
    printf("\n\n ====== AST After semantic analyser ======\n\n");
    PrintingVisitor pv = PrintingVisitor(&table, fname.c_str());
    pv.acceptScriptFileStatement(sfs);
  }

  if (errors.getErrorCount() != 0) {
    return EXIT_FAILURE;
  }

  runSemanticTransformer(ctx, sfs);

  if (settings.printAst & PRINTAST_AFTER_TRANSFORM) {
    printf("\n\n ====== AST After semantic transformer ======\n\n");
    PrintingVisitor pv = PrintingVisitor(&table, fname.c_str());
    pv.acceptScriptFileStatement(sfs);
  }

  if (settings.command == CMD_COMPILE) {
    BytecodeFile bfile = compile(ctx);
    uint64 byteArraySize = 0;
    uint8* savedData = serializeBytecodeFile(bfile, &byteArraySize);

    std::string outFile = std::string(settings.outputFile);
    FILE* openFile = fopen(outFile.c_str(), "wb");
    fwrite(savedData, 1, byteArraySize, openFile);
    fclose(openFile);

    printf("Saved compiled output to '%s'\n", outFile.c_str());
  }

  return EXIT_SUCCESS;
}

