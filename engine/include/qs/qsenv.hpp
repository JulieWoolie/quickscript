#ifndef QS_QSENV_H
#define QS_QSENV_H
#include <vector>
#include <string>

#include "compiler_opts.hpp"
#include "nativeinterface.hpp"
#include "bytecode/bytecode_file.hpp"

typedef void* NativeModuleHandle;

class NativeModule {
  BindingsObject* m_bindings = nullptr;
  NativeModuleHandle m_handle = nullptr;
  std::string m_namespace = "";

  public:
    NativeModule();
    ~NativeModule();

    BindingsObject* getBindings();

    const std::string& getNamespace() const;
};

class QsEnvironment {
  CompilationOptions m_options = CompilationOptions();
  std::vector<std::string> m_libraryDirectories;

  std::vector<NativeModule> m_nativeModules;

  public:
    QsEnvironment();
    ~QsEnvironment();

    CompilationOptions& getOptions();

    std::vector<std::string>& getLibraryDirectories();

    std::vector<NativeModule>& getNativeModules();

    bool findLibrary(const std::string_view& name, std::vector<BytecodeFile*>& out);

    bool compileSourceFile(const std::string& content, conststring fileName, BytecodeFile** fileOut);
};


#endif //QS_QSENV_H
