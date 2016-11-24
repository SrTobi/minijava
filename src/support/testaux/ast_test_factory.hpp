/**
 * @file ast_test_factory.hpp
 *
 * @brief
 *     Helper functions to create ASTs for tests.
 *
 */

#pragma once

#include <memory>
#include <random>

#include "parser/ast.hpp"
#include "parser/ast_factory.hpp"
#include "symbol/symbol_pool.hpp"


namespace testaux
{

	/**
	 * @brief
	 *     Factory with many convenience functions to create ASTs more quickly.
	 *
	 * The idea is to use one instance of this `class` to create one program.
	 * Doing so might require one or more member functions to be called.
	 * Sometimes, you will be unable to construct the whole AST using only
	 * member functions and will have to create nodes yourself.  In order to
	 * obtain a consistent AST, use the exposed `pool` and `factory` for this.
	 *
	 */
	struct ast_test_factory final
	{

		/** @brief Symbol pool that can be used when needed. */
		minijava::symbol_pool<> pool{};

		/** @brief AST factory that can be used when needed. */
		minijava::ast_factory factory{};

		/** @brief Random engine that can be used when needed. */
		std::default_random_engine engine{};

		/**
		 * @brief
		 *     Creates an empty main method.
		 *
		 * @param name
		 *     name of the method
		 *
		 * @returns
		 *     main method AST
		 *
		 */
		std::unique_ptr<minijava::ast::main_method>
		make_empty_main(const std::string& name);

		/**
		 * @brief
		 *     Creates an empty method.
		 *
		 * The method will be of type `void`, take no parameters and have an empty
		 * body.
		 *
		 * @param name
		 *     name of the method
		 *
		 * @returns
		 *     instance method AST
		 *
		 */
		std::unique_ptr<minijava::ast::instance_method>
		make_empty_method(const std::string& name);

		/**
		 * @brief
		 *     Creates an empty class with the given name.
		 *
		 * @param name
		 *     name of the class
		 *
		 * @returns
		 *     empty class node
		 *
		 */
		std::unique_ptr<minijava::ast::class_declaration>
		make_empty_class(const std::string& name);

		/**
		 * @brief
		 *     Creates an empty class with the name `TestN` where `N` is the ID
		 *     of the node.
		 *
		 * @returns
		 *     empty class node
		 *
		 */
		std::unique_ptr<minijava::ast::class_declaration>
		make_empty_class();

		/**
		 * @brief
		 *     Creates an empty class with a random name.
		 *
		 * @returns
		 *     empty class node
		 *
		 */
		std::unique_ptr<minijava::ast::class_declaration>
		make_empty_random_class();

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
		 * @returns
		 *     AST for class
		 *
		 */
		std::unique_ptr<minijava::ast::class_declaration>
		as_class(const std::string& name, std::unique_ptr<minijava::ast::main_method> method);

		/**
		 * @brief
		 *     Wraps a main method into a program with a single class.
		 *
		 * @param method
		 *     main method to wrap
		 *
		 * @returns
		 *     AST for complete program
		 *
		 */
		std::unique_ptr<minijava::ast::program>
		as_program(std::unique_ptr<minijava::ast::main_method> method);

		/**
		 * @brief
		 *     Creates a <q>hello world</q> program in a class with the given name.
		 *
		 * @param name
		 *     name of the class
		 *
		 * @returns
		 *     program AST
		 *
		 */
		std::unique_ptr<minijava::ast::program>
		make_hello_world(const std::string& name);

	};  // class ast_test_factory

}  // namespace testaux
