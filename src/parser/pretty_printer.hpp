#pragma once

#include <ostream>

#include "parser/ast.hpp"

namespace minijava
{
	namespace ast
	{
		/**
		 * AST pretty printer
		 */
		class pretty_printer : public visitor
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

			void visit(type &node) override {
				// TODO
				visitor::visit(node);
			}

			void visit(var_decl &node) override {
				_output << "public ";
				node.type().accept(*this);
				_output << node.name();
			}

			void visit(assignment_expression &node) override {
				node.lhs().accept(*this);
				_output << " = ";
				node.rhs().accept(*this);
			}

			void visit(binary_expression &node) override {
				node.lhs().accept(*this);
				switch (node.type()) {
					case ast::binary_operation_type::type_add:
						_output << " + ";
						break;
					case ast::binary_operation_type::type_subtract:
						_output << " - ";
						break;
					case ast::binary_operation_type::type_multiplay:
						_output << " * ";
						break;
					case ast::binary_operation_type::type_divide:
						_output << " / ";
						break;
					case ast::binary_operation_type::type_modulo:
						_output << " % ";
						break;
					case ast::binary_operation_type::type_greater:
						_output << " > ";
						break;
					case ast::binary_operation_type::type_greater_equal:
						_output << " >= ";
						break;
					case ast::binary_operation_type::type_equality:
						_output << " == ";
						break;
					case ast::binary_operation_type::type_unequality:
						_output << " != ";
						break;
					case ast::binary_operation_type::type_lower:
						_output << " < ";
						break;
					case ast::binary_operation_type::type_lower_equal:
						_output << " <= ";
						break;
					case ast::binary_operation_type::type_logical_and:
						_output << " & ";
						break;
					case ast::binary_operation_type::type_logical_or:
						_output << " | ";
						break;
				}
				node.rhs().accept(*this);
			}

			void visit(unary_expression &node) override {
				switch (node.type()) {
					case ast::unary_operation_type::type_negate:
						_output << "!";
						break;
					case ast::unary_operation_type::type_not:
						_output << "~";
						break;
				}
				node.target().accept(*this);
			}

			void visit(object_instantiation &node) override {
				_output << "new " << node.class_name() << "()";
			}

			void visit(array_instantiation &node) override {
				_output << "new ";
				node.type().accept(*this);
				_output << "[";
				node.extent().accept(*this);
				_output << "]";
				//for (size_t rank = 0; rank < node.)
				// TODO: Print Type correctly
			}

			void visit(array_access &node) override {
				node.target().accept(*this);
				_output << "[";
				node.index().accept(*this);
				_output << "]";
			}

			void visit(variable_access &node) override {
				if (node.target() != nullptr) {
					node.target()->accept(*this);
					_output << ".";
				}
				_output << node.name();
			}

			void visit(method_invocation &node) override {
				if (node.target() != nullptr) {
					node.target()->accept(*this);
					_output << ".";
				}
				_output << node.name() << "(";
				for (size_t i = 0; i < node.arguments().size(); i++) {
					if (i > 0) {
						_output << ", ";
					}
					node.arguments()[i]->accept(*this);
				}
				_output << ")";
			}

			void visit(this_ref &node) override {
				node.accept(*this);
			}

			void visit(boolean_constant &node) override {
				_output << (node.value() ? "true" : "false");
			}

			void visit(integer_constant &node) override {
				_output << node.literal();
			}

			void visit(null_constant &node) override {
				(void)node;
				_output << "null";
			}

			void visit(local_variable_statement &node) override {
				node.declaration().accept(*this);
				if (node.initial_value() != nullptr) {
					_output << " = ";
					node.initial_value()->accept(*this);
				}
			}

			void visit(expression_statement &node) override {
				node.expression().accept(*this);
				_output << ";\n";
			}

			void visit(block &node) override {
				_output << "{\n";
				for (size_t i = 0; i < node.body().size(); i++) {
					node.body()[i]->accept(*this);
				}
				_output << "}\n";
			}

			void visit(if_statement &node) override {
				_output << "if (";
				node.condition().accept(*this);
				_output << ") {\n";

				node.then_statement().accept(*this);
				if (node.else_statement() == nullptr) {
					_output << "}\n";
				} else {
					_output << "} else {\n";
					node.else_statement();
					_output << "}\n";
				}
			}

			void visit(while_statement &node) override {
				_output << "while (";
				node.condition().accept(*this);
				_output << ") {\n";
				node.body().accept(*this);
				_output << "}\n";
			}

			void visit(return_statement &node) override {
				if (node.value() == nullptr) {
					_output << "return;";
				} else {
					_output << "return ";
					node.value()->accept(*this);
					_output << ";";
				}
			}

			void visit(empty_statement &node) override {
				(void)node;
				_output << ";\n";
			}

			void visit(main_method &node) override {
				_output << "public static void " << node.name() << "(String[] args) {\n";
				node.body().accept(*this);
				_output << "}\n";
			}

			void visit(method &node) override {
				_output << "public ";
				node.return_type().accept(*this);
				_output << " " << node.name() << "(";
				for (size_t i = 0; i < node.parameters().size(); i++) {
					if (i > 0) {
						_output << ", ";
					}
					node.parameters()[i]->accept(*this);
				}
				_output << ") {\n";
				node.body().accept(*this);
				_output << "}\n";
			}

			void visit(class_declaration &node) override {
				_output << "class " << node.name() << "{\n";
				for (size_t i = 0; i < node.fields().size(); i++) {
					node.fields()[i]->accept(*this);
				}

				for (size_t i = 0; i < node.main_methods().size(); i++) {
					node.main_methods()[i]->accept(*this);
				}

				for (size_t i = 0; i < node.methods().size(); i++) {
					node.methods()[i]->accept(*this);
				}

				_output << "}\n";
			}

			void visit(program &node) override {
				for (size_t i = 0; i < node.classes().size(); i++) {
					node.classes()[i]->accept(*this);
				}
			}

		private:

			/** @brief output stream */
			std::ostream& _output;

			//std::size_t indentation_level;
			// ...
		};
	}
}