#ifndef MSVCCOLORIZER_NO_MODULES
module;
#endif  // MSVCCOLORIZER_NO_MODULES

#include "compiler.h"

#ifndef MSVCCOLORIZER_CMDLINE_H

WARN_UNUSED_MACROS_OFF
#define MSVCCOLORIZER_CMDLINE_H
WARN_UNUSED_MACROS_ON

#ifdef MSVCCOLORIZER_NO_MODULES
#define EXPORT

#include "std.h"
#else
#define EXPORT export

export module cmdline;

export import std;
#endif  // MSVCCOLORIZER_NO_MODULES

EXPORT enum class CmdLineCalcOffsetResult { commandLineToArgvAFailed, notEnoughArguments, success };

EXPORT [[nodiscard]] std::pair<CmdLineCalcOffsetResult, std::size_t> calculate_offset(char* cmdline);

#endif  // MSVCCOLORIZER_CMDLINE_H
