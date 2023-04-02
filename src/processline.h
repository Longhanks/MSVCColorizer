// WINAPI
#include <Windows.h>

#include <string>

/*!
 * @brief Colorize warnings and errors and write UTF-8 using WriteFile
 * @param str UTF-8 encoded std::string
 * @param out_handle The STD_OUTPUT_HANDLE
 */
void process_line(std::string&& str, HANDLE out);
