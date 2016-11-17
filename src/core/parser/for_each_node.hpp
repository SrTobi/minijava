/**
 * @file for_each_node.hpp
 *
 * @brief
 *     Skeleton for an AST visitor that treats traverses an entire AST.
 *
 */

#pragma once

#include "parser/ast.hpp"


namespace minijava
{

	/**
	 * @brief
	 *     An AST visitor that visits each node in an AST once.
	 *
	 * The order in which nodes are visited is unspecified.
	 *
	 * In order to perform an action on each node of an AST, derive from this
	 * `class` and `override` the `visit_node` function.  You can also override
	 * other member functions if you need to to special actions on some nodes.
	 * It is then your responsibility, hovever, to make sure that all child
	 * nodes are still visited.
	 *
	 */
	struct for_each_node : ast::visitor
	{

		void visit(const ast::var_decl& node) override
		{
			visit_node(node);
			node.var_type().accept(*this);
		}

		void visit(const ast::binary_expression& node) override
		{
			visit_expression(node);
			node.lhs().accept(*this);
			node.rhs().accept(*this);
		}

		void visit(const ast::unary_expression& node) override
		{
			visit_expression(node);
			node.target().accept(*this);
		}

		void visit(const ast::array_instantiation& node) override
		{
			visit_expression(node);
			node.array_type().accept(*this);
			node.extent().accept(*this);
		}

		void visit(const ast::array_access& node) override
		{
			visit_expression(node);
			node.target().accept(*this);
			node.index().accept(*this);
		}

		void visit(const ast::variable_access& node) override
		{
			visit_expression(node);
			if (const auto p = node.target()) {
				p->accept(*this);
			}
		}

		void visit(const ast::method_invocation& node) override
		{
			visit_expression(node);
			if (const auto p = node.target()) {
				p->accept(*this);
			}
			for (auto&& n : node.arguments()) {
				n->accept(*this);
			}
		}

		void visit(const ast::local_variable_statement& node) override
		{
			visit_block_statement(node);
			node.declaration().accept(*this);
			if (const auto p = node.initial_value()) {
				p->accept(*this);
			}
		}

		void visit(const ast::expression_statement& node) override
		{
			visit_statement(node);
			node.inner_expression().accept(*this);
		}

		void visit(const ast::block& node) override
		{
			visit_statement(node);
			for (auto&& n : node.body()) {
				n->accept(*this);
			}
		}

		void visit(const ast::if_statement& node) override
		{
			visit_statement(node);
			node.condition().accept(*this);
			node.then_statement().accept(*this);
			if (const auto p = node.else_statement()) {
				p->accept(*this);
			}
		}

		void visit(const ast::while_statement& node) override
		{
			visit_statement(node);
			node.condition().accept(*this);
			node.body().accept(*this);
		}

		void visit(const ast::return_statement& node) override
		{
			visit_statement(node);
			if (const auto p = node.value()) {
				p->accept(*this);
			}
		}

		void visit(const ast::main_method& node) override
		{
			visit_node(node);
			node.body().accept(*this);
		}

		void visit(const ast::method& node) override
		{
			visit_node(node);
			node.return_type().accept(*this);
			for (auto&& n : node.parameters()) {
				n->accept(*this);
			}
			node.body().accept(*this);
		}

		void visit(const ast::class_declaration& node) override
		{
			visit_node(node);
			for (auto&& n : node.fields()) {
				n->accept(*this);
			}
			for (auto&& n : node.methods()) {
				n->accept(*this);
			}
			for (auto&& n : node.main_methods()) {
				n->accept(*this);
			}
		}

		void visit(const ast::program& node) override
		{
			visit_node(node);
			for (auto&& n : node.classes()) {
				n->accept(*this);
			}
		}

	};  // struct for_each_node

}  // namespace minijava
