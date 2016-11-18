#include "parser/ast.hpp"

namespace minijava
{
	namespace ast
	{
		void visitor::visit_node(const node&)
		{
		}

		void visitor::visit_expression(const expression& node)
		{
			visit_node(node);
		}

		void visitor::visit_constant(const constant& node)
		{
			visit_expression(node);
		}

		void visitor::visit_block_statement(const block_statement& node)
		{
			visit_node(node);
		}

		void visitor::visit_statement(const statement& node)
		{
			visit_block_statement(node);
		}

		void visitor::visit(const type& node)
		{
			visit_node(node);
		}

		void visitor::visit(const var_decl& node)
		{
			visit_node(node);
		}

		void visitor::visit(const binary_expression& node)
		{
			visit_expression(node);
		}

		void visitor::visit(const unary_expression& node)
		{
			visit_expression(node);
		}

		void visitor::visit(const object_instantiation& node)
		{
			visit_expression(node);
		}

		void visitor::visit(const array_instantiation& node)
		{
			visit_expression(node);
		}

		void visitor::visit(const array_access& node)
		{
			visit_expression(node);
		}

		void visitor::visit(const variable_access& node)
		{
			visit_expression(node);
		}

		void visitor::visit(const method_invocation& node)
		{
			visit_expression(node);
		}

		void visitor::visit(const this_ref& node)
		{
			visit_expression(node);
		}

		void visitor::visit(const boolean_constant& node)
		{
			visit_constant(node);
		}

		void visitor::visit(const integer_constant& node)
		{
			visit_constant(node);
		}

		void visitor::visit(const null_constant& node)
		{
			visit_constant(node);
		}

		void visitor::visit(const local_variable_statement& node)
		{
			visit_block_statement(node);
		}


		void visitor::visit(const expression_statement& node)
		{
			visit_statement(node);
		}

		void visitor::visit(const block& node)
		{
			visit_statement(node);
		}

		void visitor::visit(const if_statement& node)
		{
			visit_statement(node);
		}

		void visitor::visit(const while_statement& node)
		{
			visit_statement(node);
		}

		void visitor::visit(const return_statement& node)
		{
			visit_statement(node);
		}

		void visitor::visit(const empty_statement& node)
		{
			visit_statement(node);
		}

		void visitor::visit(const main_method& node)
		{
			visit(static_cast<const method&>(node));
		}

		void visitor::visit(const method& node)
		{
			visit_node(node);
		}

		void visitor::visit(const class_declaration& node)
		{
			visit_node(node);
		}

		void visitor::visit(const program& node)
		{
			visit_node(node);
		}
	}
}
