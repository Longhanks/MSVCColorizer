#include "utfconvert.h"

// WINAPI
#pragma warning(push)
#pragma warning(disable : 5039)  // pointer or reference to potentially throwing function passed to 'extern "C"'
#include <Windows.h>
#pragma warning(pop)

std::wstring utf16_from_utf8(const std::string& utf8) {
  size_t utf16_bufsize = static_cast<size_t>(static_cast<unsigned int>(MultiByteToWideChar(
      CP_UTF8, 0, utf8.c_str(), static_cast<int>(static_cast<ptrdiff_t>(std::size(utf8))), nullptr, 0)));
  std::wstring str;
  str.resize(utf16_bufsize + 1);
  utf16_bufsize = static_cast<size_t>(static_cast<ptrdiff_t>(
      MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(std::size(utf8)), std::data(str),
                          static_cast<int>(static_cast<ptrdiff_t>(utf16_bufsize + 1)))));
  str[utf16_bufsize] = 0;
  return str;
}

std::string utf8_from_utf16(const std::wstring& utf16) {
  size_t utf8_bufsize = static_cast<size_t>(static_cast<ptrdiff_t>(
      WideCharToMultiByte(CP_UTF8, 0, utf16.c_str(), static_cast<int>(static_cast<ptrdiff_t>(std::size(utf16))),
                          nullptr, 0, nullptr, nullptr)));
  std::string str;
  str.resize(utf8_bufsize + 1);
  utf8_bufsize = static_cast<size_t>(static_cast<ptrdiff_t>(WideCharToMultiByte(
      CP_UTF8, 0, utf16.c_str(), static_cast<int>(static_cast<ptrdiff_t>(std::size(utf16))), std::data(str),
      static_cast<int>(static_cast<ptrdiff_t>(utf8_bufsize + 1)), nullptr, nullptr)));
  str[utf8_bufsize] = 0;
  return str;
}
