#include "consolecp.h"

// WINAPI
#pragma warning(push)
#pragma warning(disable : 5039)  // pointer or reference to potentially throwing function passed to 'extern "C"'
#include <Windows.h>
#pragma warning(pop)

// MSVCRT
#include <fcntl.h>
#include <io.h>

class ConsoleCPRestorator::impl final {
 public:
  explicit impl() : m_old_cp(GetConsoleCP()), m_old_output_cp(GetConsoleOutputCP()) {
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

  impl(const impl&) = delete;
  impl& operator=(const impl&) = delete;
  impl(impl&&) noexcept = delete;
  impl& operator=(impl&&) noexcept = delete;

  ~impl() {
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

ConsoleCPRestorator::ConsoleCPRestorator() : m_impl(std::make_unique<ConsoleCPRestorator::impl>()) {}

ConsoleCPRestorator::~ConsoleCPRestorator() noexcept = default;
