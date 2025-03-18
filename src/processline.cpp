#ifdef MSVCCOLORIZER_NO_MODULES
#include "processline.cppm"
#else
module processline;
#endif  // MSVCCOLORIZER_NO_MODULES

static void print_with_color(const char* str, std::size_t str_size, const std::size_t diagnostic_str_size,
                             const std::size_t diagnostic_str_index, const char* color_str, std::size_t color_str_size,
                             HANDLE out) {
  const std::size_t idx_brace_close = diagnostic_str_index - 3;
  const std::size_t idx_comma = [str, idx_brace_close]() -> std::size_t {
    std::size_t idx_comma_ = idx_brace_close;
    while (str[idx_comma_] != ',') {
      idx_comma_ -= 1;
      if (str[idx_comma_] == '(') {
        return static_cast<std::size_t>(-1);
      }
    }
    return idx_comma_;
  }();
  const std::size_t idx_brace_open = [str, idx_brace_close, idx_comma]() -> std::size_t {
    std::size_t idx_brace_open_ = idx_brace_close;
    if (idx_comma == static_cast<std::size_t>(-1)) {
      idx_brace_open_ = idx_brace_close;
    }
    while (str[idx_brace_open_] != '(') {
      idx_brace_open_ -= 1;
    }
    return idx_brace_open_;
  }();
  WriteFile(out, str, static_cast<DWORD>(idx_brace_open), nullptr, nullptr);
  WriteFile(out, ":", 1, nullptr, nullptr);
  if (idx_comma != static_cast<std::size_t>(-1)) {
    WriteFile(out, str + idx_brace_open + 1, static_cast<DWORD>(idx_comma - idx_brace_open - 1), nullptr, nullptr);
    WriteFile(out, ":", 1, nullptr, nullptr);
    WriteFile(out, str + idx_comma + 1, static_cast<DWORD>(idx_brace_close - idx_comma - 1), nullptr, nullptr);
  } else {
    WriteFile(out, str + idx_brace_open + 1, static_cast<DWORD>(idx_brace_close - idx_brace_open - 1), nullptr,
              nullptr);
  }
  WriteFile(out, ": ", 2, nullptr, nullptr);
  WriteFile(out, color_str, static_cast<DWORD>(color_str_size), nullptr, nullptr);
  WriteFile(out, str + diagnostic_str_index, static_cast<DWORD>(diagnostic_str_size), nullptr, nullptr);
  WriteFile(out, "\033[0m", sizeof("\033[0m") - 1, nullptr, nullptr);
  WriteFile(out, str + diagnostic_str_index + diagnostic_str_size,
            static_cast<DWORD>(str_size - diagnostic_str_index - diagnostic_str_size), nullptr, nullptr);
}

void process_line(std::string&& str, HANDLE out) {
  const char* const c_str = str.c_str();

  const char* const fatal_link_error = strstr(c_str, ": fatal error LNK");
  if (fatal_link_error != nullptr) {
    print_with_color(c_str, std::size(str), sizeof("fatal error LNK") + 5 - 1,
                     static_cast<std::size_t>(fatal_link_error - c_str) + 2, "\033[31m", sizeof("\033[31m") - 1, out);
    return;
  }

  const char* const fatal_error = strstr(c_str, ": fatal error C");
  if (fatal_error != nullptr) {
    print_with_color(c_str, std::size(str), sizeof("fatal error C") + 5 - 1,
                     static_cast<std::size_t>(fatal_error - c_str) + 2, "\033[31m", sizeof("\033[31m") - 1, out);

    return;
  }

  const char* const error = strstr(c_str, ": error C");
  if (error != nullptr) {
    print_with_color(c_str, std::size(str), sizeof("error C") + 5 - 1, static_cast<std::size_t>(error - c_str) + 2,
                     "\033[31m", sizeof("\033[31m") - 1, out);

    return;
  }

  const char* const warning = strstr(c_str, ": warning C");
  if (warning != nullptr) {
    print_with_color(c_str, std::size(str), sizeof("warning C") + 5 - 1, static_cast<std::size_t>(warning - c_str) + 2,
                     "\033[33m", sizeof("\033[33m") - 1, out);

    return;
  }

  const char* const note = strstr(c_str, ": note:");
  if (note != nullptr) {
    print_with_color(c_str, std::size(str), sizeof("note") + 1 - 1, static_cast<std::size_t>(note - c_str) + 2,
                     "\033[35m", sizeof("\033[35m") - 1, out);

    return;
  }

  WriteFile(out, c_str, static_cast<DWORD>(std::size(str)), nullptr, nullptr);
}
