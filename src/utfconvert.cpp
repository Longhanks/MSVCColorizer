#include "utfconvert.h"

// WINAPI
#pragma warning(push)
#pragma warning(disable : 5039)  // pointer or reference to potentially throwing function passed to 'extern "C"'
#include <Windows.h>
#pragma warning(pop)

std::wstring utf16_from_utf8(const std::string& utf8) { return utf16_from_utf8(utf8.c_str(), std::size(utf8)); }

std::wstring utf16_from_utf8(const char* utf8, const size_t utf8_size) {
  size_t utf16_bufsize = static_cast<size_t>(static_cast<unsigned int>(
      MultiByteToWideChar(CP_UTF8, 0, utf8, static_cast<int>(static_cast<ptrdiff_t>(utf8_size)), nullptr, 0)));
  std::wstring str;
  str.resize(utf16_bufsize + 1);
  utf16_bufsize = static_cast<size_t>(
      static_cast<ptrdiff_t>(MultiByteToWideChar(CP_UTF8, 0, utf8, static_cast<int>(utf8_size), std::data(str),
                                                 static_cast<int>(static_cast<ptrdiff_t>(utf16_bufsize + 1)))));
  str[utf16_bufsize] = 0;
  return str;
}

std::string utf8_from_utf16(const std::wstring& utf16) { return utf8_from_utf16(utf16.c_str(), std::size(utf16)); }

std::string utf8_from_utf16(const wchar_t* utf16, const size_t utf16_size) {
  size_t utf8_bufsize = static_cast<size_t>(static_cast<ptrdiff_t>(WideCharToMultiByte(
      CP_UTF8, 0, utf16, static_cast<int>(static_cast<ptrdiff_t>(utf16_size)), nullptr, 0, nullptr, nullptr)));
  std::string str;
  str.resize(utf8_bufsize + 1);
  utf8_bufsize = static_cast<size_t>(static_cast<ptrdiff_t>(
      WideCharToMultiByte(CP_UTF8, 0, utf16, static_cast<int>(static_cast<ptrdiff_t>(utf16_size)), std::data(str),
                          static_cast<int>(static_cast<ptrdiff_t>(utf8_bufsize + 1)), nullptr, nullptr)));
  str[utf8_bufsize] = 0;
  return str;
}
