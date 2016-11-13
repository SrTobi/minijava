#include "parser/pretty_printer.hpp"

namespace minijava
{
	namespace ast
	{
		void pretty_printer::visit(type &node)
		{
			_output << _type_name(node.name());

			for (size_t i = 0; i < node.rank(); i++)
				_output << "[]";
		}

		void pretty_printer::visit(var_decl &node)
		{
			_output << "public ";
			node.var_type().accept(*this);
			_output << node.name();
		}

		void pretty_printer::visit(assignment_expression &node)
		{
			node.lhs().accept(*this);
			_output << " = ";
			node.rhs().accept(*this);
		}

		void pretty_printer::visit(binary_expression &node)
		{
			node.lhs().accept(*this);
			switch (node.type()) {
				case ast::binary_operation_type::add:
					_output << " + ";
					break;
				case ast::binary_operation_type::subtract:
					_output << " - ";
					break;
				case ast::binary_operation_type::multiply:
					_output << " * ";
					break;
				case ast::binary_operation_type::divide:
					_output << " / ";
					break;
				case ast::binary_operation_type::modulo:
					_output << " % ";
					break;
				case ast::binary_operation_type::greater:
					_output << " > ";
					break;
				case ast::binary_operation_type::greater_equal:
					_output << " >= ";
					break;
				case ast::binary_operation_type::equal:
					_output << " == ";
					break;
				case ast::binary_operation_type::not_equal:
					_output << " != ";
					break;
				case ast::binary_operation_type::lower:
					_output << " < ";
					break;
				case ast::binary_operation_type::lower_equal:
					_output << " <= ";
					break;
				case ast::binary_operation_type::logical_and:
					_output << " && ";
					break;
				case ast::binary_operation_type::logical_or:
					_output << " || ";
					break;
			}
			node.rhs().accept(*this);
		}

		void pretty_printer::visit(unary_expression &node)
		{
			switch (node.type()) {
				case ast::unary_operation_type::minus:
					_output << "-";
					break;
				case ast::unary_operation_type::logical_not:
					_output << "!";
					break;
			}
			node.target().accept(*this);
		}

		void pretty_printer::visit(object_instantiation &node)
		{
			_output << "new " << node.class_name() << "()";
		}

		void pretty_printer::visit(array_instantiation &node)
		{
			_output << "new " << _type_name(node.array_type().name());
			_output << "[";
			node.extent().accept(*this);
			_output << "]";
			for (size_t i = 1; i < node.array_type().rank(); i++)
				_output << "[]";
		}

		void pretty_printer::visit(array_access &node)
		{
			node.target().accept(*this);
			_output << "[";
			node.index().accept(*this);
			_output << "]";
		}

		void pretty_printer::visit(variable_access &node)
		{
			if (node.target() != nullptr) {
				node.target()->accept(*this);
				_output << ".";
			}
			_output << node.name();
		}

		void pretty_printer::visit(method_invocation &node)
		{
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

		void pretty_printer::visit(this_ref &node)
		{
			node.accept(*this);
		}

		void pretty_printer::visit(boolean_constant &node)
		{
			_output << (node.value() ? "true" : "false");
		}

		void pretty_printer::visit(integer_constant &node)
		{
			_output << node.literal();
		}

		void pretty_printer::visit(null_constant &node)
		{
			(void)node;
			_output << "null";
		}

		void pretty_printer::visit(local_variable_statement &node)
		{
			node.declaration().accept(*this);
			if (node.initial_value() != nullptr) {
				_output << " = ";
				node.initial_value()->accept(*this);
			}
		}

		void pretty_printer::visit(expression_statement &node)
		{
			_output << _indentation();
			node.inner_expression().accept(*this);
			_output << ";\n";
		}

		void pretty_printer::visit(block &node)
		{
			_output << _indentation() << "{\n";

			_indentation_level++;
			for (size_t i = 0; i < node.body().size(); i++) {
				node.body()[i]->accept(*this);
			}
			_indentation_level--;

			_output << _indentation() << "}\n";
		}

		void pretty_printer::visit(if_statement &node)
		{
			_output << _indentation() << "if (";
			node.condition().accept(*this);
			_output << ") {\n";

			_indentation_level++;
			node.then_statement().accept(*this);
			_indentation_level--;

			if (node.else_statement() == nullptr) {
				_output << _indentation() << "}\n";
			} else {
				_output << _indentation() << "} else {\n";

				_indentation_level++;
				node.else_statement();
				_indentation_level--;

				_output << _indentation() << "}\n";
			}
		}

		void pretty_printer::visit(while_statement &node)
		{
			_output << _indentation() << "while (";
			node.condition().accept(*this);
			_output << ") {\n";

			_indentation_level++;
			node.body().accept(*this);
			_indentation_level--;

			_output << _indentation() << "}\n";
		}

		void pretty_printer::visit(return_statement &node)
		{
			if (node.value() == nullptr) {
				_output << _indentation() << "return;";
			} else {
				_output << _indentation() << "return ";
				node.value()->accept(*this);
				_output << ";";
			}
		}

		void pretty_printer::visit(empty_statement &node)
		{
			(void)node;
			_output << _indentation() << ";\n";
		}

		void pretty_printer::visit(main_method &node)
		{
			_output << "public static void " << node.name() << "(String[] args) {\n";
			node.body().accept(*this);
			_output << "}\n";
		}

		void pretty_printer::visit(method &node)
		{
			_output <<_indentation() << "public ";
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

			// don't use accept() on nody.body(), because of implicitly
			// added curly braces
			for (size_t i = 0; i < node.body().body().size(); i++) {
				node.body().body()[i]->accept(*this);
			}

			_indentation_level--;
			_output << _indentation() << "}\n";
		}

		void pretty_printer::visit(class_declaration &node)
		{
			_output << _indentation() << "class " << node.name() << " {\n";
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
			_output << _indentation() << "}\n";
		}

		void pretty_printer::visit(program &node)
		{
			for (size_t i = 0; i < node.classes().size(); i++) {
				node.classes()[i]->accept(*this);
			}
		}

		std::string pretty_printer::_indentation() {
			return std::string(_indentation_level, '\t');
		}

		std::string pretty_printer::_type_name(const ast::type_name& type) {
			if (auto p = boost::get<ast::primitive_type>(&type)) {
				switch (*p) {
					case ast::primitive_type::type_int:
						return "int";
					case ast::primitive_type::type_boolean:
						return "bool";
					case ast::primitive_type::type_void:
						return "void";
				}
			} else if (auto p = boost::get<symbol>(&type)) {
				return std::string(p->c_str());
			}

			assert(false);
			return nullptr;
		}
	}
}
