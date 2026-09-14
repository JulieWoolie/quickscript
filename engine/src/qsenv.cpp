#include "qs/qsenv.hpp"

#include "qs/allocator.hpp"
#include "qs/errors.hpp"
#include "qs/analysis/analyzer.hpp"
#include "qs/analysis/SemanticContext.hpp"
#include "qs/analysis/transformer.hpp"
#include "qs/codegen/compiler.hpp"
#include "qs/parse/lexer.hpp"
#include "qs/parse/parser.hpp"

QsEnvironment::QsEnvironment() {

}

QsEnvironment::~QsEnvironment() {

}

CompilationOptions& QsEnvironment::getOptions() {
  return m_options;
}

std::vector<std::string>& QsEnvironment::getLibraryDirectories() {
  return m_libraryDirectories;
}

std::vector<NativeModule>& QsEnvironment::getNativeModules() {
  return m_nativeModules;
}

bool QsEnvironment::findLibrary(const std::string_view& name, std::vector<BytecodeFile*>& out) {
  return false;
}

bool QsEnvironment::compileSourceFile(const std::string& content, conststring fileName, BytecodeFile** fileOut) {
  CompilerErrors errors = CompilerErrors(content, fileName);
  StringTable table = StringTable();
  TokenList tokens = TokenList();

  Lexer l = Lexer(content, &tokens, &table, &errors);

  try {
    l.next();
    l.lex();
  } catch (std::runtime_error& error) {
    return false;
  }

  NoFreeAllocator alloc = NoFreeAllocator();

  Parser p = Parser(&tokens, &alloc, &errors, &table);
  ScriptFileStatement* sfs = nullptr;

  try {
    sfs = p.parse();
  } catch (std::runtime_error& error) {
    return false;
  }

  TypeTable types = TypeTable();

  SemanticContext ctx = SemanticContext(types, table, errors, alloc, this);
  runSemanticAnalysis(sfs, ctx);

  if (errors.getErrorCount() != 0) {
    return false;
  }

  runSemanticTransformer(ctx, sfs);

  BytecodeFile& bFile = compile(ctx);
  *fileOut = &bFile;

  return true;
}
