#include "cmdline.h"
#include "consolecp.h"
#include "printlasterror.h"
#include "utfconvert.h"

// WINAPI
#pragma warning(push)
#pragma warning(disable : 5039)  // pointer or reference to potentially throwing function passed to 'extern "C"'
#include <Windows.h>
#pragma warning(pop)

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

constexpr static size_t PIPE_BUFSIZE = 4096;

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
 * @brief Colorize warnings and errors and write UTF-8 using WriteFile
 * @param str UTF-8 encoded std::string
 * @param out_handle The STD_OUTPUT_HANDLE
 */
void process_line(std::string&& str, HANDLE out) {
  const std::string processed = colorize_line(str);
  WriteFile(out, processed.c_str(), static_cast<DWORD>(std::size(processed)), nullptr, nullptr);
}

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  ConsoleCPRestorator cp_restorator;

  wchar_t* cmdline = GetCommandLineW();
  if (cmdline == nullptr) {
    print_last_error("GetCommandLineW", sizeof("GetCommandLineW"));
    return EXIT_FAILURE;
  }

  const auto [cmdline_calc_offset_result, cmdline_skip] = calculate_offset(cmdline);

  switch (cmdline_calc_offset_result) {
    case CmdLineCalcOffsetResult::commandLineToArgvWFailed: {
      print_last_error("CommandLineToArgvW", sizeof("CommandLineToArgvW"));
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
    print_last_error("CreatePipe for stdout", sizeof("CreatePipe for stdout"));
    return EXIT_FAILURE;
  }

  // Ensure the read handle to the pipe for STDOUT is not inherited.
  if (SetHandleInformation(pipe_stdout_read, HANDLE_FLAG_INHERIT, 0) == FALSE) {
    print_last_error("SetHandleInformation for stdout read pipe", sizeof("SetHandleInformation for stdout read pipe"));
    return EXIT_FAILURE;
  }

  // Create a pipe for the child process's STDIN.
  if (CreatePipe(&pipe_stdin_read, &pipe_stdin_write, &security_attributes, 0) == FALSE) {
    print_last_error("CreatePipe for stdin", sizeof("CreatePipe for stdin"));
    return EXIT_FAILURE;
  }

  // Ensure the write handle to the pipe for STDIN is not inherited.
  if (SetHandleInformation(pipe_stdin_write, HANDLE_FLAG_INHERIT, 0) == FALSE) {
    print_last_error("SetHandleInformation for stdin write pipe", sizeof("SetHandleInformation for stdin write pipe"));
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
    print_last_error("CreateProcess", sizeof("CreateProcess"));
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
  HANDLE handle_stdout = GetStdHandle(STD_OUTPUT_HANDLE);

  while (true) {
    read_success = ReadFile(pipe_stdout_read, buf, PIPE_BUFSIZE, &bytes_read_count, nullptr);
    if (read_success == FALSE || bytes_read_count == 0) {
      break;
    }

    std::copy_n(buf, bytes_read_count, std::back_inserter(read_bytes));

    std::vector<std::string> lines;
    size_t start = 0;
    while (true) {
      if (start >= std::size(read_bytes)) {
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
        if (raw_line[std::size(raw_line) - 2 - trailing_crs] == '\r') {
          trailing_crs += 1;
          if (std::size(raw_line) - 1 - trailing_crs == 0) {
            break;
          }
          continue;
        }
        break;
      }

      std::string line = (trailing_crs + 1 == std::size(raw_line))
                             ? "\n"
                             : raw_line.substr(0, std::size(raw_line) - 1 - trailing_crs) + '\n';
      read_bytes = read_bytes.substr(std::size(raw_line), std::size(read_bytes));

      process_line(std::move(line), handle_stdout);
    }
  }

  WaitForSingleObject(process_information.hProcess, INFINITE);

  HANDLE accessible_child = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, process_information.dwProcessId);
  if (accessible_child == nullptr) {
    print_last_error("OpenProcess", sizeof("OpenProcess"));
    return EXIT_FAILURE;
  }

  DWORD exit_code = 0;
  if (GetExitCodeProcess(accessible_child, &exit_code) == 0) {
    print_last_error("GetExitCodeProcess", sizeof("GetExitCodeProcess"));
    return EXIT_FAILURE;
  }

  return static_cast<int>(exit_code);
}
