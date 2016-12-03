#pragma once

#include <ostream>
#include <string>

#include "parser/ast.hpp"

namespace minijava
{

	/**
	 * AST pretty printer
	 */
	class pretty_printer final : public ast::visitor
	{
	public:

		/**
		 * @brief
		 *     Constructs a new pretty printer which writes to the given
		 *     output stream.
		 *
		 * @param output
		 *     output stream
		 *
		 */
		pretty_printer(std::ostream& output)
				: _output{output} {}

		void visit(const ast::type& node) override;

		void visit(const ast::var_decl& node) override;

		void visit(const ast::binary_expression& node) override;

		void visit(const ast::unary_expression& node) override;

		void visit(const ast::object_instantiation& node) override;

		void visit(const ast::array_instantiation& node) override;

		void visit(const ast::array_access& node) override;

		void visit(const ast::variable_access& node) override;

		void visit(const ast::method_invocation& node) override;

		void visit(const ast::this_ref& node) override;

		void visit(const ast::boolean_constant& node) override;

		void visit(const ast::integer_constant& node) override;

		void visit(const ast::null_constant& node) override;

		void visit(const ast::local_variable_statement& node) override;

		void visit(const ast::expression_statement& node) override;

		void visit(const ast::block& node) override;

		void visit(const ast::if_statement& node) override;

		void visit(const ast::while_statement& node) override;

		void visit(const ast::return_statement& node) override;

		void visit(const ast::empty_statement& node) override;

		void visit(const ast::main_method& node) override;

		void visit(const ast::instance_method& node) override;

		void visit(const ast::class_declaration& node) override;

		void visit(const ast::program& node) override;

	private:

		/** @brief output stream */
		std::ostream& _output;

		/** @brief current indentation level */
		std::size_t _indentation_level {0};

		/** @brief whether we're currently starting the body of a method */
		bool _start_method {false};

		/** @brief whether we're currently starting the body of an if */
		bool _start_if {false};

		/** @brief whether we're currently starting the body of an else */
		bool _start_else {false};

		/** @brief whether we're currently starting the body of a loop */
		bool _start_loop {false};

		/** @brief helper that starts a regular block statement */
		void _start_block_statement();

		/** @brief whether we're currently printing the fields of a class */
		bool _in_fields {false};

		/** @brief whether we're currently printing a parameter list */
		bool _in_parameters {false};

		/** @brief whether expression should currently be parenthesized */
		bool _print_expression_parens {true};

		/**
		 * @brief
		 *     Convenience function for printing a line with the current
		 *     indentation level.
		 *
		 * This function does not append a newline character.
		 *
		 * @param line
		 *     content of the line
		 *
		 */
		void _print(const std::string& line);

		/**
		 * @brief
		 *     Convenience function for printing a line with the current
		 *     indentation level.
		 *
		 * This function appends a newline character to the end of the line.
		 *
		 * @param line
		 *     content of the line
		 *
		 */
		void _println(const std::string& line);

		/**
		 * @brief
		 *     Turns a `type_name` into a string.
		 *
		 * @param type
		 *     type name to convert
		 *
		 * @return
		 *     type name as a string
		 *
		 */
		std::string _type_name(const ast::type_name& type);
	};

}
