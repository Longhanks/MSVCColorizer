#include "cmdline.h"

// WINAPI
#pragma warning(push)
#pragma warning(disable : 5039)  // pointer or reference to potentially throwing function passed to 'extern "C"'
#include <Windows.h>
#pragma warning(pop)

// CommandLineToArgvW
#include <shellapi.h>

std::pair<CmdLineCalcOffsetResult, size_t> calculate_offset(wchar_t* cmdline) {
  int argc_w = 0;
  wchar_t** argv_w = CommandLineToArgvW(cmdline, &argc_w);
  if (argv_w == nullptr) {
    LocalFree(argv_w);
    return {CmdLineCalcOffsetResult::commandLineToArgvWFailed, static_cast<size_t>(-1)};
  }
  if (argc_w <= 1) {
    LocalFree(argv_w);
    return {CmdLineCalcOffsetResult::notEnoughArguments, static_cast<size_t>(-1)};
  }

  const size_t idx_argv0 = static_cast<size_t>(wcsstr(cmdline, argv_w[0]) - cmdline);
  const size_t cmdline_skip = [cmdline, argv_w, idx_argv0]() -> size_t {
    size_t cmdline_skip_ = wcslen(argv_w[0]) + (2 * idx_argv0) + 1;
    while (cmdline[cmdline_skip_] == L' ') {
      cmdline_skip_ += 1;
    }
    return cmdline_skip_;
  }();

  LocalFree(argv_w);

  return {CmdLineCalcOffsetResult::success, cmdline_skip};
}
