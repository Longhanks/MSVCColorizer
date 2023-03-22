#include <string>

/*!
 * @brief Convert UTF-8 std::string to UTF-16 std::wstring
 * @param utf8 UTF-8 encoded std::string
 * @return UTF-16 encoded std::wstring
 */
[[nodiscard]] std::wstring utf16_from_utf8(const std::string& utf8);

/*!
 * @brief Convert UTF-8 const char* to UTF-16 std::wstring
 * @param utf8 UTF-8 encoded const char*
 * @param utf8_size size of utf8
 * @return UTF-16 encoded std::wstring
 */
[[nodiscard]] std::wstring utf16_from_utf8(const char* utf8, size_t utf8_size);

/*!
 * @brief Convert UTF-16 std::wstring to UTF-8 std::string
 * @param utf16 UTF-16 encoded std::wstring
 * @return UTF-8 encoded std::string
 */
[[nodiscard]] std::string utf8_from_utf16(const std::wstring& utf16);

/*!
 * @brief Convert UTF-16 const wchar_t* to UTF-8 std::string
 * @param utf16 UTF-16 encoded const wchar_t*
 * @param utf16_size size of utf16
 * @return UTF-8 encoded std::string
 */
[[nodiscard]] std::string utf8_from_utf16(const wchar_t* utf16, size_t utf16_size);
