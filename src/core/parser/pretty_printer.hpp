#pragma once

#include <ostream>
#include <string>

#include "parser/ast.hpp"

namespace minijava
{
	namespace ast
	{
		/**
		 * AST pretty printer
		 */
		class pretty_printer final : public visitor
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

			void visit(const type& node) override;

			void visit(const var_decl& node) override;

			void visit(const binary_expression& node) override;

			void visit(const unary_expression& node) override;

			void visit(const object_instantiation& node) override;

			void visit(const array_instantiation& node) override;

			void visit(const array_access& node) override;

			void visit(const variable_access& node) override;

			void visit(const method_invocation& node) override;

			void visit(const this_ref& node) override;

			void visit(const boolean_constant& node) override;

			void visit(const integer_constant& node) override;

			void visit(const null_constant& node) override;

			void visit(const local_variable_statement& node) override;

			void visit(const expression_statement& node) override;

			void visit(const block& node) override;

			void visit(const if_statement& node) override;

			void visit(const while_statement& node) override;

			void visit(const return_statement& node) override;

			void visit(const empty_statement& node) override;

			void visit(const main_method& node) override;

			void visit(const instance_method& node) override;

			void visit(const class_declaration& node) override;

			void visit(const program& node) override;

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
}
