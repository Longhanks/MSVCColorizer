#ifdef MSVCCOLORIZER_NO_MODULES
#include "consolecp.cppm"

#include "syslibs.cppm"
#else
module consolecp;

import syslibs;
#endif  // MSVCCOLORIZER_NO_MODULES

class ConsoleCPRestorator::impl final {
 public:
  explicit impl() : m_old_cp(GetConsoleCP()), m_old_output_cp(GetConsoleOutputCP()) {
    // needed to make caller processes aware that this tool outputs UTF-8 to stdout
    SetConsoleCP(syslibs::cp_utf8);
    // Needed to make called processes inherit UTF-8 so that this is what arrives at this tool's stdout pipe
    SetConsoleOutputCP(syslibs::cp_utf8);

    DWORD console_mode = 0;
    const bool is_console = GetConsoleMode(GetStdHandle(syslibs::std_output_handle), &console_mode) != 0;
    if (is_console) {
      m_old_console_mode = console_mode;
      SetConsoleMode(GetStdHandle(syslibs::std_output_handle),
                     console_mode | syslibs::enable_virtual_terminal_processing | syslibs::enable_processed_output);
    } else {
      m_old_stdout_mode = _setmode(_fileno(syslibs::get_stdout()), syslibs::o_binary);
    }
  }

  impl(const impl&) = delete;
  impl& operator=(const impl&) = delete;
  impl(impl&&) noexcept = delete;
  impl& operator=(impl&&) noexcept = delete;

  ~impl() {
    if (m_old_stdout_mode != -1) {
      _setmode(_fileno(syslibs::get_stdout()), m_old_stdout_mode);
    }
    if (m_old_console_mode != 0) {
      SetConsoleMode(GetStdHandle(syslibs::std_output_handle), m_old_console_mode);
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
