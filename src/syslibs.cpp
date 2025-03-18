#ifdef MSVCCOLORIZER_NO_MODULES
#include "syslibs.cppm"
#else
module;
#endif  // MSVCCOLORIZER_NO_MODULES

#include <Windows.h>

#include <cerrno>
#include <cstdio>

#include "compiler.h"

#ifndef MSVCCOLORIZER_NO_MODULES
module syslibs;
#endif  // MSVCCOLORIZER_NO_MODULES

namespace syslibs {

std::FILE* get_stdout() {
  WARN_DISABLED_MACRO_EXPANSION_OFF
  return stdout;
  WARN_DISABLED_MACRO_EXPANSION_ON
}

std::FILE* get_stderr() {
  WARN_DISABLED_MACRO_EXPANSION_OFF
  return stderr;
  WARN_DISABLED_MACRO_EXPANSION_ON
}

int get_errno() { return errno; }

[[nodiscard]] DWORD makelangid(DWORD p, DWORD s) { return static_cast<DWORD>(MAKELANGID(p, s)); }

}  // namespace syslibs
