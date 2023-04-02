#include "cmdline.h"
#include "consolecp.h"
#include "printlasterror.h"
#include "processline.h"

// WINAPI
#include <Windows.h>

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

constexpr static size_t PIPE_BUFSIZE = 4096;

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  ConsoleCPRestorator cp_restorator;

  char* cmdline = GetCommandLineA();
  if (cmdline == nullptr) {
    print_last_error("GetCommandLineA", sizeof("GetCommandLineA") - 1);
    return EXIT_FAILURE;
  }

  const auto [cmdline_calc_offset_result, cmdline_skip] = calculate_offset(cmdline);

  switch (cmdline_calc_offset_result) {
    case CmdLineCalcOffsetResult::commandLineToArgvAFailed: {
      print_last_error("CommandLineToArgvA", sizeof("CommandLineToArgvA") - 1);
      return EXIT_FAILURE;
    }
    case CmdLineCalcOffsetResult::notEnoughArguments: {
      HANDLE handle_stderr = GetStdHandle(STD_ERROR_HANDLE);
      WriteFile(handle_stderr, "Not enough arguments: Give me a program\n",
                sizeof("Not enough arguments: Give me a program\n") - 1, nullptr, nullptr);
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
    print_last_error("CreatePipe for stdout", sizeof("CreatePipe for stdout") - 1);
    return EXIT_FAILURE;
  }

  // Ensure the read handle to the pipe for STDOUT is not inherited.
  if (SetHandleInformation(pipe_stdout_read, HANDLE_FLAG_INHERIT, 0) == FALSE) {
    print_last_error("SetHandleInformation for stdout read pipe",
                     sizeof("SetHandleInformation for stdout read pipe") - 1);
    return EXIT_FAILURE;
  }

  // Create a pipe for the child process's STDIN.
  if (CreatePipe(&pipe_stdin_read, &pipe_stdin_write, &security_attributes, 0) == FALSE) {
    print_last_error("CreatePipe for stdin", sizeof("CreatePipe for stdin") - 1);
    return EXIT_FAILURE;
  }

  // Ensure the write handle to the pipe for STDIN is not inherited.
  if (SetHandleInformation(pipe_stdin_write, HANDLE_FLAG_INHERIT, 0) == FALSE) {
    print_last_error("SetHandleInformation for stdin write pipe",
                     sizeof("SetHandleInformation for stdin write pipe") - 1);
    return EXIT_FAILURE;
  }

  STARTUPINFOA startup_info = {};
  startup_info.cb = sizeof(STARTUPINFOA);
  startup_info.hStdError = pipe_stdout_write;
  startup_info.hStdOutput = pipe_stdout_write;
  startup_info.hStdInput = pipe_stdin_read;
  startup_info.dwFlags |= STARTF_USESTDHANDLES;

  PROCESS_INFORMATION process_information = {};

  // Create the child process.
  BOOL create_process_success = FALSE;
  create_process_success = CreateProcessA(nullptr,
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
    print_last_error("CreateProcessA", sizeof("CreateProcessA") - 1);
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
    print_last_error("OpenProcess", sizeof("OpenProcess") - 1);
    return EXIT_FAILURE;
  }

  DWORD exit_code = 0;
  if (GetExitCodeProcess(accessible_child, &exit_code) == 0) {
    print_last_error("GetExitCodeProcess", sizeof("GetExitCodeProcess") - 1);
    return EXIT_FAILURE;
  }

  return static_cast<int>(exit_code);
}
