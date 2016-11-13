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

			void visit(type &node) override;

			void visit(var_decl &node) override;

			void visit(assignment_expression &node) override;

			void visit(binary_expression &node) override;

			void visit(unary_expression &node) override;

			void visit(object_instantiation &node) override;

			void visit(array_instantiation &node) override;

			void visit(array_access &node) override;

			void visit(variable_access &node) override;

			void visit(method_invocation &node) override;

			void visit(this_ref &node) override;

			void visit(boolean_constant &node) override;

			void visit(integer_constant &node) override;

			void visit(null_constant &node) override;

			void visit(local_variable_statement &node) override;

			void visit(expression_statement &node) override;

			void visit(block &node) override;

			void visit(if_statement &node) override;

			void visit(while_statement &node) override;

			void visit(return_statement &node) override;

			void visit(empty_statement &node) override;

			void visit(main_method &node) override;

			void visit(method &node) override;

			void visit(class_declaration &node) override;

			void visit(program &node) override;

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
