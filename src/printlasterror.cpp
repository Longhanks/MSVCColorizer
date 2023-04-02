#include "printlasterror.h"

// WINAPI
#include <Windows.h>

#include <string>

void print_last_error(const char* prefix, const size_t prefix_size) {
  HANDLE handle_stderr = GetStdHandle(STD_ERROR_HANDLE);
  char* msg = nullptr;
  void* msg_char = &msg;
  const DWORD dw = GetLastError();

  FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
                 dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), static_cast<char*>(msg_char), 0, nullptr);

  const size_t msg_size = strlen(msg);

  WriteFile(handle_stderr, prefix, static_cast<DWORD>(prefix_size), nullptr, nullptr);
  WriteFile(handle_stderr, " failed with error: ", sizeof(" failed with error: ") - 1, nullptr, nullptr);
  WriteFile(handle_stderr, msg, static_cast<DWORD>(msg_size), nullptr, nullptr);
  WriteFile(handle_stderr, "\n", sizeof("\n") - 1, nullptr, nullptr);

  LocalFree(msg);
}
