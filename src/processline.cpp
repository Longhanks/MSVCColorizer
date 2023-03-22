#include "processline.h"

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

void process_line(std::string&& str, HANDLE out) {
  const std::string processed = colorize_line(str);
  WriteFile(out, processed.c_str(), static_cast<DWORD>(std::size(processed)), nullptr, nullptr);
}
