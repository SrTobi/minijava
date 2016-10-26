/**
 * @file cx_string.hpp
 *
 * @brief
 *     `constexpr` replacements for some of the C-functions in `<string.h>`.
 *
 */

#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>


namespace testaux
{

   /**
	 * @brief
	 *     A `constexpr` version of `strlen`.
	 *
	 * If `s` is not a NUL-terminated character string, the behavior is
	 * undefined.
	 *
	 * @param s
	 *     NUL terminated character string
	 *
	 * @returns
	 *     number of characters in `s` before the first NUL value
	 *
	 */
	constexpr std::size_t
	cx_strlen(const char *const s) noexcept
	{
		auto p = s;
		while (*p) { ++p; }
		return p - s;
	}

	static_assert(cx_strlen("") == 0, "");
	static_assert(cx_strlen("a") == 1, "");
	static_assert(cx_strlen("abc") == 3, "");

	/**
	 * @brief
	 *     A `constexpr` version of `strcmp`.
	 *
	 * Unlike `strcmp`, this function has well-defined behavior for `nullptr`s.
	 * A `nullptr` is less than every other string and equal to another
	 * `nullptr`.  If either parameter is neither a `nullptr` nor a
	 * NUL-terminated character string, the behavior is undefined.
	 *
	 * @param s1
	 *     first string to compare
	 *
	 * @param s2
	 *     second string to compare
	 *
	 * @returns
	 *     an integer less than, equal to, or greater than zero if `s1` is
	 *     found, respectively, to be less than, to match, or be greater than
	 *     `s2`
	 *
	 */
	constexpr int cx_strcmp(const char * s1, const char * s2) noexcept
	{
		if ((s1 == nullptr) && (s2 == nullptr)) { return 0; }
		if (s1 == nullptr) { return -256; }
		if (s2 == nullptr) { return +256; }
		while ((*s1 != '\0') && (*s2 != '\0') && (*s1 == *s2)) {
			++s1;
			++s2;
		}
		return (*s1 - *s2);
	}

	static_assert(cx_strcmp(static_cast<char*>(nullptr), static_cast<char*>(nullptr)) == 0, "");
	static_assert(cx_strcmp(static_cast<char*>(nullptr), "") < 0, "");
	static_assert(cx_strcmp(static_cast<char*>(nullptr), "abcd") < 0, "");
	static_assert(cx_strcmp("", static_cast<char*>(nullptr)) > 0, "");
	static_assert(cx_strcmp("abcd", static_cast<char*>(nullptr)) > 0, "");
	static_assert(cx_strcmp("a", "b") < 0, "");
	static_assert(cx_strcmp("abc", "axc") < 0, "");
	static_assert(cx_strcmp("aa", "aaa") < 0, "");
	static_assert(cx_strcmp("", "") == 0, "");
	static_assert(cx_strcmp("a", "a") == 0, "");
	static_assert(cx_strcmp("abc", "abc") == 0, "");
	static_assert(cx_strcmp("a", "") > 0, "");
	static_assert(cx_strcmp("b", "a") > 0, "");
	static_assert(cx_strcmp("abc", "ab") > 0, "");

}  // namespace testaux
