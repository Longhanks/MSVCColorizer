#ifdef MSVCCOLORIZER_NO_MODULES
#include "printlasterror.cppm"

#include "syslibs.cppm"
#else
module printlasterror;

import syslibs;
#endif  // MSVCCOLORIZER_NO_MODULES

void print_last_error(const char* prefix, const std::size_t prefix_size) {
  HANDLE handle_stderr = GetStdHandle(syslibs::std_error_handle);
  char* msg = nullptr;
  void* msg_char = &msg;
  const DWORD dw = GetLastError();

  FormatMessageA(syslibs::format_message_allocate_buffer | syslibs::format_message_from_system |
                     syslibs::format_message_ignore_inserts,
                 nullptr, dw, syslibs::makelangid(syslibs::lang_neutral, syslibs::sublang_default),
                 static_cast<char*>(msg_char), 0, nullptr);

  const std::size_t msg_size = std::strlen(msg);

  WriteFile(handle_stderr, prefix, static_cast<DWORD>(prefix_size), nullptr, nullptr);
  WriteFile(handle_stderr, " failed with error: ", sizeof(" failed with error: ") - 1, nullptr, nullptr);
  WriteFile(handle_stderr, msg, static_cast<DWORD>(msg_size), nullptr, nullptr);
  WriteFile(handle_stderr, "\n", sizeof("\n") - 1, nullptr, nullptr);

  LocalFree(msg);
}
