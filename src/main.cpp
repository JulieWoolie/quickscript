#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "allocator.h"
#include "common.h"
#include "analysis/analyzer.h"
#include "errors.h"
#include "tester.h"
#include "analysis/transformer.h"
#include "codegen/compiler.h"
#include "interpreter/interpreter.h"
#include "bytecode/bytecode_file.h"
#include "interpreter/script_error.h"
#include "parse/lexer.h"
#include "parse/syntaxtree.h"
#include "parse/parser.h"
#include "parse/print-visitor.h"
#include "stdlib/qs_stdlib.h"

static bool compileBytecode(const ProgramSettings& settings, BytecodeFile** out, const BindingsObject* bindings) {
  std::string fname = std::string(settings.inputFile);
  std::ifstream file(fname);

  if (!file.is_open()) {
    printf("File '%s' doesn't exist or can't be read.\n", fname.c_str());
    return false;
  }

  std::string file_contents { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };

  TokenList tokens = TokenList();
  StringTable table = StringTable();

  CompilerErrors errors = CompilerErrors(&file_contents, fname.c_str());
  errors.setLogLevel(settings.loggerLevel);

  Lexer l = Lexer(file_contents, &tokens, &table, &errors);
  l.next();
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

  SemanticContext ctx = SemanticContext(lookup, table, errors, pool, settings.compilationOptions, bindings);
  runSemanticAnalysis(sfs, ctx);

  if (settings.printAst & PRINTAST_AFTER_ANALYSIS) {
    printf("\n\n ====== AST After semantic analyser ======\n\n");
    PrintingVisitor pv = PrintingVisitor(&table, fname.c_str());
    pv.acceptScriptFileStatement(sfs);
  }

  if (errors.getErrorCount() != 0) {
    return false;
  }

  runSemanticTransformer(ctx, sfs);

  if (settings.printAst & PRINTAST_AFTER_TRANSFORM) {
    printf("\n\n ====== AST After semantic transformer ======\n\n");
    PrintingVisitor pv = PrintingVisitor(&table, fname.c_str());
    pv.acceptScriptFileStatement(sfs);
  }

  *out = &compile(ctx);
  return true;
}

static void compileSource(const BytecodeFile& bfile, const ProgramSettings& settings) {
  uint64 byteArraySize = 0;

  std::string outFile = std::string(settings.outputFile);
  FILE* openFile = fopen(outFile.c_str(), "wb");

  if (settings.compileToBinary) {
    uint8* savedData = serializeBytecodeFile(bfile, &byteArraySize);
    fwrite(savedData, 1, byteArraySize, openFile);
  } else {
    printBytecodeFile(bfile, openFile);
  }

  BytecodeFile::destroy(bfile);
  fclose(openFile);

  printf("Saved compiled output to '%s'\n", outFile.c_str());
}

static int32 runCompiledFile(const BytecodeFile& bfile, const ProgramSettings& settings, const BindingsObject* bindings) {
  VirtualMachine vm;
  vm.addBindings(bindings);

  std::string filename = std::string(settings.inputFile);
  const uint32 entryPoint = vm.addBytecodeFile(bfile, filename);

  BytecodeFile::destroy(bfile);

  int32 retVal = EXIT_SUCCESS;

  try {
    retVal = vm.beginExecution(entryPoint, settings.runArgs);
  } catch (ScriptError& err) {
    std::string formattedError = formatScriptError(err);
    fprintf(stderr, "%s", formattedError.c_str());
    retVal = EXIT_FAILURE;
  }

  // printf("Finished with return code %d\n", retVal);
  return retVal;
}

int32 main(int32 argc, cstring argv[]) {
  ProgramSettings settings;
  const ParseResult res = parseSettings(settings, argc, argv);

  if (res == RES_FAILED) {
    fprintf(stderr, "Use 'quickscript help' for usage info\n");
    return EXIT_FAILURE;
  }

  if (settings.command == CMD_HELP) {
    showHelpMessage();
    return EXIT_SUCCESS;
  }

  BindingsObject* bindings = BindingsObject::create();
  addStandardLibrary(bindings);

  if (settings.command == CMD_TESTS) {
    runTests(settings, bindings);
    return EXIT_SUCCESS;
  }

  BytecodeFile* bfile;
  const bool successfullyCompiled = compileBytecode(settings, &bfile, bindings);

  if (!successfullyCompiled) {
    return EXIT_FAILURE;
  }

  if (settings.command == CMD_COMPILE) {
    compileSource(*bfile, settings);
    return EXIT_SUCCESS;
  }

  return runCompiledFile(*bfile, settings, bindings);
}

