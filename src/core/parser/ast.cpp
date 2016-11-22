#include "parser/ast.hpp"

#include <algorithm>

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

		void visitor::visit_method(const method& node)
		{
			visit_node(node);
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
			visit_method(node);
		}

		void visitor::visit(const instance_method& node)
		{
			visit_method(node);
		}

		void visitor::visit(const class_declaration& node)
		{
			visit_node(node);
		}

		void visitor::visit(const program& node)
		{
			visit_node(node);
		}


		method_invocation::method_invocation(std::unique_ptr<expression> target,
											 symbol name,
											 std::vector<std::unique_ptr<expression>> arguments)
				: _target{std::move(target)}, _name{name}
				, _arguments{std::move(arguments)}
		{
			assert(!_name.empty());
			assert(std::all_of(_arguments.begin(), _arguments.end(), [](auto&& el){ return !!el; }));
		}

		block::block(std::vector<std::unique_ptr<block_statement>> statements)
				: _body{std::move(statements)}
		{
			assert(std::all_of(_body.begin(), _body.end(), [](auto&& el){ return !!el; }));
		}

		method::method(symbol name, std::unique_ptr<type> return_type,
					   std::vector<std::unique_ptr<var_decl>> parameters,
					   std::unique_ptr<block> body)
				: _name{name}, _return_type{std::move(return_type)},
				  _parameters{std::move(parameters)}, _body{std::move(body)}
		{
			assert(!_name.empty());
			assert(_return_type);
			assert(std::all_of(_parameters.begin(), _parameters.end(), [](auto&& el) { return !!el; }));
			assert(_body);
		}

		class_declaration::class_declaration(symbol name,
											 std::vector<std::unique_ptr<var_decl>> fields,
											 std::vector<std::unique_ptr<instance_method>> methods,
											 std::vector<std::unique_ptr<main_method>> main_methods)
				: _name{name}, _fields{std::move(fields)},
				  _methods{std::move(methods)},
				  _main_methods{std::move(main_methods)}
		{
			assert(!_name.empty());
			assert(std::all_of(_fields.begin(), _fields.end(), [](auto&& el){ return !!el; }));
			assert(std::all_of(_methods.begin(), _methods.end(), [](auto&& el){ return !!el; }));
			assert(std::all_of(_main_methods.begin(), _main_methods.end(), [](auto&& el){ return !!el; }));
			// sort to enable binary search for symbol in find_*
			const auto comparator = symbol_comparator{};
			std::sort(_fields.begin(), _fields.end(), [comparator](const auto& f1, const auto& f2) {
				return comparator(f1->name(), f2->name());
			});
			std::sort(_methods.begin(), _methods.end(), [comparator](const auto& m1, const auto& m2) {
				return comparator(m1->name(), m2->name());
			});
		}

		std::pair<const std::unique_ptr<var_decl>*, const std::unique_ptr<var_decl>*>
		class_declaration::find_fields(symbol name) const noexcept
		{
			struct {
				const symbol_comparator sym_cmp{};

				bool operator()(const std::unique_ptr<var_decl>& f,
								const symbol& sym) {
					return sym_cmp(f->name(), sym);
				}

				bool operator()(const symbol& sym,
								const std::unique_ptr<var_decl>& f) {
					return sym_cmp(sym, f->name());
				}
			} comparator;
			return std::equal_range(
					_fields.data(), _fields.data() + _fields.size(), name, comparator
			);
		}

		std::pair<const std::unique_ptr<instance_method>*, const std::unique_ptr<instance_method>*>
		class_declaration::find_instance_methods(symbol name) const noexcept
		{
			struct {
				const symbol_comparator sym_cmp{};

				bool operator()(const std::unique_ptr<instance_method>& m,
								const symbol sym) {
					return sym_cmp(m->name(), sym);
				}

				bool operator()(const symbol sym,
								const std::unique_ptr<instance_method>& m) {
					return sym_cmp(sym, m->name());
				}
			} comparator;
			return std::equal_range(
					_methods.data(), _methods.data() + _methods.size(), name, comparator
			);
		}

		program::program(std::vector<std::unique_ptr<class_declaration>> classes)
				: _classes{std::move(classes)}
		{
			assert(std::all_of(_classes.begin(), _classes.end(), [](auto&& el){ return !!el; }));
		}
	}
}
