#include <string>

/*!
 * @brief Convert UTF-8 std::string to UTF-16 std::wstring
 * @param utf8 UTF-8 encoded std::string
 * @return UTF-16 encoded std::wstring
 */
[[nodiscard]] std::wstring utf16_from_utf8(const std::string& utf8);

/*!
 * @brief Convert UTF-16 std::wstring to UTF-8 std::string
 * @param utf16 UTF-16 encoded std::wstring
 * @return UTF-8 encoded std::string
 */
[[nodiscard]] std::string utf8_from_utf16(const std::wstring& utf16);
