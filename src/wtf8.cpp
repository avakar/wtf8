#include "../include/avakar/wtf8.h"
#include <stdexcept>
#include <stdint.h>

#if defined(__has_cpp_attribute)
	#if __has_cpp_attribute(likely) && (!defined(__GNUC__) || __GNUC__ >= 10)
		#define HAVE_LIKELY 1
	#endif
#endif

#if HAVE_LIKELY
#define likely [[likely]]
#define unlikely [[unlikely]]
#else
#define likely
#define unlikely
#endif

namespace avakar::wtf8 {

namespace {

template <typename T>
bool _is_utf8_cont(T ch) noexcept
{
	return (static_cast<uint8_t>(ch) & 0xc0) == 0x80;
}

template <typename T>
bool _is_utf8(std::basic_string_view<T> s, bool allow_surrogates) noexcept
{
	auto const * p = s.data();
	auto const * last = p + s.size();

	while (p != last) likely
	{
		auto ch = static_cast<uint8_t>(*p++);
		if (ch < 0x80) likely
			continue;

		if (ch < 0xc2) unlikely
			return false;

		if (ch < 0xe0)
		{
			if (p == last || !_is_utf8_cont(*p++)) unlikely
				return false;
		}
		else if (ch < 0xf0)
		{
			if (p == last || !_is_utf8_cont(*p++)
				|| p == last || !_is_utf8_cont(*p++)) unlikely
			{
				return false;
			}

			if (ch == 0xe0 && (p[-2] & 0x20) == 0) unlikely
				return false;

			if (!allow_surrogates && ch == 0xed && (p[-2] & 0x20) != 0) unlikely
				return false;
		}
		else if (ch < 0xf5)
		{
			if (p == last || !_is_utf8_cont(*p++)
				|| p == last || !_is_utf8_cont(*p++)
				|| p == last || !_is_utf8_cont(*p++)) unlikely
			{
				return false;
			}

			if (ch == 0xf0 && (p[-3] & 0x30) == 0) unlikely
				return false;

			if (ch == 0xf4 && (p[-3] & 0x30) != 0) unlikely
				return false;
		}
		else unlikely
		{
			return false;
		}
	}

	return true;
}

bool _is_surrogate(char16_t ch) noexcept
{
	return 0xd800 <= ch && ch < 0xe000;
}

bool _is_high_surrogate(char16_t ch) noexcept
{
	return 0xd840 <= ch && ch < 0xdc00;
}

bool _is_low_surrogate(char16_t ch) noexcept
{
	return 0xdc00 <= ch && ch < 0xe000;
}

template <typename T>
bool _is_utf16(std::basic_string_view<T> s) noexcept
{
	bool expect_low_surrogate = false;

	for (char16_t ch: s)
	{
		if (expect_low_surrogate)
		{
			if (!_is_low_surrogate(ch))
				return false;
			expect_low_surrogate = false;
		}
		else
		{
			if (_is_surrogate(ch))
			{
				if (!_is_high_surrogate(ch))
					return false;
				expect_low_surrogate = true;
			}
		}
	}

	return true;
}

template <typename T>
std::size_t _narrow_size(std::basic_string_view<T> s) noexcept
{
	std::size_t r = 0;
	for (char16_t ch: s)
	{
		if (ch < 0x80)
			r += 1;
		else if (ch < 0x800)
			r += 2;
		else
			r += 3;
	}
	return r;
}

template <typename T>
std::size_t _widen_size(std::basic_string_view<T> s) noexcept
{
	auto const * p = s.data();
	auto const * last = p + s.size();

	std::size_t r = 0;
	while (p != last)
	{
		auto ch = static_cast<uint8_t>(*p++);
		if (ch < 0x80)
		{
			r += 1;
			continue;
		}

		if (ch < 0xc0)
			continue;

		if (0xf0 <= ch)
			r += 2;
		else
			r += 1;
	}

	return r;
}

template <typename O, typename I>
O * _narrow(O * out, std::basic_string_view<I> in) noexcept
{
	auto const * p = in.data();
	auto const * const last = p + in.size();

	while (p != last)
	{
		char16_t ch = *p++;
		if (ch < 0x80)
		{
			*out++ = static_cast<O>(ch);
			continue;
		}

		if (ch < 0x800)
		{
			*out++ = 0xc0 | (ch >> 6);
			*out++ = 0x80 | (ch & 0x3f);
		}
		else if (_is_surrogate(ch) && p != last && _is_low_surrogate(*p))
		{
			char16_t ch2 = *p++;
			*out++ = 0xf0 | ((ch >> 8) & 0x03);
			*out++ = 0x80 | ((ch >> 2) & 0x3f);
			*out++ = 0x80 | ((ch << 4) & 0x30) | ((ch2 >> 6) & 0x0f);
			*out++ = 0x80 | (ch2 & 0x3f);
		}
		else
		{
			*out++ = 0xe0 | (ch >> 12);
			*out++ = 0x80 | ((ch >> 6) & 0x3f);
			*out++ = 0x80 | (ch & 0x3f);
		}
	}

	return out;
}

template <typename O, typename I>
O * _widen(O * out, std::basic_string_view<I> in) noexcept
{
	auto const * p = in.data();
	auto const * const last = p + in.size();

	while (p != last)
	{
		auto ch = static_cast<uint8_t>(*p++);
		if (ch < 0x80)
		{
			*out++ = ch;
			continue;
		}

		if (ch < 0xc2)
			return nullptr;

		if (ch < 0xe0)
		{
			if (p == last || !_is_utf8_cont(*p++))
				return nullptr;

			auto ch2 = static_cast<uint8_t>(p[-1]);
			*out++ = ((ch & 0x1f) << 6) | (ch2 & 0x3f);
		}
		else if (ch < 0xf0)
		{
			if (p == last || !_is_utf8_cont(*p++)
				|| p == last || !_is_utf8_cont(*p++))
			{
				return nullptr;
			}

			auto ch2 = static_cast<uint8_t>(p[-2]);
			auto ch3 = static_cast<uint8_t>(p[-1]);

			if (ch == 0xe0 && (ch2 & 0x20) == 0)
				return nullptr;

			*out++ = ((ch & 0x0f) << 12) | ((ch2 & 0x3f) << 6) | (ch3 & 0x3f);
		}
		else if (ch < 0xf5)
		{
			if (p == last || !_is_utf8_cont(*p++)
				|| p == last || !_is_utf8_cont(*p++)
				|| p == last || !_is_utf8_cont(*p++))
			{
				return nullptr;
			}

			auto ch2 = static_cast<uint8_t>(p[-3]);
			auto ch3 = static_cast<uint8_t>(p[-2]);
			auto ch4 = static_cast<uint8_t>(p[-1]);

			if (ch == 0xf0 && (ch2 & 0x30) == 0)
				return nullptr;

			if (ch == 0xf4 && (ch2 & 0x30) != 0)
				return nullptr;

			*out++ = 0xd800 | ((ch & 0x03) << 8) | ((ch2 & 0x3f) << 2) | ((ch3 & 0x30) >> 4);
			*out++ = 0xdc00 | ((ch3 & 0x0f) << 6) | (ch4 & 0x3f);
		}
		else
		{
			return nullptr;
		}
	}

	return out;
}

template <typename O, typename I>
std::basic_string<O> _convert(std::basic_string_view<I> in)
{
	auto r_size = convert_size(in);
	std::basic_string<O> r(r_size, 0);
	auto e = convert(r.data(), in);
	if (e == nullptr)
		throw std::runtime_error("invalid utf-8 sequence");
	r.resize(e - r.data());
	return r;
}

}

bool is_wtf8(std::string_view s) noexcept
{
	return _is_utf8(s, true);
}

bool is_utf8(std::string_view s) noexcept
{
	return _is_utf8(s, false);
}

bool is_utf16(std::u16string_view s) noexcept
{
	return _is_utf16(s);
}

std::size_t convert_size(std::string_view w8) noexcept
{
	return _widen_size(w8);
}

std::size_t convert_size(std::u16string_view w16) noexcept
{
	return _narrow_size(w16);
}

char * convert(char * out, std::u16string_view in) noexcept
{
	return _narrow(out, in);
}

char16_t * convert(char16_t * out, std::string_view in) noexcept
{
	return _widen(out, in);
}

std::u16string to_u16string(std::string_view in)
{
	return _convert<char16_t>(in);
}

std::string to_string(std::u16string_view in)
{
	return _convert<char>(in);
}

#if WCHAR_MAX == 0xffff
bool is_utf16(std::wstring_view s) noexcept
{
	return _is_utf16(s);
}

std::size_t convert_size(std::wstring_view w16) noexcept
{
	return _narrow_size(w16);
}

char * convert(char * out, std::wstring_view in) noexcept
{
	return _narrow(out, in);
}

wchar_t * convert(wchar_t * out, std::string_view in) noexcept
{
	return _widen(out, in);
}

std::wstring to_wstring(std::string_view in)
{
	return _convert<wchar_t>(in);
}

std::string to_string(std::wstring_view in)
{
	return _convert<char>(in);
}

#endif

#ifdef __cpp_char8_t
bool is_wtf8(std::u8string_view s) noexcept
{
	return _is_utf8(s, true);
}

bool is_utf8(std::u8string_view s) noexcept
{
	return _is_utf8(s, false);
}

std::size_t convert_size(std::u8string_view w8) noexcept
{
	return _widen_size(w8);
}

#if WCHAR_MAX == 0xffff

char8_t * convert(char8_t * out, std::wstring_view in) noexcept
{
	return _narrow(out, in);
}

wchar_t * convert(wchar_t * out, std::u8string_view in) noexcept
{
	return _widen(out, in);
}

std::u8string to_u8string(std::wstring_view in)
{
	return _convert<char8_t>(in);
}

std::wstring to_wstring(std::u8string_view in)
{
	return _convert<wchar_t>(in);
}

#endif

char8_t * convert(char8_t * out, std::u16string_view in) noexcept
{
	return _narrow(out, in);
}

char16_t * convert(char16_t * out, std::u8string_view in) noexcept
{
	return _widen(out, in);
}

std::u16string to_u16string(std::u8string_view in)
{
	return _convert<char16_t>(in);
}

std::u8string to_u8string(std::u16string_view in)
{
	return _convert<char8_t>(in);
}

#endif

}
