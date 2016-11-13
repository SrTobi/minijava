#include "parser/pretty_printer.hpp"

#include <map>

#include "exceptions.hpp"

namespace minijava
{
	namespace ast
	{
		using namespace std::string_literals;

		void pretty_printer::visit(type& node)
		{
			_output << _type_name(node.name());

			for (size_t i = 0; i < node.rank(); i++)
				_output << "[]";
		}

		void pretty_printer::visit(var_decl& node)
		{
			if (_in_fields) {
				_print("public ");
			} else if (!_in_parameters) {
				_print("");
			}
			node.var_type().accept(*this);
			_output << " " << node.name();
			if (_in_fields) {
				_output << ";\n";
			}
		}

		void pretty_printer::visit(assignment_expression& node)
		{
			bool parens = _print_expression_parens;
			_print_expression_parens = true; // for nested expressions
			if (parens) {
				_output << "(";
			}
			node.lhs().accept(*this);
			_output << " = ";
			node.rhs().accept(*this);
			if (parens) {
				_output << ")";
			}
		}

		void pretty_printer::visit(binary_expression& node)
		{
			bool parens = _print_expression_parens;
			_print_expression_parens = true; // for nested expressions
			if (parens) {
				_output << "(";
			}
			node.lhs().accept(*this);
			switch (node.type()) {
				case ast::binary_operation_type::plus:
					_output << " + ";
					break;
				case ast::binary_operation_type::minus:
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
				case ast::binary_operation_type::greater_than:
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
				case ast::binary_operation_type::less_than:
					_output << " < ";
					break;
				case ast::binary_operation_type::less_equal:
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
			if (parens) {
				_output << ")";
			}
		}

		void pretty_printer::visit(unary_expression& node)
		{
			bool parens = _print_expression_parens;
			_print_expression_parens = true; // for nested expressions
			if (parens) {
				_output << "(";
			}
			switch (node.type()) {
				case ast::unary_operation_type::minus:
					_output << "-";
					break;
				case ast::unary_operation_type::logical_not:
					_output << "!";
					break;
			}
			node.target().accept(*this);
			if (parens) {
				_output << ")";
			}
		}

		void pretty_printer::visit(object_instantiation& node)
		{
			bool parens = _print_expression_parens;
			if (parens) {
				_output << "(";
			}
			_output << "new " << node.class_name() << "()";
			if (parens) {
				_output << ")";
			}
		}

		void pretty_printer::visit(array_instantiation& node)
		{
			bool parens = _print_expression_parens;
			if (parens) {
				_output << "(";
			}
			_output << "new " << _type_name(node.array_type().name());
			_output << "[";
			_print_expression_parens = false;
			node.extent().accept(*this);
			_print_expression_parens = true;
			_output << "]";
			for (size_t i = 1; i < node.array_type().rank(); i++) {
				_output << "[]";
			}
			if (parens) {
				_output << ")";
			}
		}

		void pretty_printer::visit(array_access& node)
		{
			bool parens = _print_expression_parens;
			_print_expression_parens = true; // for nested expressions
			if (parens) {
				_output << "(";
			}
			node.target().accept(*this);
			_output << "[";
			_print_expression_parens = false;
			node.index().accept(*this);
			_print_expression_parens = true;
			_output << "]";
			if (parens) {
				_output << ")";
			}
		}

		void pretty_printer::visit(variable_access& node)
		{
			bool parens = _print_expression_parens && node.target() != nullptr;
			_print_expression_parens = true; // for nested expressions
			if (parens) {
				_output << "(";
			}
			if (node.target() != nullptr) {
				node.target()->accept(*this);
				_output << ".";
			}
			_output << node.name();
			if (parens) {
				_output << ")";
			}
		}

		void pretty_printer::visit(method_invocation& node)
		{
			bool parens = _print_expression_parens;
			_print_expression_parens = true; // for nested expressions
			if (parens) {
				_output << "(";
			}
			if (node.target() != nullptr) {
				node.target()->accept(*this);
				_output << ".";
			}
			_output << node.name() << "(";
			for (size_t i = 0; i < node.arguments().size(); i++) {
				if (i > 0) {
					_output << ", ";
				}
				_print_expression_parens = false;
				node.arguments()[i]->accept(*this);
				_print_expression_parens = true;
			}
			_output << ")";
			if (parens) {
				_output << ")";
			}
		}

		void pretty_printer::visit(this_ref& node)
		{
			node.accept(*this);
		}

		void pretty_printer::visit(boolean_constant& node)
		{
			_output << (node.value() ? "true" : "false");
		}

		void pretty_printer::visit(integer_constant& node)
		{
			_output << node.literal();
		}

		void pretty_printer::visit(null_constant&)
		{
			_output << "null";
		}

		void pretty_printer::visit(local_variable_statement& node)
		{
			_start_block_statement();

			node.declaration().accept(*this);
			if (node.initial_value() != nullptr) {
				_output << " = ";
				_print_expression_parens = false;
				node.initial_value()->accept(*this);
				_print_expression_parens = true;
			}
			_output << ";\n";
		}

		void pretty_printer::visit(expression_statement& node)
		{
			_start_block_statement();

			_print("");
			_print_expression_parens = false;
			node.inner_expression().accept(*this);
			_print_expression_parens = true;
			_output << ";\n";
		}

		void pretty_printer::visit(block& node)
		{
			bool is_conditional = _start_if || _start_else;
			bool is_empty = node.body().empty();

			if (is_conditional || _start_loop || _start_method) {
				_output << " {";
				_start_if = _start_else = _start_loop = _start_method = false;
			} else {
				_print("{");
			}
			if (is_empty) {
				_output << " }\n";
				return;
			}
			_output << '\n';

			_indentation_level++;
			for (auto& block_stmt : node.body()) {
				block_stmt->accept(*this);
			}
			_indentation_level--;

			if (is_conditional) {
				_print("}");
			} else {
				_println("}");
			}
		}

		namespace /* anonymous */
		{
			bool is_empty_statement(const ast::node* node)
			{
				return dynamic_cast<const ast::empty_statement*>(node) != nullptr;
			}

			bool is_if_statement(const ast::node* node)
			{
				return dynamic_cast<const ast::if_statement*>(node) != nullptr;
			}

			bool is_nonempty_block(const ast::node *node)
			{
				if (const auto p = dynamic_cast<const ast::block*>(node)) {
					return !p->body().empty();
				}
				return false;
			}
		}

		void pretty_printer::visit(if_statement& node)
		{
			bool then_is_block = is_nonempty_block(&node.then_statement());
			bool else_is_block = is_nonempty_block(node.else_statement());
			bool else_is_chain = is_if_statement(node.else_statement());

			if (_start_else) {
				_output << " if (";
			} else {
				_print("if (");
			}
			_print_expression_parens = false;
			node.condition().accept(*this);
			_print_expression_parens = true;
			_output << ")";

			if (!then_is_block) {
				_indentation_level++;
			}
			_start_if = true;
			node.then_statement().accept(*this);
			if (!then_is_block) {
				_indentation_level--;
			}

			auto else_stmt = node.else_statement();
			if (else_stmt && !is_empty_statement(else_stmt)) {
				if (then_is_block) {
					_output << " else";
				} else {
					_print("else");
				}
				if (!else_is_block && !else_is_chain) {
					_indentation_level++;
				}
				_start_else = true;
				else_stmt->accept(*this);
				if (!else_is_block && !else_is_chain) {
					_indentation_level--;
				} else if (else_is_block) {
					_output << '\n';
				}
			} else if (then_is_block) {
				_output << '\n';
			}
		}

		void pretty_printer::visit(while_statement& node)
		{
			_start_block_statement();

			_print("while (");
			_print_expression_parens = false;
			node.condition().accept(*this);
			_print_expression_parens = true;
			_output << ")\n";
			_start_loop = true;
			_indentation_level++;
			node.body().accept(*this);
			_indentation_level--;
		}

		void pretty_printer::visit(return_statement& node)
		{
			_start_block_statement();

			if (node.value() == nullptr) {
				_print("return;");
			} else {
				_print("return ");
				_print_expression_parens = false;
				node.value()->accept(*this);
				_print_expression_parens = true;
				_output << ';';
			}
			_output << '\n';
		}

		void pretty_printer::visit(empty_statement&)
		{
			bool print = _start_if || _start_else || _start_loop;

			_start_block_statement();

			if (print) {
				_println(";");
			}
		}

		void pretty_printer::visit(main_method& node)
		{
			_print("public static void "s + node.name().c_str() + "(String[] " + node.argname().c_str() + ")");
			_start_method = true;
			node.body().accept(*this);
		}

		void pretty_printer::visit(method& node)
		{
			_print("public ");
			node.return_type().accept(*this);
			_output << " " << node.name() << "(";
			_in_parameters = true;
			for (size_t i = 0; i < node.parameters().size(); i++) {
				if (i > 0) {
					_output << ", ";
				}
				node.parameters()[i]->accept(*this);
			}
			_in_parameters = false;
			_output << ")";
			_start_method = true;
			node.body().accept(*this);
		}

		void pretty_printer::visit(class_declaration& node)
		{
			_println("class "s + node.name().c_str() + " {");
			_indentation_level++;

			// sort methods together by inserting into sorted map
			auto sorted_method_map = std::map<std::string,
					std::pair<bool, void*>>{};
			for (auto& method : node.methods()) {
				sorted_method_map.insert(
						std::make_pair(method->name().c_str(),
						               std::make_pair(false, method.get()))
				);
			}
			for (auto& method : node.main_methods()) {
				sorted_method_map.insert(
						std::make_pair(method->name().c_str(),
						               std::make_pair(true, method.get()))
				);
			}
			for (auto& el : sorted_method_map) {
				bool is_main_method = el.second.first;
				void* p = el.second.second;

				if (is_main_method) {
					(reinterpret_cast<main_method*>(p))->accept(*this);
				} else {
					(reinterpret_cast<method*>(p))->accept(*this);
				}
			}

			_in_fields = true;
			// sort fields by inserting into sorted map
			auto sorted_field_map = std::map<std::string, var_decl*>{};
			for (auto& field : node.fields()) {
				sorted_field_map.insert(
						std::make_pair(field->name().c_str(), field.get())
				);
			}
			for (auto& el : sorted_field_map) {
				el.second->accept(*this);
			}

			_indentation_level--;
			_println("}");
		}

		void pretty_printer::visit(program& node)
		{
			for (auto& clazz : node.classes()) {
				clazz->accept(*this);
			}
		}


		// common code for most block statements to handle _start_if/else/loop
		void pretty_printer::_start_block_statement()
		{
			if (_start_if || _start_else || _start_loop) {
				_output << '\n';
				_start_if = _start_else = _start_loop = false;
			}
		}


		void pretty_printer::_print(const std::string& line)
		{
			_output << std::string(_indentation_level, '\t') << line;
		}

		void pretty_printer::_println(const std::string& line)
		{
			_print(line);
			_output << '\n';
		}

		std::string pretty_printer::_type_name(const ast::type_name& type)
		{
			if (auto p = boost::get<ast::primitive_type>(&type)) {
				switch (*p) {
					case ast::primitive_type::type_int:
						return "int";
					case ast::primitive_type::type_boolean:
						return "boolean";
					case ast::primitive_type::type_void:
						return "void";
				}
			} else if (auto p = boost::get<symbol>(&type)) {
				return std::string(p->c_str());
			}

			MINIJAVA_NOT_REACHED();
		}
	}
}
