#include "qs/qsenv.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

#include "qs/allocator.hpp"
#include "qs/errors.hpp"
#include "qs/analysis/analyzer.hpp"
#include "qs/analysis/SemanticContext.hpp"
#include "qs/analysis/transformer.hpp"
#include "qs/codegen/compiler.hpp"
#include "qs/parse/lexer.hpp"
#include "qs/parse/parser.hpp"

typedef void (*ModuleLoadCallback)(QsEnvironment* env, conststring ns);

#define LIBRARY_ENTRYPOINT_NAME "qs_onLoadNativeModule"

NativeModule::NativeModule() {

}

NativeModule::~NativeModule() {
  if (m_handle) {
    freeNativeLibrary(m_handle);
    m_handle = nullptr;
  }
  if (m_bindings) {
    BindingsObject::destroy(m_bindings);
    m_bindings = nullptr;
  }
}

BindingsObject* NativeModule::getBindings() const {
  return m_bindings;
}

const std::string& NativeModule::getNamespace() const {
  return m_namespace;
}

NativeLibraryHandle NativeModule::getHandle() const {
  return m_handle;
}

void NativeModule::setHandle(NativeLibraryHandle handle) {
  m_handle = handle;
}

void NativeModule::setNamespace(const std::string& ns) {
  m_namespace = ns;
}

void NativeModule::setBindings(BindingsObject* bindings) {
  m_bindings = bindings;
}

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
  bool libFindResult = false;

  for (const std::string& libDir : m_libraryDirectories) {
    for (const auto& entry: std::filesystem::recursive_directory_iterator(libDir)) {
      if (!entry.is_regular_file()) {
        continue;
      }

      const std::filesystem::path& path = entry.path();

      if (path.extension() != ".qsir") {
        continue;
      }

      std::ifstream stream = std::ifstream(path.string(), std::ios::binary | std::ios::ate);
      std::streamsize size = stream.tellg();
      stream.seekg(0, std::ios::beg);

      uint8 buf[size];
      stream.read(reinterpret_cast<int8*>(buf), size);

      BytecodeFile& bf = BytecodeFile::create();
      BytecodeReadResult result = deserializeBytecodeFile(buf, size, bf);

      if (result != IR_RESULT_OK) {
        BytecodeFile::destroy(bf);
        continue;
      }

      if (bf.moduleType == BF_MODTYPE_NONE
        || bf.moduleName != name
        || bf.exportedSymbols.empty()
      ) {
        continue;
      }

      out.push_back(&bf);
      libFindResult = true;
    }
  }

  return libFindResult;
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

bool QsEnvironment::loadNativeSource(conststring name, conststring ns, NativeModule** out) {
  std::string path = std::string(name);
  suffixWithLibraryFormat(path);

  NativeLibraryHandle handle = nullptr;
  const uint32 resultCode = loadNativeLibrary(path, &handle);

  if (resultCode != 0) {
    return false;
  }

  for (NativeModule& mod : m_nativeModules) {
    if (mod.getHandle() != handle) {
      continue;
    }

    // Calling free here decrements the reference
    // counter from the load we did at the start
    freeNativeLibrary(handle);

    *out = &mod;
    return true;
  }

  NativeModule mod = NativeModule();
  mod.setHandle(handle);
  mod.setNamespace(ns);

  BindingsObject* obj = BindingsObject::create();
  mod.setBindings(obj);

  const ModuleLoadCallback cb = reinterpret_cast<ModuleLoadCallback>(
    findLibraryFunction(handle, LIBRARY_ENTRYPOINT_NAME)
  );

  if (!cb) {
    BindingsObject::destroy(obj);
    return false;
  }

  m_nativeModules.push_back(mod);
  NativeModule& pushed = m_nativeModules.back();

  cb(this, ns);

  *out = &pushed;
  return true;
}

void QsEnvironment::registerNative(
  const conststring ns,
  const conststring funcName,
  const conststring signature,
  const NativeFunction func
) {
  FunctionSignature* sign = FunctionSignature::parse(signature);
  if (!sign) {
    return;
  }

  for (NativeModule& mod : m_nativeModules) {
    if (mod.getNamespace() != ns) {
      continue;
    }

    BindingsObject* bindings = mod.getBindings();
    bindings->addFunctionBinding(funcName, sign, func);

    return;
  }

  // Not registered into any existing native modules
  NativeModule mod = NativeModule();
  mod.setNamespace(ns);

  BindingsObject* obj = BindingsObject::create();
  mod.setBindings(obj);
  obj->addFunctionBinding(funcName, sign, func);

  m_nativeModules.push_back(mod);
}
