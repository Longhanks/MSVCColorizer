#include <cstddef>
#include <utility>

enum class CmdLineCalcOffsetResult { commandLineToArgvAFailed, notEnoughArguments, success };

[[nodiscard]] std::pair<CmdLineCalcOffsetResult, size_t> calculate_offset(char* cmdline);
