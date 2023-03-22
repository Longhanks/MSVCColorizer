#include "printlasterror.h"

#include "utfconvert.h"

// WINAPI
#pragma warning(push)
#pragma warning(disable : 5039)  // pointer or reference to potentially throwing function passed to 'extern "C"'
#include <Windows.h>
#pragma warning(pop)

#include <string>

void print_last_error(const char* prefix, const size_t prefix_size) {
  HANDLE handle_stderr = GetStdHandle(STD_ERROR_HANDLE);
  wchar_t* msg = nullptr;
  void* msg_wchar = &msg;
  const DWORD dw = GetLastError();

  FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
                 dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), static_cast<wchar_t*>(msg_wchar), 0, nullptr);

  const size_t msg_size = wcslen(msg);
  std::string msg_utf8 = utf8_from_utf16(msg, msg_size);

  WriteFile(handle_stderr, prefix, static_cast<DWORD>(prefix_size), nullptr, nullptr);
  WriteFile(handle_stderr, " failed with error: ", sizeof(" failed with error: "), nullptr, nullptr);
  WriteFile(handle_stderr, msg_utf8.data(), static_cast<DWORD>(std::size(msg_utf8)), nullptr, nullptr);
  WriteFile(handle_stderr, "\n", sizeof("\n"), nullptr, nullptr);

  LocalFree(msg);
}
