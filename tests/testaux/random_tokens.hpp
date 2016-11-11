/**
 * @file random_tokens.hpp
 *
 * @brief
 *     Utility functions to generate random tokens.
 *
 */

#pragma once

#include <cstddef>
#include <string>

#include "lexer/token_type.hpp"


namespace testaux
{

	/**
	 * @brief
	 *     Generates a random valid identifier with a given length.
	 *
	 * Unless `length >= 1`, the behavior is undefined.
	 *
	 * @tparam RndEngT
	 *     type of the random engine to use
	 *
	 * @param engine
	 *     random engine to use
	 *
	 * @param length
	 *     length of the identifier
	 *
	 * @returns
	 *     random identifier
	 *
	 */
	template <typename RndEngT>
	std::string get_random_identifier(RndEngT& engine, const std::size_t length);

	/**
	 * @brief
	 *     Generates a random valid identifier with a length chosen according
	 *     to a geometric distribution with parameter `p`.
	 *
	 * Unless `0 < p < 1`, the behavior is undefined.
	 *
	 * @tparam RndEngT
	 *     type of the random engine to use
	 *
	 * @param engine
	 *     random engine to use
	 *
	 * @param p
	 *     parameter of the geometric distribution
	 *
	 * @returns
	 *     random identifier
	 *
	 */
	template <typename RndEngT>
	std::string get_random_identifier(RndEngT& engine, const double p = 0.1);

	/**
	 * @brief
	 *     Generates a random valid identifier with a length chosen uniformly
	 *     between two limits.
	 *
	 * Unless `1 <= minlen <= maxlen`, the behavior is undefined.
	 *
	 * @tparam RndEngT
	 *     type of the random engine to use
	 *
	 * @param engine
	 *     random engine to use
	 *
	 * @param minlen
	 *     minimum length of the identifier (inclusive)
	 *
	 * @param maxlen
	 *     maximum length of the identifier (inclusive)
	 *
	 * @returns
	 *     random identifier
	 *
	 */
	template <typename RndEngT>
	std::string get_random_identifier(RndEngT& engine, const std::size_t minlen, const std::size_t maxlen);

	/**
	 * @brief
	 *     Generates a random valid integer literal with a given length.
	 *
	 * Unless `length >= 1`, the behavior is undefined.
	 *
	 * @tparam RndEngT
	 *     type of the random engine to use
	 *
	 * @param engine
	 *     random engine to use
	 *
	 * @param length
	 *     length of the integer literal
	 *
	 * @returns
	 *     random integer literal
	 *
	 */
	template <typename RndEngT>
	std::string get_random_integer_literal(RndEngT& engine, const std::size_t length);

	/**
	 * @brief
	 *     Generates a random valid integer literal with a length chosen
	 *     according to a geometric distribution with parameter `p`.
	 *
	 * Unless `0 < p < 1`, the behavior is undefined.
	 *
	 * @tparam RndEngT
	 *     type of the random engine to use
	 *
	 * @param engine
	 *     random engine to use
	 *
	 * @param p
	 *     parameter of the geometric distribution
	 *
	 * @returns
	 *     random integer literal
	 *
	 */
	template <typename RndEngT>
	std::string get_random_integer_literal(RndEngT& engine, const double p = 0.1);

	/**
	 * @brief
	 *     Generates a random valid integer literal with a length chosen
	 *     uniformly between two limits.
	 *
	 * Unless `1 <= minlen <= maxlen`, the behavior is undefined.
	 *
	 * @tparam RndEngT
	 *     type of the random engine to use
	 *
	 * @param engine
	 *     random engine to use
	 *
	 * @param minlen
	 *     minimum length of the integer literal (inclusive)
	 *
	 * @param maxlen
	 *     maximum length of the integer literal (inclusive)
	 *
	 * @returns
	 *     random integer literal
	 *
	 */
	template <typename RndEngT>
	std::string get_random_integer_literal(RndEngT& engine, const std::size_t minlen, const std::size_t maxlen);

	/**
	 * @brief
	 *     Uniformly picks a random keyword token.
	 *
	 * @tparam RndEngT
	 *     type of the random engine to use
	 *
	 * @param engine
	 *     random engine to use
	 *
	 * @returns
	 *     random `token_type` of `keyword` category
	 *
	 */
	template <typename RndEngT>
	minijava::token_type get_random_keyword(RndEngT& engine);

	/**
	 * @brief
	 *     Uniformly picks a random punctuation token.
	 *
	 * @tparam RndEngT
	 *     type of the random engine to use
	 *
	 * @param engine
	 *     random engine to use
	 *
	 * @returns
	 *     random `token_type` of `punctuation` category
	 *
	 */
	template <typename RndEngT>
	minijava::token_type get_random_punctuation(RndEngT& engine);

}  // namespace testaux


#define TESTAUX_INCLUDED_FROM_RANDOM_TOKENS_HPP
#include "random_tokens.tpp"
#undef TESTAUX_INCLUDED_FROM_RANDOM_TOKENS_HPP
