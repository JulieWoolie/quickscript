#ifndef QS_LIBLOADER_HPP
#define QS_LIBLOADER_HPP

#include <string>

#include "common.hpp"

typedef void* NativeLibraryHandle;
typedef void (*LibraryFunc)();

void suffixWithLibraryFormat(std::string& path);

uint32 loadNativeLibrary(const std::string& path, NativeLibraryHandle* libOut);

std::string getLibraryLoadErrorMessage(uint32 code);

void freeNativeLibrary(NativeLibraryHandle handle);

LibraryFunc findLibraryFunction(NativeLibraryHandle handle, conststring funcName);

#endif //QS_LIBLOADER_HPP
