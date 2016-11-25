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
		 *     `return`s a default-constructed (ie empty)
		 *     `std::unique_ptr<minijava::ast::expression>`.
		 *
		 * @returns
		 *     empty smart pointer
		 *
		 */
		std::unique_ptr<minijava::ast::expression> nox()
		{
			return {};
		}

		/**
		 * @brief
		 *     Creates an integer literal.
		 *
		 * @param lexval
		 *     lexical value of the integer literal
		 *
		 * @returns
		 *     integer literal AST
		 *
		 */
		std::unique_ptr<minijava::ast::integer_constant>
		make_literal(const std::string& lexval);

		/**
		 * @brief
		 *     Creates a variable access node.
		 *
		 * @param name
		 *     name of the referenced identifier
		 *
		 * @returns
		 *     variable access AST
		 *
		 */
		std::unique_ptr<minijava::ast::variable_access>
		make_idref(const std::string& name);

		/**
		 * @brief
		 *     Creates a `this`-qualified variable access node.
		 *
		 * @param name
		 *     name of the referenced identifier
		 *
		 * @returns
		 *     variable access AST
		 *
		 */
		std::unique_ptr<minijava::ast::variable_access>
		make_idref_this(const std::string& name);

		/**
		 * @brief
		 *     Creates a method invocation node with no arguments.
		 *
		 * @param name
		 *     name of the referenced identifier
		 *
		 * @returns
		 *     AST for method invocation
		 *
		 */
		std::unique_ptr<minijava::ast::method_invocation>
		make_call(const std::string& name);

		/**
		 * @brief
		 *     Creates a `this`-qualified method invocation node with no
		 *     arguments.
		 *
		 * @param name
		 *     name of the referenced identifier
		 *
		 * @returns
		 *     AST for method invocation
		 *
		 */
		std::unique_ptr<minijava::ast::method_invocation>
		make_call_this(const std::string& name);

		/**
		 * @brief
		 *     Creates a `this` access node.
		 *
		 * @returns
		 *     `this` access AST
		 *
		 */
		std::unique_ptr<minijava::ast::this_ref>
		make_this();

		/**
		 * @brief
		 *     Creates an empty block.
		 *
		 * @returns
		 *     empty block AST
		 *
		 */
		std::unique_ptr<minijava::ast::block>
		make_empty_block();

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
		 *     Wraps a single expression in a block.
		 *
		 * @param expr
		 *     expression to wrap
		 *
		 * @returns
		 *     block AST
		 *
		 */
		std::unique_ptr<minijava::ast::block>
		as_block(std::unique_ptr<minijava::ast::expression> expr);

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
		 *     Wraps a class as a program.
		 *
		 * @param clazz
		 *     class to wrap
		 *
		 * @returns
		 *     AST for complete program
		 *
		 */
		std::unique_ptr<minijava::ast::program>
		as_program(std::unique_ptr<minijava::ast::class_declaration> clazz);

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
		 *     Wraps a list of statements a program as the body of the main
		 *     method.
		 *
		 * @param stmts
		 *     statements in body of of main
		 *
		 * @returns
		 *     AST for complete program
		 *
		 */
		std::unique_ptr<minijava::ast::program>
		as_program(std::vector<std::unique_ptr<minijava::ast::block_statement>> stmts);

		/**
		 * @brief
		 *     Wraps a single statement as a program as the body of the main
		 *     method.
		 *
		 * @param stmt
		 *     statement to wrap
		 *
		 * @returns
		 *     AST for complete program
		 *
		 */
		std::unique_ptr<minijava::ast::program>
		as_program(std::unique_ptr<minijava::ast::statement> stmt);

		/**
		 * @brief
		 *     Wraps a single expression as a program as the body of the main
		 *     method.
		 *
		 * @param expr
		 *     expression to wrap
		 *
		 * @returns
		 *     AST for complete program
		 *
		 */
		std::unique_ptr<minijava::ast::program>
		as_program(std::unique_ptr<minijava::ast::expression> expr);

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
		make_hello_world(const std::string& name = "MiniJava");

		/**
		 * @brief
		 *     Extracts the pointed-to address of a smart pointer, stores it
		 *     into a pointer and `return`s the smart pointer.
		 *
		 * Usage example (silly):
		 *
		 *     ast_test_factory tf{};
		 *     int* p{};
		 *     std::unique_ptr<int> sp = tf.x(std::make_unique<int>(42));
		 *     assert(p == sp.get());
		 *
		 * This feature will of course be more useful if the smart pointer you
		 * want to get the address out of is never saved into a named variable
		 * but only occurs inside an expression that passes it on to a
		 * consumer.
		 *
		 * @tparam SmartPtrT
		 *     smart pointer type
		 *
		 * @tparam PtrT
		 *     raw pointer type
		 *
		 * @param p
		 *     pointer that should  be set to the pointed-to address
		 *
		 * @param sp
		 *     smart pointer to extract the address out of
		 *
		 * @returns
		 *     `sp`
		 *
		 */
		template <typename SmartPtrT, typename PtrT = typename SmartPtrT::pointer>
		decltype(auto) x(PtrT& p, SmartPtrT&& sp)
		{
			p = sp.get();
			return std::forward<SmartPtrT>(sp);
		}

	};  // class ast_test_factory

}  // namespace testaux
