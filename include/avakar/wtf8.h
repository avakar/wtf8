#include <cwchar>
#include <string>
#include <string_view>
#include <type_traits>

namespace avakar::wtf8 {

bool is_wtf8(std::string_view s) noexcept;
bool is_utf8(std::string_view s) noexcept;
bool is_utf16(std::u16string_view s) noexcept;

std::size_t convert_size(std::string_view w8) noexcept;
std::size_t convert_size(std::u16string_view w16) noexcept;

char * convert(char * out, std::u16string_view in) noexcept;
char16_t * convert(char16_t * out, std::string_view in) noexcept;

std::u16string to_u16string(std::string_view in);
std::string to_string(std::u16string_view in);

#if WCHAR_MAX == 0xffff
bool is_utf16(std::wstring_view s) noexcept;

std::size_t convert_size(std::wstring_view w16) noexcept;

char * convert(char * out, std::wstring_view in) noexcept;
wchar_t * convert(wchar_t * out, std::string_view in) noexcept;
std::wstring to_wstring(std::string_view in);
std::string to_string(std::wstring_view in);
#endif

#ifdef __cpp_char8_t
bool is_wtf8(std::u8string_view s) noexcept;
bool is_utf8(std::u8string_view s) noexcept;
std::size_t convert_size(std::u8string_view w8) noexcept;

#if WCHAR_MAX == 0xffff

char8_t * convert(char8_t * out, std::wstring_view in) noexcept;
wchar_t * convert(wchar_t * out, std::u8string_view in) noexcept;
std::wstring to_wstring(std::u8string_view in);
std::u8string to_u8string(std::wstring_view in);

#endif

char8_t * convert(char8_t * out, std::u16string_view in) noexcept;
char16_t * convert(char16_t * out, std::u8string_view in) noexcept;
std::u16string to_u16string(std::u8string_view in);
std::u8string to_u8string(std::u16string_view in);
#endif

}

#pragma once
