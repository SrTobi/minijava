/**
 * @file astgen.hpp
 *
 * @brief
 *     Generate semantically correct random ASTs.
 *
 * The generated ASTs are meant to be used as test inputs for the compiler.
 * Attempting to *execute* the generated programs would be extremely unwise,
 * however.  Not only is executing random code a really bad idea in general,
 * but even more so as the generated programs will invoke undefined behavior
 * all over the place.
 *
 */

#pragma once

#include <cstddef>
#include <memory>

#include "parser/ast.hpp"
#include "parser/ast_factory.hpp"
#include "symbol/symbol_pool.hpp"


namespace testaux
{

	/**
	 * @brief
	 *     Generates a semantically correct random AST.
	 *
	 * The AST will have a depth of no more than a constant plus `limit`.
	 *
	 * @tparam EngineT
	 *     type of random engine
	 *
	 * @tparam PoolT
	 *     type of symbol pool
	 *
	 * @param engine
	 *     random engine to use
	 *
	 * @param pool
	 *     symbol pool to use
	 *
	 * @param factory
	 *     AST factory to use
	 *
	 * @param limit
	 *     recursion limit to obey
	 *
	 * @returns
	 *     random AST
	 *
	 */
	template <typename EngineT, typename PoolT>
	std::unique_ptr<minijava::ast::program>
	generate_semantic_ast(
		EngineT& engine,
		PoolT& pool,
		minijava::ast_factory& factory,
		std::size_t limit
	);

}  // namespace testaux


#define TESTAUX_INCLUDED_FROM_ASTGEN_HPP
#include "testaux/astgen.tpp"
#undef TESTAUX_INCLUDED_FROM_ASTGEN_HPP
