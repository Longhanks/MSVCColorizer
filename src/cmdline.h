#include <cstddef>
#include <utility>

enum class CmdLineCalcOffsetResult { commandLineToArgvWFailed, notEnoughArguments, success };

[[nodiscard]] std::pair<CmdLineCalcOffsetResult, size_t> calculate_offset(wchar_t* cmdline);
