/**
 * @file ast_test_factory.hpp
 *
 * @brief
 *     Helper functions to create AST nodes for tests.
 *
 */

#pragma once

#include <memory>

#include "parser/ast.hpp"
#include "parser/ast_factory.hpp"
#include "symbol/symbol_pool.hpp"


namespace testaux
{

	/**
	 * @brief
	 *     Creates an empty main method.
	 *
	 * @param name
	 *     name of the method
	 *
	 * @param pool
	 *     string pool to use for creating the identifier
	 *
	 * @param factory
	 *     factory to use for creating the AST node
	 *
	 * @returns
	 *     main method AST
	 *
	 */
	std::unique_ptr<minijava::ast::main_method>
	make_empty_main(const std::string& name,
					minijava::symbol_pool<>& pool,
					minijava::ast_factory& factory);

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
	 * @returns
	 *     empty class node
	 *
	 */
	std::unique_ptr<minijava::ast::class_declaration>
	make_empty_class(const std::string& name, minijava::symbol_pool<>& pool);

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
	 * @returns
	 *     empty class node
	 *
	 */
	std::unique_ptr<minijava::ast::class_declaration>
	make_empty_class(const std::string& name,
					 minijava::symbol_pool<>& pool,
					 minijava::ast_factory& factory);


	/**
	 * @brief
	 *     Wraps a main method into a class.
	 *
	 * @param name
	 *     name of the class
	 *
	 * @param method
	 *     main method to wrap
	 *
	 * @param pool
	 *     string pool to use for creating the identifier
	 *
	 * @param factory
	 *     factory to use for creating the AST node
	 *
	 * @returns
	 *     AST for class
	 *
	 */
	std::unique_ptr<minijava::ast::class_declaration>
	as_class(const std::string& name,
			 std::unique_ptr<minijava::ast::main_method> method,
			 minijava::symbol_pool<>& pool,
			 minijava::ast_factory& factory);

	/**
	 * @brief
	 *     Wraps a main method into a program with a single class.
	 *
	 * @param method
	 *     main method to wrap
	 *
	 * @param pool
	 *     string pool to use for creating the identifier
	 *
	 * @param factory
	 *     factory to use for creating the AST node
	 *
	 * @returns
	 *     AST for complete program
	 *
	 */
	std::unique_ptr<minijava::ast::program>
	as_program(std::unique_ptr<minijava::ast::main_method> method,
			   minijava::symbol_pool<>& pool,
			   minijava::ast_factory& factory);

	/**
	 * @brief
	 *     Creates a <q>hello world</q> program in a class with the given name.
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
	 * @returns
	 *     program AST
	 *
	 */
	std::unique_ptr<minijava::ast::program>
	make_hello_world(const std::string& name, minijava::symbol_pool<>& pool,
					 minijava::ast_factory& factory);

}  // namespace testaux
