#include <memory>

class ConsoleCPRestorator final {
 public:
  explicit ConsoleCPRestorator();
  ConsoleCPRestorator(const ConsoleCPRestorator&) = delete;
  ConsoleCPRestorator& operator=(const ConsoleCPRestorator&) = delete;
  ConsoleCPRestorator(ConsoleCPRestorator&&) noexcept = delete;
  ConsoleCPRestorator& operator=(ConsoleCPRestorator&&) noexcept = delete;
  ~ConsoleCPRestorator() noexcept;

 private:
  class impl;
  std::unique_ptr<impl> m_impl;
};
