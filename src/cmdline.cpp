#include "cmdline.h"

// WINAPI
#include <Windows.h>

static char** MSVCColorizerCommandLineToArgvA(const char* const cmdline, int* out_count_args) {
  if (out_count_args == nullptr) {
    SetLastError(ERROR_INVALID_PARAMETER);
    return nullptr;
  }

  char** argv_out = nullptr;
  if (*cmdline == 0) {
    DWORD size_outbuf_targetindex = MAX_PATH;
    DWORD size_outbuf = sizeof(char*) * 2 + size_outbuf_targetindex;
    while (true) {
      argv_out = (static_cast<char**>(LocalAlloc(LMEM_FIXED, size_outbuf)));
      if (argv_out == nullptr) {
        return nullptr;
      }
      const DWORD len_module =
          GetModuleFileNameA(0, static_cast<char*>(static_cast<void*>(argv_out + 2)), size_outbuf_targetindex);
      if (len_module == 0) {
        LocalFree(argv_out);
        return nullptr;
      }
      if (len_module < size_outbuf_targetindex) {
        break;
      }
      size_outbuf_targetindex *= 2;
      size_outbuf = sizeof(char*) * 2 + size_outbuf_targetindex;
      LocalFree(argv_out);
    }
    argv_out[0] = static_cast<char*>(static_cast<void*>(argv_out + 2));
    argv_out[1] = nullptr;
    *out_count_args = 1;

    return argv_out;
  }

  const char* cmdline_it = cmdline;
  if (*cmdline_it == '"') {
    cmdline_it += 1;
    while (*cmdline_it != 0) {
      if (*cmdline_it == '"') {
        cmdline_it += 1;
        break;
      }
      cmdline_it += 1;
    }
  } else {
    while (*cmdline_it != 0 && *cmdline_it != ' ' && *cmdline_it != '\t') {
      cmdline_it += 1;
    }
  }
  while (*cmdline_it == ' ' || *cmdline_it == '\t') {
    cmdline_it += 1;
  }

  DWORD argc = 1;
  if (*cmdline_it != 0) {
    argc += 1;
  }

  int count_quotes = 0;
  int count_backslashes = 0;

  while (*cmdline_it) {
    if ((*cmdline_it == ' ' || *cmdline_it == '\t') && count_quotes == 0) {
      while (*cmdline_it == ' ' || *cmdline_it == '\t') {
        cmdline_it += 1;
      }
      if (*cmdline_it != 0) {
        argc += 1;
      }
      count_backslashes = 0;
    } else if (*cmdline_it == '\\') {
      count_backslashes += 1;
      cmdline_it += 1;
    } else if (*cmdline_it == '"') {
      if ((count_backslashes & 1) == 0) {
        count_quotes += 1;
      }
      cmdline_it += 1;
      count_backslashes = 0;
      while (*cmdline_it == '"') {
        count_quotes += 1;
        cmdline_it += 1;
      }
      count_quotes = count_quotes % 3;
      if (count_quotes == 2) {
        count_quotes = 0;
      }
    } else {
      count_backslashes = 0;
      cmdline_it += 1;
    }
  }

  argv_out = static_cast<char**>(LocalAlloc(LMEM_FIXED, (argc + 1) * sizeof(char*) + (strlen(cmdline) + 1)));
  if (argv_out == nullptr) {
    return nullptr;
  }

  strcpy_s(static_cast<char*>(static_cast<void*>(argv_out + argc + 1)), (strlen(cmdline) + 1), cmdline);
  argv_out[0] = static_cast<char*>(static_cast<void*>(argv_out + argc + 1));

  char* char_it = argv_out[0];
  argc = 1;

  if (*char_it == '"') {
    cmdline_it = char_it + 1;
    while (*cmdline_it != 0) {
      if (*cmdline_it == '"') {
        cmdline_it += 1;
        break;
      }
      *char_it = *cmdline_it;
      char_it += 1;
      cmdline_it += 1;
    }
  } else {
    while (*char_it != 0 && *char_it != ' ' && *char_it != '\t') {
      char_it += 1;
    }
    cmdline_it = char_it;
    if (*cmdline_it != 0) {
      cmdline_it += 1;
    }
  }
  *char_it = 0;
  char_it += 1;
  while (*cmdline_it == ' ' || *cmdline_it == '\t') {
    cmdline_it += 1;
  }
  if (*cmdline_it == 0) {
    argv_out[argc] = nullptr;
    *out_count_args = static_cast<int>(argc);
    return argv_out;
  }

  argv_out[argc] = char_it;
  argc += 1;
  count_quotes = 0;
  count_backslashes = 0;
  while (*cmdline_it != 0) {
    if ((*cmdline_it == ' ' || *cmdline_it == '\t') && count_quotes == 0) {
      *char_it = 0;
      char_it += 1;
      count_backslashes = 0;

      do {
        cmdline_it += 1;
      } while (*cmdline_it == ' ' || *cmdline_it == '\t');
      if (*cmdline_it != 0) {
        argv_out[argc] = char_it;
        argc += 1;
      }
    } else if (*cmdline_it == '\\') {
      *char_it = *cmdline_it;
      char_it += 1;
      cmdline_it += 1;
      count_backslashes += 1;
    } else if (*cmdline_it == '"') {
      if ((count_backslashes & 1) == 0) {
        char_it -= count_backslashes / 2;
        count_quotes += 1;
      } else {
        char_it = char_it - count_backslashes / 2 - 1;
        *char_it = '"';
        char_it += 1;
      }
      cmdline_it += 1;
      count_backslashes = 0;
      while (*cmdline_it == '"') {
        count_quotes += 1;
        if (count_quotes == 3) {
          *char_it = '"';
          char_it += 1;
          count_quotes = 0;
        }
        cmdline_it += 1;
      }
      if (count_quotes == 2) {
        count_quotes = 0;
      }
    } else {
      *char_it = *cmdline_it;
      char_it += 1;
      cmdline_it += 1;
      count_backslashes = 0;
    }
  }
  *char_it = '\0';
  argv_out[argc] = nullptr;
  *out_count_args = static_cast<int>(argc);

  return argv_out;
}

std::pair<CmdLineCalcOffsetResult, size_t> calculate_offset(char* cmdline) {
  int argc_ = 0;
  char** argv_ = MSVCColorizerCommandLineToArgvA(cmdline, &argc_);
  if (argv_ == nullptr) {
    LocalFree(argv_);
    return {CmdLineCalcOffsetResult::commandLineToArgvAFailed, static_cast<size_t>(-1)};
  }
  if (argc_ <= 1) {
    LocalFree(argv_);
    return {CmdLineCalcOffsetResult::notEnoughArguments, static_cast<size_t>(-1)};
  }

  const size_t idx_argv0 = static_cast<size_t>(strstr(cmdline, argv_[0]) - cmdline);
  const size_t cmdline_skip = [cmdline, argv_, idx_argv0]() -> size_t {
    size_t cmdline_skip_ = strlen(argv_[0]) + (2 * idx_argv0) + 1;
    while (cmdline[cmdline_skip_] == ' ') {
      cmdline_skip_ += 1;
    }
    return cmdline_skip_;
  }();

  LocalFree(argv_);

  return {CmdLineCalcOffsetResult::success, cmdline_skip};
}
