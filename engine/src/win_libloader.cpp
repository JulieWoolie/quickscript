#include "qs/libloader.hpp"

#include <windows.h>

#include "qs/strings/utf8.hpp"

#define DLL_SUFFIX ".dll"

void suffixWithLibraryFormat(std::string& path) {
  if (path.ends_with(DLL_SUFFIX)) {
    return;
  }
  path.append(DLL_SUFFIX);
}

uint32 loadNativeLibrary(const std::string& path, NativeLibraryHandle* libOut) {
  const HMODULE result = LoadLibraryA(path.c_str());

  if (!result) {
    return GetLastError();
  }

  *libOut = result;
  return 0;
}

std::string getLibraryLoadErrorMessage(const uint32 code) {
  int8* buf = nullptr;

  const uint32 size = FormatMessageA(
    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
    nullptr,
    code,
    0,
    reinterpret_cast<LPSTR>(&buf),
    0,
    nullptr
  );

  std::string result = std::string(buf, size);
  LocalFree(buf);

  return result;
}

void freeNativeLibrary(const NativeLibraryHandle handle) {
  FreeLibrary(static_cast<HMODULE>(handle));
}

LibraryFunc findLibraryFunction(const NativeLibraryHandle handle, const conststring funcName) {
  const FARPROC procAddr = GetProcAddress(static_cast<HMODULE>(handle), funcName);
  return reinterpret_cast<LibraryFunc>(procAddr);
}