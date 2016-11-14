/**
 * @file syntaxgen.hpp
 *
 * @brief
 *     Generator for syntactically correct MiniJava programs.
 *
 */

#pragma once

#include <cstddef>
#include <vector>

#include "lexer/token.hpp"


namespace testaux
{

	/**
	 * @brief
	 *     Generates a token sequence that corresponds to a syntactically
	 *     correct random MiniJava program.
	 *
	 * @tparam RndEngT
	 *     random engine type
	 *
	 * @tparam PoolT
	 *     symbol pool type
	 *
	 * @param engine
	 *     random engine to use for generating the program
	 *
	 * @param pool
	 *     symbol pool to store identifiers and integer literals in
	 *
	 * @param depth
	 *     maximum recursion depth (in the grammar)
	 *
	 */
	template <typename RndEngT, typename PoolT>
	std::vector<minijava::token> generate_valid_program(RndEngT& engine, PoolT& pool, const std::size_t depth);

}  // namespace testaux


#define TESTAUX_INCLUDED_FROM_SYNTAXGEN_HPP
#include "syntaxgen.tpp"
#undef TESTAUX_INCLUDED_FROM_SYNTAXGEN_HPP
