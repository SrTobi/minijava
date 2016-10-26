/**
 * @file testaux.hpp
 *
 * @brief
 *     This header file provides a grab bag of auxiliary features that were
 *     considered useful for writing unit tests.
 *
 */

#pragma once

#include <iostream>
#include <memory>


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
