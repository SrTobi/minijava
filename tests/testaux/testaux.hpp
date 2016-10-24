/**
 * @file testaux.hpp
 *
 * @brief
 *     This header file provides a grab bag of auxiliary features that were
 *     considered useful for writing unit tests.
 *
 */

#pragma once

#include <cstddef>
#include <iostream>
#include <memory>
#include <type_traits>
#include <utility>


/**
 * @brief
 *     Expands to `BOOST_FAIL` with a message that no exception was `throw`n.
 *
 */
#define TESTAUX_FAIL_NO_EXCEPTION()  BOOST_FAIL("No exception thrown")


/**
 * @brief
 *     Miscellaneous auxiliary features for writing unit tests.
 *
 */
namespace testaux
{

	/**
	 * @brief
	 *     A `constexpr` version of `strlen`.
	 *
	 * @tparam CharT
	 *     character type
	 *
	 * @param str
	 *     NUL terminated character string
	 *
	 * @returns
	 *     number of characters in `str` before the first NUL value
	 *
	 */
	template <typename CharT>
	constexpr std::enable_if_t<std::is_integral<CharT>{}, std::size_t>
	cx_strlen(const CharT *const str) noexcept
	{
		auto last = str;
		while (*last) { ++last; }
		return last - str;
	}

	static_assert(cx_strlen("") == 0, "self-test failed");
	static_assert(cx_strlen("a") == 1, "self-test failed");
	static_assert(cx_strlen("abc") == 3, "self-test failed");


	/**
	 * @brief
	 *     A silly printable wrapper around any type to silence Boost.Test.
	 *
	 * Boost.Test requires that types overload `operator<<` in many cases.  If
	 * you have a type that just doesn't do so, this wrapper can be used to
	 * mollify Boost.Test.  This type is an aggregate.
	 *
	 * @tparam T
	 *     type to wrap
	 *
	 */
	template <typename T>
	struct you_can_print_me
	{
		/** @brief The wrapped value. */
		T value;
	};

	/**
	 * @brief
	 *     Inserts a barely useful character sequence into the stream.
	 *
	 * @tparam T
	 *     wrapped type
	 *
	 * @param os
	 *     stream to print to
	 *
	 * @param val
	 *     wrapped value to print
	 *
	 * @returns
	 *     a reference to `os`
	 *
	 */
	template <typename T>
	std::ostream& operator<<(std::ostream& os,
	                         const you_can_print_me<T>& val)
	{
		return os << "{" << std::addressof(val) << "}";
	}

} // namespace testaux
