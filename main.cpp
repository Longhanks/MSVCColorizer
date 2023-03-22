#include <algorithm>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

// WINAPI
#pragma warning(push)
#pragma warning(disable : 5039)
#include <Windows.h>
#pragma warning(pop)

// CommandLineToArgvW
#include <shellapi.h>

// MSVCRT
#include <fcntl.h>
#include <io.h>

// Tool Help Library
#pragma warning(push)
#pragma warning(disable : 4820)
#include <TlHelp32.h>
#pragma warning(pop)

// PSAPI
#include <Psapi.h>

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

/*!
 * @brief Convert UTF-8 std::string to UTF-16 std::wstring
 * @param utf8 UTF-8 encoded std::string
 * @return UTF-16 encoded std::wstring
 */
[[nodiscard]] std::wstring utf16_from_utf8(const std::string& utf8) {
  size_t utf16_bufsize = static_cast<size_t>(static_cast<unsigned int>(MultiByteToWideChar(
      CP_UTF8, 0, utf8.c_str(), static_cast<int>(static_cast<ptrdiff_t>(std::size(utf8))), nullptr, 0)));
  auto str = std::make_unique_for_overwrite<wchar_t[]>(utf16_bufsize + 1);
  utf16_bufsize = static_cast<size_t>(
      static_cast<ptrdiff_t>(MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(std::size(utf8)), str.get(),
                                                 static_cast<int>(static_cast<ptrdiff_t>(utf16_bufsize + 1)))));
  str[utf16_bufsize] = 0;
  return str.get();
}

/*!
 * @brief Convert UTF-16 std::wstring to UTF-8 std::string
 * @param utf16 UTF-16 encoded std::wstring
 * @return UTF-8 encoded std::string
 */
[[nodiscard]] std::string utf8_from_utf16(const std::wstring& utf16) {
  size_t utf8_bufsize = static_cast<size_t>(static_cast<ptrdiff_t>(
      WideCharToMultiByte(CP_UTF8, 0, utf16.c_str(), static_cast<int>(static_cast<ptrdiff_t>(std::size(utf16))),
                          nullptr, 0, nullptr, nullptr)));
  auto str = std::make_unique_for_overwrite<char[]>(utf8_bufsize + 1);
  utf8_bufsize = static_cast<size_t>(static_cast<ptrdiff_t>(
      WideCharToMultiByte(CP_UTF8, 0, utf16.c_str(), static_cast<int>(static_cast<ptrdiff_t>(std::size(utf16))),
                          str.get(), static_cast<int>(static_cast<ptrdiff_t>(utf8_bufsize + 1)), nullptr, nullptr)));
  str[utf8_bufsize] = 0;
  return str.get();
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
 * @brief Format a readable error message, display a message box, and exit from the application.
 * @param prefix UTF-8 encoded std::string used as prefix
 */
void print_error_and_exit(std::string prefix) {
  void* msg = nullptr;
  void* msg_wchar = &msg;
  const DWORD dw = GetLastError();

  FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
                 dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), static_cast<wchar_t*>(msg_wchar), 0, nullptr);

  const size_t msg_size = wcslen(static_cast<const wchar_t*>(msg));
  std::vector<char> utf8_buf;
  int utf8_bufsize =
      WideCharToMultiByte(CP_UTF8, 0, static_cast<const wchar_t*>(msg),
                          static_cast<int>(static_cast<ptrdiff_t>(msg_size)), nullptr, 0, nullptr, nullptr);
  utf8_buf.resize(static_cast<size_t>(static_cast<unsigned int>(utf8_bufsize)) + 1);
  utf8_bufsize = WideCharToMultiByte(CP_UTF8, 0, static_cast<const wchar_t*>(msg),
                                     static_cast<int>(static_cast<ptrdiff_t>(msg_size)), std::data(utf8_buf),
                                     static_cast<int>(static_cast<ptrdiff_t>(utf8_bufsize)), nullptr, nullptr);
  utf8_buf[static_cast<size_t>(static_cast<unsigned int>(utf8_bufsize))] = 0;

  do_print(std::move(prefix) + " failed with error: " + std::data(utf8_buf) + "\n", true);

  LocalFree(msg);
  ExitProcess(EXIT_FAILURE);
}

class ConsoleCPRestorator final {
 public:
  explicit ConsoleCPRestorator() : m_old_cp(GetConsoleCP()), m_old_output_cp(GetConsoleOutputCP()) {
    // needed to make caller processes aware that this tool outputs UTF-8 to stdout
    SetConsoleCP(CP_UTF8);
    // Needed to make called processes inherit UTF-8 so that this is what arrives at this tool's stdout pipe
    SetConsoleOutputCP(CP_UTF8);

    DWORD console_mode = 0;
    const bool is_console = GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &console_mode) != 0;
    if (is_console) {
      m_old_console_mode = console_mode;
      SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE),
                     console_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT);
    } else {
      m_old_stdout_mode = _setmode(_fileno(stdout), _O_BINARY);
    }
  }

  ConsoleCPRestorator(const ConsoleCPRestorator&) = delete;
  ConsoleCPRestorator& operator=(const ConsoleCPRestorator&) = delete;
  ConsoleCPRestorator(ConsoleCPRestorator&&) noexcept = delete;
  ConsoleCPRestorator& operator=(ConsoleCPRestorator&&) noexcept = delete;

  ~ConsoleCPRestorator() {
    if (m_old_stdout_mode != -1) {
      _setmode(_fileno(stdout), m_old_stdout_mode);
    }
    if (m_old_console_mode != 0) {
      SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), m_old_console_mode);
    }
    SetConsoleOutputCP(m_old_output_cp);
    SetConsoleCP(m_old_cp);
  }

 private:
  UINT m_old_cp;
  UINT m_old_output_cp;
  DWORD m_old_console_mode = 0;
  int m_old_stdout_mode = -1;
};

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  ConsoleCPRestorator cp_restorator;

  wchar_t* cmdline = GetCommandLineW();
  if (cmdline == nullptr) {
    print_error_and_exit("GetCommandLineW");
  }

  int argc_w = 0;
  wchar_t** argv_w = CommandLineToArgvW(cmdline, &argc_w);
  if (argv_w == nullptr) {
    LocalFree(argv_w);
    print_error_and_exit("CommandLineToArgvW");
  }
  if (argc_w <= 1) {
    LocalFree(argv_w);
    do_print("Not enough arguments: Give me a program\n", true);
    ExitProcess(1);
  }

  std::wstring cmdline_str(cmdline);
  const size_t idx_argv0 = cmdline_str.find(argv_w[0]);
  const size_t cmdline_skip = [cmdline, argv_w, idx_argv0]() -> size_t {
    size_t cmdline_skip_ = wcslen(argv_w[0]) + (2 * idx_argv0) + 1;
    while (cmdline[cmdline_skip_] == L' ') {
      cmdline_skip_ += 1;
    }
    return cmdline_skip_;
  }();

  LocalFree(argv_w);

  const size_t cmdline_len = std::size(cmdline_str) - cmdline_skip;
  auto cmdline_mut = std::make_unique_for_overwrite<wchar_t[]>(cmdline_len + 1);
  std::copy_n((cmdline + cmdline_skip), cmdline_len, cmdline_mut.get());
  cmdline_mut[cmdline_len] = 0;

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
    print_error_and_exit("StdoutRd CreatePipe");
  }

  // Ensure the read handle to the pipe for STDOUT is not inherited.
  if (SetHandleInformation(pipe_stdout_read, HANDLE_FLAG_INHERIT, 0) == FALSE) {
    print_error_and_exit("Stdout SetHandleInformation");
  }

  // Create a pipe for the child process's STDIN.
  if (CreatePipe(&pipe_stdin_read, &pipe_stdin_write, &security_attributes, 0) == FALSE) {
    print_error_and_exit("Stdin CreatePipe");
  }

  // Ensure the write handle to the pipe for STDIN is not inherited.
  if (SetHandleInformation(pipe_stdin_write, HANDLE_FLAG_INHERIT, 0) == FALSE) {
    print_error_and_exit("Stdin SetHandleInformation");
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
                                          cmdline_mut.get(),      // command line
                                          nullptr,                // process security attributes
                                          nullptr,                // primary thread security attributes
                                          TRUE,                   // handles are inherited
                                          0,                      // creation flags
                                          nullptr,                // use parent's environment
                                          nullptr,                // use parent's current directory
                                          &startup_info,          // STARTUPINFO pointer
                                          &process_information);  // receives PROCESS_INFORMATION

  // If an error occurs, exit the application.
  if (create_process_success == FALSE) {
    print_error_and_exit("CreateProcess");
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
    print_error_and_exit("OpenProcess");
    return EXIT_FAILURE;
  }

  DWORD exit_code = 0;
  if (GetExitCodeProcess(accessible_child, &exit_code) == 0) {
    print_error_and_exit("GetExitCodeProcess");
    return EXIT_FAILURE;
  }

  return static_cast<int>(exit_code);
}
