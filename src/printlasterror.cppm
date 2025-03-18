#ifndef MSVCCOLORIZER_NO_MODULES
module;
#endif  // MSVCCOLORIZER_NO_MODULES

#include "compiler.h"

#ifndef MSVCCOLORIZER_PRINTLASTERROR_H

WARN_UNUSED_MACROS_OFF
#define MSVCCOLORIZER_PRINTLASTERROR_H
WARN_UNUSED_MACROS_ON

#ifdef MSVCCOLORIZER_NO_MODULES
#define EXPORT

#include "std.h"
#else
#define EXPORT export

export module printlasterror;

export import std;
#endif  // MSVCCOLORIZER_NO_MODULES

/*!
 * @brief Print GetLastError() as a readable error message
 * @param prefix UTF-8 encoded const char* used as prefix
 */
EXPORT void print_last_error(const char* prefix, const std::size_t prefix_size);

#endif  // MSVCCOLORIZER_CONSOLECP_H
