// WINAPI
#pragma warning(push)
#pragma warning(disable : 5039)  // pointer or reference to potentially throwing function passed to 'extern "C"'
#include <Windows.h>
#pragma warning(pop)

#include <string>

/*!
 * @brief Colorize warnings and errors and write UTF-8 using WriteFile
 * @param str UTF-8 encoded std::string
 * @param out_handle The STD_OUTPUT_HANDLE
 */
void process_line(std::string&& str, HANDLE out);
