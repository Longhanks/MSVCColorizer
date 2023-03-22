#include "cmdline.h"
#include "consolecp.h"
#include "utfconvert.h"

// WINAPI
#pragma warning(push)
#pragma warning(disable : 5039)  // pointer or reference to potentially throwing function passed to 'extern "C"'
#include <Windows.h>
#pragma warning(pop)

// Tool Help Library
#pragma warning(push)
#pragma warning(disable : 4820)  // padding
#include <TlHelp32.h>
#pragma warning(pop)

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

constexpr static size_t PIPE_BUFSIZE = 128;

[[nodiscard]] std::string colorize_line(const std::string& line) {
  const size_t index_warning = line.find(": warning C");
  if (index_warning != std::string::npos) {
    return line.substr(0, index_warning + 2) + "\033[33m" + line.substr(index_warning + 2, 14) + "\033[0m" +
           line.substr(index_warning + 16);
  }
  const size_t index_fatal_error = line.find(": fatal error C");
  if (index_fatal_error != std::string::npos) {
    return line.substr(0, index_fatal_error + 2) + "\033[31m" + line.substr(index_fatal_error, 18) + "\033[0m" +
           line.substr(index_fatal_error + 20);
  }
  const size_t index_error = line.find(": error C");
  if (index_error != std::string::npos) {
    return line.substr(0, index_error + 2) + "\033[31m" + line.substr(index_error + 2, 12) + "\033[0m" +
           line.substr(index_error + 14);
  }
  const size_t index_note = line.find(": note:");
  if (index_note != std::string::npos) {
    return line.substr(0, index_note + 2) + "\033[35m" + line.substr(index_note + 2, 5) + "\033[0m" +
           line.substr(index_note + 7);
  }
  return line;
}

[[nodiscard]] DWORD parent_of_pid(DWORD pid) {
  HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (snapshot == INVALID_HANDLE_VALUE) {
    return 0;
  }

  PROCESSENTRY32W process_entry;
  ZeroMemory(&process_entry, sizeof(PROCESSENTRY32W));
  process_entry.dwSize = sizeof(PROCESSENTRY32W);
  if (Process32FirstW(snapshot, &process_entry) != TRUE) {
    CloseHandle(snapshot);
    return 0;
  }

  do {
    if (process_entry.th32ProcessID == pid) {
      CloseHandle(snapshot);
      return process_entry.th32ParentProcessID;
    }
  } while (Process32NextW(snapshot, &process_entry));

  CloseHandle(snapshot);
  return 0;
}

[[nodiscard]] std::string pid_name(DWORD pid) {
  HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
  if (process == INVALID_HANDLE_VALUE) {
    return {};
  }

  wchar_t process_name[MAX_PATH];
  DWORD process_name_wchar_size = MAX_PATH;
  if (QueryFullProcessImageNameW(process, 0, process_name, &process_name_wchar_size) != 0) {
    const size_t process_name_size = wcslen(process_name);
    std::vector<char> utf8_buf;
    int utf8_bufsize =
        WideCharToMultiByte(CP_UTF8, 0, process_name, static_cast<int>(static_cast<ptrdiff_t>(process_name_size)),
                            nullptr, 0, nullptr, nullptr);
    utf8_buf.resize(static_cast<size_t>(static_cast<ptrdiff_t>(utf8_bufsize)) + 1);
    utf8_bufsize = WideCharToMultiByte(CP_UTF8, 0, process_name,
                                       static_cast<int>(static_cast<ptrdiff_t>(process_name_size)), std::data(utf8_buf),
                                       static_cast<int>(static_cast<ptrdiff_t>(utf8_bufsize)), nullptr, nullptr);
    utf8_buf[static_cast<size_t>(static_cast<ptrdiff_t>(utf8_bufsize))] = 0;

    CloseHandle(process);
    return std::data(utf8_buf);
  }

  CloseHandle(process);
  return {};
}

[[nodiscard]] bool is_debugged_by_vscode_vsdbg_exe() {
  DWORD parent_pid = parent_of_pid(GetCurrentProcessId());
  while (true) {
    if (parent_pid == 0) {
      return false;
    }
    const std::string parent_name = pid_name(parent_pid);
    if (parent_name.ends_with("vsdbg.exe") || parent_name.ends_with("Code.exe")) {
      return true;
    }
    parent_pid = parent_of_pid(parent_pid);
  }
}

/*!
 * @brief If in terminal, print UTF-16 using WriteConsoleW, if redirected, write UTF-8 using WriteFile
 * @param str UTF-8 encoded std::string
 * @param to_stderr If true and in terminal, print to stderr, else print to stdout, if not terminal, no impact
 */
void do_print(std::string str, bool to_stderr = false) {
  if (IsDebuggerPresent() != 0) {
    const std::wstring unicode = utf16_from_utf8((is_debugged_by_vscode_vsdbg_exe() ? colorize_line(str) : str));
    OutputDebugStringW(unicode.c_str());
    return;
  }

  DWORD console_mode = 0;
  const bool is_console = GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &console_mode) != 0;
  const bool is_console_vt =
      (console_mode & ENABLE_PROCESSED_OUTPUT) > 0 && (console_mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) > 0;

  if (is_console) {
    const std::wstring unicode = utf16_from_utf8(((is_console_vt && !to_stderr) ? colorize_line(str) : str));
    WriteConsoleW(GetStdHandle((to_stderr ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE)), unicode.c_str(),
                  static_cast<DWORD>(std::size(unicode)), nullptr, nullptr);
    return;
  }

  const std::string processed = (is_debugged_by_vscode_vsdbg_exe() ? colorize_line(str) : str);
  WriteFile(GetStdHandle((to_stderr ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE)), processed.c_str(),
            static_cast<DWORD>(std::size(processed)), nullptr, nullptr);
}

/*!
 * @brief Print GetLastError() as a readable error message
 * @param prefix UTF-8 encoded const char* used as prefix
 */
void print_error(const char* prefix, const size_t prefix_size) {
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

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  ConsoleCPRestorator cp_restorator;

  wchar_t* cmdline = GetCommandLineW();
  if (cmdline == nullptr) {
    print_error("GetCommandLineW", sizeof("GetCommandLineW"));
    return EXIT_FAILURE;
  }

  const auto [cmdline_calc_offset_result, cmdline_skip] = calculate_offset(cmdline);

  switch (cmdline_calc_offset_result) {
    case CmdLineCalcOffsetResult::commandLineToArgvWFailed: {
      print_error("CommandLineToArgvW", sizeof("CommandLineToArgvW"));
      return EXIT_FAILURE;
    }
    case CmdLineCalcOffsetResult::notEnoughArguments: {
      HANDLE handle_stderr = GetStdHandle(STD_ERROR_HANDLE);
      WriteFile(handle_stderr, "Not enough arguments: Give me a program\n",
                sizeof("Not enough arguments: Give me a program\n"), nullptr, nullptr);
      return EXIT_FAILURE;
    }
    case CmdLineCalcOffsetResult::success: {
    }
  }

  SECURITY_ATTRIBUTES security_attributes;
  security_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
  security_attributes.bInheritHandle = TRUE;
  security_attributes.lpSecurityDescriptor = nullptr;

  HANDLE pipe_stdin_read = nullptr;
  HANDLE pipe_stdin_write = nullptr;
  HANDLE pipe_stdout_read = nullptr;
  HANDLE pipe_stdout_write = nullptr;

  // Create a pipe for the child process's STDOUT.
  if (CreatePipe(&pipe_stdout_read, &pipe_stdout_write, &security_attributes, 0) == FALSE) {
    print_error("CreatePipe for stdout", sizeof("CreatePipe for stdout"));
    return EXIT_FAILURE;
  }

  // Ensure the read handle to the pipe for STDOUT is not inherited.
  if (SetHandleInformation(pipe_stdout_read, HANDLE_FLAG_INHERIT, 0) == FALSE) {
    print_error("SetHandleInformation for stdout read pipe", sizeof("SetHandleInformation for stdout read pipe"));
    return EXIT_FAILURE;
  }

  // Create a pipe for the child process's STDIN.
  if (CreatePipe(&pipe_stdin_read, &pipe_stdin_write, &security_attributes, 0) == FALSE) {
    print_error("CreatePipe for stdin", sizeof("CreatePipe for stdin"));
    return EXIT_FAILURE;
  }

  // Ensure the write handle to the pipe for STDIN is not inherited.
  if (SetHandleInformation(pipe_stdin_write, HANDLE_FLAG_INHERIT, 0) == FALSE) {
    print_error("SetHandleInformation for stdin write pipe", sizeof("SetHandleInformation for stdin write pipe"));
    return EXIT_FAILURE;
  }

  // Create the child process.
  PROCESS_INFORMATION process_information;
  STARTUPINFOW startup_info;
  BOOL create_process_success = FALSE;

  // Set up members of the PROCESS_INFORMATION structure.
  ZeroMemory(&process_information, sizeof(PROCESS_INFORMATION));

  // Set up members of the STARTUPINFO structure.
  // This structure specifies the STDIN and STDOUT handles for redirection.
  ZeroMemory(&startup_info, sizeof(STARTUPINFOW));
  startup_info.cb = sizeof(STARTUPINFOW);
  startup_info.hStdError = pipe_stdout_write;
  startup_info.hStdOutput = pipe_stdout_write;
  startup_info.hStdInput = pipe_stdin_read;
  startup_info.dwFlags |= STARTF_USESTDHANDLES;

  // Create the child process.
  create_process_success = CreateProcessW(nullptr,
                                          cmdline + cmdline_skip,  // command line
                                          nullptr,                 // process security attributes
                                          nullptr,                 // primary thread security attributes
                                          TRUE,                    // handles are inherited
                                          0,                       // creation flags
                                          nullptr,                 // use parent's environment
                                          nullptr,                 // use parent's current directory
                                          &startup_info,           // STARTUPINFO pointer
                                          &process_information);   // receives PROCESS_INFORMATION

  // If an error occurs, exit the application.
  if (create_process_success == FALSE) {
    print_error("CreateProcess", sizeof("CreateProcess"));
    return EXIT_FAILURE;
  } else {
    CloseHandle(process_information.hProcess);
    CloseHandle(process_information.hThread);
    CloseHandle(pipe_stdout_write);
    CloseHandle(pipe_stdin_read);
  }

  // Read from pipe that is the standard output for child process.
  std::string read_bytes;
  DWORD bytes_read_count = 0;
  char buf[PIPE_BUFSIZE];
  BOOL read_success = FALSE;

  while (true) {
    read_success = ReadFile(pipe_stdout_read, buf, PIPE_BUFSIZE, &bytes_read_count, nullptr);
    if (read_success == FALSE || bytes_read_count == 0) {
      break;
    }

    std::copy_n(buf, bytes_read_count, std::back_inserter(read_bytes));

    std::vector<std::string> lines;
    size_t start = 0;
    while (true) {
      if (start >= read_bytes.size()) {
        break;
      }
      const size_t pos = read_bytes.find('\n', start);
      if (pos == std::string::npos) {
        break;
      }
      const size_t len = pos - start + 1;
      lines.emplace_back(read_bytes.substr(start, len));
      start += len;
    }
    for (const std::string& raw_line : lines) {
      size_t trailing_crs = 0;
      while (true) {
        if (raw_line[raw_line.size() - 2 - trailing_crs] == '\r') {
          trailing_crs += 1;
          if (raw_line.size() - 1 - trailing_crs == 0) {
            break;
          }
          continue;
        }
        break;
      }

      std::string line =
          (trailing_crs + 1 == raw_line.size()) ? "\n" : raw_line.substr(0, raw_line.size() - 1 - trailing_crs) + '\n';
      read_bytes = read_bytes.substr(raw_line.size(), read_bytes.size());

      do_print(std::move(line));
    }
  }

  WaitForSingleObject(process_information.hProcess, INFINITE);

  HANDLE accessible_child = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, process_information.dwProcessId);
  if (accessible_child == nullptr) {
    print_error("OpenProcess", sizeof("OpenProcess"));
    return EXIT_FAILURE;
  }

  DWORD exit_code = 0;
  if (GetExitCodeProcess(accessible_child, &exit_code) == 0) {
    print_error("GetExitCodeProcess", sizeof("GetExitCodeProcess"));
    return EXIT_FAILURE;
  }

  return static_cast<int>(exit_code);
}
