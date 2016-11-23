/**
 * @file ast_test_factory.hpp
 *
 * @brief
 *     Helper functions to create AST nodes for tests.
 *
 */

#pragma once

#include "parser/ast.hpp"
#include "parser/ast_factory.hpp"
#include "symbol/symbol_pool.hpp"


namespace testaux
{
	/**
	 * @brief
	 *     Creates an empty class with the given name.
	 *
	 * @param name
	 *     name of the class
	 *
	 * @param pool
	 *     string pool to use for creating the identifier
	 *
	 * @return
	 *     empty class node
	 *
	 */
	std::unique_ptr<minijava::ast::class_declaration>
	make_empty_class(const char* name, minijava::symbol_pool<>& pool);

	/**
	 * @brief
	 *     Creates an empty class with the given name.
	 *
	 * @param name
	 *     name of the class
	 *
	 * @param pool
	 *     string pool to use for creating the identifier
	 *
	 * @param factory
	 *     factory to use for creating the AST node
	 *
	 * @return
	 *     empty class node
	 *
	 */
	std::unique_ptr<minijava::ast::class_declaration>
	make_empty_class(const char* name, minijava::symbol_pool<>& pool,
					 minijava::ast_factory& factory);
}
