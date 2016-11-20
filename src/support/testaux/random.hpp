/**
 * @file random.hpp
 *
 * @brief
 *     Miscelaneous ranom-related utility functions.
 *
 */

#pragma once


namespace testaux
{

	/**
	 * @brief
	 *     Creates a string with `n` uniformly distributed random bytes.
	 *
	 * @tparam RndEngT
	 *     type of random engine to use
	 *
	 * @param engine
	 *     random engine to use
	 *
	 * @param n
	 *     desired number of bytes in the output string
	 *
	 * @returns
	 *     random string with `n` bytes
	 *
	 */
	template <typename RndEngT>
	std::string make_random_string(RndEngT& engine, std::size_t n);

	/**
	 * @brief
	 *     Convenience function that calls `make_random_string` with a
	 *     default-seeded instance of `std::default_random_engine`.
	 *
	 * @param n
	 *     desired number of bytes in the output string
	 *
	 * @returns
	 *     random string with `n` bytes
	 *
	 */
	std::string make_random_string(std::size_t n);

}  // namespace testaux


#define TESTAUX_INCLUDED_FROM_RANDOM_HPP
#include "testaux/random.tpp"
#undef TESTAUX_INCLUDED_FROM_RANDOM_HPP
