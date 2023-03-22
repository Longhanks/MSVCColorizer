#include "processline.h"

#include <cstring>

static void print_with_color(const char* str, size_t str_size, const size_t diagnostic_str_size,
                             const size_t diagnostic_str_index, const char* color_str, size_t color_str_size,
                             HANDLE out) {
  WriteFile(out, str, static_cast<DWORD>(diagnostic_str_index), nullptr, nullptr);
  WriteFile(out, color_str, static_cast<DWORD>(color_str_size), nullptr, nullptr);
  WriteFile(out, str + diagnostic_str_index, static_cast<DWORD>(diagnostic_str_size), nullptr, nullptr);
  WriteFile(out, "\033[0m", sizeof("\033[0m") - 1, nullptr, nullptr);
  WriteFile(out, str + diagnostic_str_index + diagnostic_str_size,
            static_cast<DWORD>(str_size - diagnostic_str_index - diagnostic_str_size), nullptr, nullptr);
}

void process_line(std::string&& str, HANDLE out) {
  const char* c_str = str.c_str();

  const char* fatal_link_error = strstr(c_str, ": fatal error LNK");
  if (fatal_link_error != nullptr) {
    print_with_color(c_str, std::size(str), sizeof("fatal error LNK") + 5 - 1,
                     static_cast<size_t>(fatal_link_error - c_str) + 2, "\033[31;1m", sizeof("\033[31;1m") - 1, out);
    return;
  }

  const char* fatal_error = strstr(c_str, ": fatal error C");
  if (fatal_error != nullptr) {
    print_with_color(c_str, std::size(str), sizeof("fatal error C") + 5 - 1,
                     static_cast<size_t>(fatal_error - c_str) + 2, "\033[31;1m", sizeof("\033[31;1m") - 1, out);

    return;
  }

  const char* error = strstr(c_str, ": error C");
  if (error != nullptr) {
    print_with_color(c_str, std::size(str), sizeof("error C") + 5 - 1, static_cast<size_t>(error - c_str) + 2,
                     "\033[31;1m", sizeof("\033[31;1m") - 1, out);

    return;
  }

  const char* warning = strstr(c_str, ": warning C");
  if (warning != nullptr) {
    print_with_color(c_str, std::size(str), sizeof("warning C") + 5 - 1, static_cast<size_t>(warning - c_str) + 2,
                     "\033[33;1m", sizeof("\033[33;1m") - 1, out);

    return;
  }

  const char* note = strstr(c_str, ": note:");
  if (note != nullptr) {
    print_with_color(c_str, std::size(str), sizeof("note") + 1 - 1, static_cast<size_t>(note - c_str) + 2, "\033[35;1m",
                     sizeof("\033[35;1m") - 1, out);

    return;
  }

  WriteFile(out, c_str, static_cast<DWORD>(std::size(str)), nullptr, nullptr);
}
