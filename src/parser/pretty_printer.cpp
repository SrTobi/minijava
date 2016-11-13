#include "parser/pretty_printer.hpp"

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
			_output << "public ";
			node.var_type().accept(*this);
			_output << node.name();
		}

		void pretty_printer::visit(assignment_expression& node)
		{
			node.lhs().accept(*this);
			_output << " = ";
			node.rhs().accept(*this);
		}

		void pretty_printer::visit(binary_expression& node)
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

		void pretty_printer::visit(unary_expression& node)
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

		void pretty_printer::visit(object_instantiation& node)
		{
			_output << "new " << node.class_name() << "()";
		}

		void pretty_printer::visit(array_instantiation& node)
		{
			_output << "new " << _type_name(node.array_type().name());
			_output << "[";
			node.extent().accept(*this);
			_output << "]";
			for (size_t i = 1; i < node.array_type().rank(); i++)
				_output << "[]";
		}

		void pretty_printer::visit(array_access& node)
		{
			node.target().accept(*this);
			_output << "[";
			node.index().accept(*this);
			_output << "]";
		}

		void pretty_printer::visit(variable_access& node)
		{
			if (node.target() != nullptr) {
				node.target()->accept(*this);
				_output << ".";
			}
			_output << node.name();
		}

		void pretty_printer::visit(method_invocation& node)
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
				node.initial_value()->accept(*this);
			}
		}

		void pretty_printer::visit(expression_statement& node)
		{
			_start_block_statement();

			_print("");
			node.inner_expression().accept(*this);
			_output << ";\n";
		}

		void pretty_printer::visit(block& node)
		{
			bool omit_newline = _start_if || _start_else;

			if (omit_newline || _start_loop || _start_method) {
				_output << " {\n";
				_start_if = _start_else = _start_loop = _start_method = false;
			} else {
				_println("{");
			}
			_indentation_level++;

			for (auto& block_stmt : node.body()) {
				block_stmt->accept(*this);
			}

			_indentation_level--;
			if (omit_newline) {
				_print("}");
			} else {
				_println("}");
			}
		}

		namespace /* anonymous */
		{
			bool is_block(const ast::node* node)
			{
				return dynamic_cast<const ast::block*>(node) != nullptr;
			}

			bool is_if_statement(const ast::node* node)
			{
				return dynamic_cast<const ast::if_statement*>(node) != nullptr;
			}
		}

		void pretty_printer::visit(if_statement& node)
		{
			bool then_is_block = is_block(&node.then_statement());
			bool else_is_block = is_block(node.else_statement());
			bool else_is_chain = is_if_statement(node.else_statement());

			if (_start_else) {
				_output << " if (";
			} else {
				_print("if (");
			}
			node.condition().accept(*this);
			_output << ")";

			if (!then_is_block) {
				_indentation_level++;
			}
			_start_if = true;
			node.then_statement().accept(*this);
			if (!then_is_block) {
				_indentation_level--;
			}

			if (auto stmt = node.else_statement()) {
				if (then_is_block) {
					_output << " else";
				} else {
					_print("else");
				}
				if (!else_is_block && !else_is_chain) {
					_indentation_level++;
				}
				_start_else = true;
				stmt->accept(*this);
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
			node.condition().accept(*this);
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
				node.value()->accept(*this);
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
			for (size_t i = 0; i < node.parameters().size(); i++) {
				if (i > 0) {
					_output << ", ";
				}
				node.parameters()[i]->accept(*this);
			}
			_output << ")";
			_start_method = true;
			node.body().accept(*this);
		}

		void pretty_printer::visit(class_declaration& node)
		{
			_println("class "s + node.name().c_str() + " {");
			_indentation_level++;

			for (auto& field : node.fields()) {
				field->accept(*this);
			}

			for (auto& main_method : node.main_methods()) {
				main_method->accept(*this);
			}

			for (auto& method : node.methods()) {
				method->accept(*this);
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


		// common code for most block statements to handle _start_if/else
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
