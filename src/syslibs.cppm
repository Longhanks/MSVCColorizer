#ifndef MSVCCOLORIZER_NO_MODULES
module;
#endif  // MSVCCOLORIZER_NO_MODULES

#ifndef MSVCCOLORIZER_SYSLIBS_H

#include "compiler.h"

WARN_UNUSED_MACROS_OFF
#define MSVCCOLORIZER_SYSLIBS_H
WARN_UNUSED_MACROS_ON

#ifdef MSVCCOLORIZER_NO_MODULES
#define EXPORT
#else
#define EXPORT export
#endif  // MSVCCOLORIZER_NO_MODULES

#include <cstdio>
#include <cstdlib>

// Windows
#include <Windows.h>
#include <processthreadsapi.h>

// MSVCRT
#include <fcntl.h>
#include <io.h>

#ifndef MSVCCOLORIZER_NO_MODULES
export module syslibs;

export using ::AttachConsole;
export using ::BOOL;
export using ::CloseHandle;
export using ::CreatePipe;
export using ::CreateProcess;
export using ::DWORD;
export using ::FormatMessageA;
export using ::FormatMessageW;
export using ::GetCommandLineA;
export using ::GetCommandLineW;
export using ::GetConsoleCP;
export using ::GetConsoleMode;
export using ::GetConsoleOutputCP;
export using ::GetCurrentThreadId;
export using ::GetCurrentThread;
export using ::GetEnvironmentVariableA;
export using ::GetEnvironmentVariableW;
export using ::GetExitCodeProcess;
export using ::GetLastError;
export using ::GetModuleFileNameA;
export using ::GetModuleFileNameW;
export using ::GetModuleHandleA;
export using ::GetModuleHandleW;
export using ::GetProcAddress;
export using ::GetStdHandle;
export using ::HANDLE;
export using ::IsDebuggerPresent;
export using ::LocalAlloc;
export using ::LocalFree;
export using ::MultiByteToWideChar;
export using ::OpenProcess;
export using ::OutputDebugStringA;
export using ::OutputDebugStringW;
export using ::PROCESS_INFORMATION;
export using ::ReadFile;
export using ::SECURITY_ATTRIBUTES;
export using ::STARTUPINFOA;
export using ::STARTUPINFOW;
export using ::SetConsoleCP;
export using ::SetConsoleMode;
export using ::SetConsoleOutputCP;
export using ::SetHandleInformation;
export using ::SetLastError;
export using ::UINT;
export using ::WaitForSingleObject;
export using ::WideCharToMultiByte;
export using ::WriteFile;
export using ::_setmode;
export using ::_fileno;
export using ::strerror_s;
export using ::strcpy_s;
export using ::strncpy_s;
export using ::strstr;

#endif  // MSVCCOLORIZER_NO_MODULES

EXPORT namespace syslibs {
  inline constexpr int bufsiz = BUFSIZ;
  inline constexpr int exit_failure = EXIT_FAILURE;
  inline constexpr int exit_success = EXIT_SUCCESS;
  inline constexpr int iolbf = _IOLBF;
  inline constexpr int o_binary = _O_BINARY;
  inline constexpr BOOL false_ = FALSE;
  inline constexpr BOOL true_ = TRUE;
  inline constexpr DWORD attach_parent_process = ATTACH_PARENT_PROCESS;
  inline constexpr DWORD enable_processed_output = ENABLE_PROCESSED_OUTPUT;
  inline constexpr DWORD enable_virtual_terminal_processing = ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  inline constexpr DWORD error_invalid_parameter = ERROR_INVALID_PARAMETER;
  inline constexpr DWORD format_message_allocate_buffer = FORMAT_MESSAGE_ALLOCATE_BUFFER;
  inline constexpr DWORD format_message_from_system = FORMAT_MESSAGE_FROM_SYSTEM;
  inline constexpr DWORD format_message_ignore_inserts = FORMAT_MESSAGE_IGNORE_INSERTS;
  inline constexpr DWORD handle_flag_inherit = HANDLE_FLAG_INHERIT;
  inline constexpr DWORD infinite = INFINITE;
  inline constexpr DWORD lang_neutral = LANG_NEUTRAL;
  inline constexpr DWORD max_path = MAX_PATH;
  inline constexpr DWORD process_query_limited_information = PROCESS_QUERY_LIMITED_INFORMATION;
  inline constexpr DWORD startf_usestdhandles = STARTF_USESTDHANDLES;
  inline constexpr DWORD std_error_handle = STD_INPUT_HANDLE;
  inline constexpr DWORD std_input_handle = STD_ERROR_HANDLE;
  inline constexpr DWORD std_output_handle = STD_OUTPUT_HANDLE;
  inline constexpr DWORD sublang_default = SUBLANG_DEFAULT;
  inline constexpr UINT cp_utf8 = CP_UTF8;
  inline constexpr UINT lmem_fixed = LMEM_FIXED;

  /*!
   * @brief Get stdout as a FILE*.
   * @return The FILE* that refers to the standard output stream.
   */
  [[nodiscard]] std::FILE* get_stdout();

  /*!
   * @brief Get stderr as a FILE*.
   * @return The FILE* that refers to the standard output stream.
   */
  [[nodiscard]] std::FILE* get_stderr();

  /*!
   * @brief Get errno as a int.
   * @return The current integer value of errno.
   */
  [[nodiscard]] int get_errno();

  /*!
   * @brief Creates a language identifier from a primary language identifier and a sublanguage identifier.
   * @param p Primary language identifier.
   * @param s Sublanguage identifier.
   * @return A language identifier.
   */
  [[nodiscard]] DWORD makelangid(DWORD p, DWORD s);
}  // namespace syslibs

#endif  // MSVCCOLORIZER_SYSLIBS_H
