#ifndef MSVCCOLORIZER_NO_MODULES
module;
#endif  // MSVCCOLORIZER_NO_MODULES

#include "compiler.h"

#ifndef MSVCCOLORIZER_PROCESSLINE_H

WARN_UNUSED_MACROS_OFF
#define MSVCCOLORIZER_PROCESSLINE_H
WARN_UNUSED_MACROS_ON

#ifdef MSVCCOLORIZER_NO_MODULES
#define EXPORT

#include "std.h"
#include "syslibs.cppm"
#else
#define EXPORT export

export module processline;

export import std;
export import syslibs;
#endif  // MSVCCOLORIZER_NO_MODULES

/*!
 * @brief Colorize warnings and errors and write UTF-8 using WriteFile
 * @param str UTF-8 encoded std::string
 * @param out The STD_OUTPUT_HANDLE
 */
EXPORT void process_line(std::string&& str, HANDLE out);

#endif  // MSVCCOLORIZER_PROCESSLINE_H
