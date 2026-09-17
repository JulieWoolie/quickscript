#ifndef QS_QSENV_H
#define QS_QSENV_H
#include <vector>
#include <string>

#include "compiler_opts.hpp"
#include "libloader.hpp"
#include "nativeinterface.hpp"
#include "bytecode/bytecode_file.hpp"

class NativeModule {
  BindingsObject* m_bindings = nullptr;
  NativeLibraryHandle m_handle = nullptr;
  std::string m_namespace = "";

  public:
    NativeModule();
    ~NativeModule();

    BindingsObject* getBindings() const;

    const std::string& getNamespace() const;

    NativeLibraryHandle getHandle() const;

    void setHandle(NativeLibraryHandle handle);

    void setNamespace(const std::string& ns);

    void setBindings(BindingsObject* bindings);
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

    bool loadNativeSource(conststring name, conststring ns, NativeModule** out);

    void registerNative(
      conststring ns,
      conststring funcName,
      conststring signature,
      NativeFunction func
    );
};


#endif //QS_QSENV_H
