#ifndef MSVCCOLORIZER_NO_MODULES
module;
#endif  // MSVCCOLORIZER_NO_MODULES

#include "compiler.h"

#ifndef MSVCCOLORIZER_CONSOLECP_H

WARN_UNUSED_MACROS_OFF
#define MSVCCOLORIZER_CONSOLECP_H
WARN_UNUSED_MACROS_ON

#ifdef MSVCCOLORIZER_NO_MODULES
#define EXPORT

#include "std.h"
#else
#define EXPORT export

export module consolecp;

export import std;
#endif  // MSVCCOLORIZER_NO_MODULES

EXPORT class ConsoleCPRestorator final {
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

#endif  // MSVCCOLORIZER_CONSOLECP_H
