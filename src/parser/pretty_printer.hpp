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
					: _output{output}, _indentation_level{0} {}

			void visit(type &node) override {
				_output << _type_name(&node.type_name());

				for (size_t i = 0; i < node.rank(); i++)
					_output << "[]";
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
				_output << "new " << _type_name(&node.type().type_name());
				_output << "[";
				node.extent().accept(*this);
				_output << "]";
				for (size_t i = 1; i < node.type().rank(); i++)
					_output << "[]";
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
				_output << _indention();
				node.expression().accept(*this);
				_output << ";\n";
			}

			void visit(block &node) override {
				_output << _indention() << "{\n";

				_indentation_level++;
				for (size_t i = 0; i < node.body().size(); i++) {
					node.body()[i]->accept(*this);
				}
				_indentation_level--;

				_output << _indention() << "}\n";
			}

			void visit(if_statement &node) override {
				_output << _indention() << "if (";
				node.condition().accept(*this);
				_output << ") {\n";

				_indentation_level++;
				node.then_statement().accept(*this);
				_indentation_level--;

				if (node.else_statement() == nullptr) {
					_output << _indention() << "}\n";
				} else {
					_output << _indention() << "} else {\n";

					_indentation_level++;
					node.else_statement();
					_indentation_level--;

					_output << _indention() << "}\n";
				}
			}

			void visit(while_statement &node) override {
				_output << _indention() << "while (";
				node.condition().accept(*this);
				_output << ") {\n";

				_indentation_level++;
				node.body().accept(*this);
				_indentation_level--;

				_output << _indention() << "}\n";
			}

			void visit(return_statement &node) override {
				if (node.value() == nullptr) {
					_output << _indention() << "return;";
				} else {
					_output << _indention() << "return ";
					node.value()->accept(*this);
					_output << ";";
				}
			}

			void visit(empty_statement &node) override {
				(void)node;
				_output << _indention() << ";\n";
			}

			void visit(main_method &node) override {
				_output << "public static void " << node.name() << "(String[] args) {\n";
				node.body().accept(*this);
				_output << "}\n";
			}

			void visit(method &node) override {
				_output <<_indention() << "public ";
				node.return_type().accept(*this);
				_output << " " << node.name() << "(";
				for (size_t i = 0; i < node.parameters().size(); i++) {
					if (i > 0) {
						_output << ", ";
					}
					node.parameters()[i]->accept(*this);
				}
				_output << ") {\n";
				_indentation_level++;

				// don't use accept, of nody.body(), because of implizit added curly braces
				for (size_t i = 0; i < node.body().body().size(); i++) {
					node.body().body()[i]->accept(*this);
				}

				_indentation_level--;
				_output << _indention() << "}\n";
			}

			void visit(class_declaration &node) override {
				_output << _indention() << "class " << node.name() << " {\n";
				_indentation_level++;
				for (size_t i = 0; i < node.fields().size(); i++) {
					node.fields()[i]->accept(*this);
				}

				for (size_t i = 0; i < node.main_methods().size(); i++) {
					node.main_methods()[i]->accept(*this);
				}

				for (size_t i = 0; i < node.methods().size(); i++) {
					node.methods()[i]->accept(*this);
				}

				_indentation_level--;
				_output << _indention() << "}\n";
			}

			void visit(program &node) override {
				for (size_t i = 0; i < node.classes().size(); i++) {
					node.classes()[i]->accept(*this);
				}
			}



		private:

			/** @brief output stream */
			std::ostream& _output;

			std::size_t _indentation_level;

			std::string _indention() {
				return std::string(_indentation_level, '\t');
			}

			std::string _type_name(ast::type_name *type) {
				if (auto p = boost::get<ast::primitive_type>(type)) {
					switch (*p) {
						case ast::primitive_type::type_int:
							return "int";
						case ast::primitive_type::type_boolean:
							return "bool";
						case ast::primitive_type::type_void:
							return "void";
					}
				} else if (auto p = boost::get<symbol>(type)) {
					return std::string(p->c_str());
				}

				assert(false);
				return nullptr;
			}
		};
	}
}