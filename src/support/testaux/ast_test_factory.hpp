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
		 *     Wraps a single block statement as a block.
		 *
		 * @param blkstmt
		 *     statement to wrap
		 *
		 * @returns
		 *     block AST
		 *
		 */
		std::unique_ptr<minijava::ast::block>
		as_block(std::unique_ptr<minijava::ast::block_statement> blkstmt);

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
		make_empty_main(const std::string& name = "main");

		/**
		 * @brief
		 *     Creates an empty method.
		 *
		 * The method will be of type `void`, take no parameters and have an
		 * empty body.
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
		 *     Creates a variable declaration (class type).
		 *
		 * @param name
		 *     name of the declared variable
		 *
		 * @param type
		 *     name of the (class) type
		 *
		 * @param rank
		 *     rank of the array
		 *
		 * @returns
		 *     variable declaration AST
		 *
		 */
		std::unique_ptr<minijava::ast::var_decl>
		make_declaration(const std::string& name, const std::string& type, std::size_t rank = 0);

		/**
		 * @brief
		 *     Creates a variable declaration (primitive type).
		 *
		 * @param name
		 *     name of the declared variable
		 *
		 * @param type
		 *     primitive type
		 *
		 * @param rank
		 *     rank of the array
		 *
		 * @returns
		 *     variable declaration AST
		 *
		 */
		std::unique_ptr<minijava::ast::var_decl>
		make_declaration(const std::string& name, minijava::ast::primitive_type type, std::size_t rank = 0);

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
		 *     Wraps an instance method into a class.
		 *
		 * @param name
		 *     name of the class
		 *
		 * @param method
		 *     instance method to wrap
		 *
		 * @returns
		 *     AST for class
		 *
		 */
		std::unique_ptr<minijava::ast::class_declaration>
		as_class(const std::string& name, std::unique_ptr<minijava::ast::instance_method> method);

		/**
		 * @brief
		 *     Wraps a (field) declaration into a class.
		 *
		 * @param name
		 *     name of the class
		 *
		 * @param decl
		 *     declaration to wrap
		 *
		 * @returns
		 *     AST for class
		 *
		 */
		std::unique_ptr<minijava::ast::class_declaration>
		as_class(const std::string& name, std::unique_ptr<minijava::ast::var_decl> decl);

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
		 *     Wraps a block as a program as the body of the main method.
		 *
		 * @param body
		 *     body of main
		 *
		 * @returns
		 *     AST for complete program
		 *
		 */
		std::unique_ptr<minijava::ast::program>
		as_program(std::unique_ptr<minijava::ast::block> body);

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
