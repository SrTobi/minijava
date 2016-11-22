/**
 * @file analyze.hpp
 *
 * @brief
 *     Public interface for semantic analysis.
 *
 */

#pragma once

#include "parser/ast.hpp"
#include "symbol/symbol_pool.hpp"


namespace minijava
{
	// FIXME: return all data relevant to future stages
	template<typename AllocT>
	void analyze_ast(const ast::program& ast, symbol_pool<AllocT>& pool);
}

#define MINIJAVA_INCLUDED_FROM_SEMANTIC_SEMANTIC_HPP
#include "semantic/semantic.tpp"
#undef MINIJAVA_INCLUDED_FROM_SEMANTIC_SEMANTIC_HPP
