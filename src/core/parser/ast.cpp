#include "parser/ast.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>


namespace minijava
{

	namespace /* anonymous */
	{

		template <typename ContainerT>
		auto all_good(ContainerT&& container)
		{
			const auto good = [](auto&& thing){ return bool(thing); };
			return std::all_of(std::begin(container), std::end(container), good);
		}

		template <typename ContainerT>
		void sort_ptrs_by_symbol_name(ContainerT& container)
		{
			const auto name_cmp = [](const auto& lhs, const auto& rhs) {
				const auto comparator = symbol_comparator{};
				return comparator(lhs->name(), rhs->name());
			};
			// TODO: We're using stable sort here because otherwise, the
			// pretty-printed AST will still be non-deterministic in the case
			// of fields or methods with the same name.  A better solution
			// would be to use the token position (which we currently don't
			// have) as a tie breaker in this situation.  We might als use the
			// node ID for this but again, it is not guaranteed to be
			// available.
			std::stable_sort(std::begin(container), std::end(container), name_cmp);
		}

		symbol get_name(const symbol s)
		{
			return s;
		}

		template<typename SmpT>
		auto get_name(SmpT&& smp) -> decltype(symbol{smp->name()})
		{
			return smp->name();
		}

		// TODO: use some kind of array view or iterator to return something
		// TODO: better than a pair of unique_ptrs

		template<typename VecT>
		auto find_in_sorted_vector(const symbol name, const VecT& v) noexcept
		{
			const auto cmp = [](auto&& lhs, auto&& rhs){
				const auto sym_cmp = symbol_comparator{};
				return sym_cmp(get_name(lhs), get_name(rhs));
			};
			return std::equal_range(v.data(), v.data() + v.size(), name, cmp);
		}

		template <typename PairT>
		auto uptr_range_to_ptr_or_null(const PairT range, const char* errmsg)
			-> decltype(range.first->get())
		{
			if (range.first == range.second) {
				return nullptr;
			} else if (range.first + 1 == range.second) {
				return range.first->get();
			} else {
				throw std::out_of_range{errmsg};
			}
		}

	}  // namespace /* anonymous */


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
			assert(all_good(_arguments));
		}

		block::block(std::vector<std::unique_ptr<block_statement>> statements)
				: _body{std::move(statements)}
		{
			assert(all_good(_body));
		}

		method::method(symbol name, std::unique_ptr<type> return_type,
					   std::vector<std::unique_ptr<var_decl>> parameters,
					   std::unique_ptr<block> body)
				: _name{name}, _return_type{std::move(return_type)},
				  _parameters{std::move(parameters)}, _body{std::move(body)}
		{
			assert(!_name.empty());
			assert(_return_type);
			assert(all_good(_parameters));
			assert(_body);
		}

		class_declaration::class_declaration(symbol name,
											 std::vector<std::unique_ptr<var_decl>> fields,
											 std::vector<std::unique_ptr<instance_method>> methods,
											 std::vector<std::unique_ptr<main_method>> main_methods)
				: _name{name},
				  _fields{std::move(fields)},
				  _methods{std::move(methods)},
				  _main_methods{std::move(main_methods)}
		{
			assert(!_name.empty());
			assert(all_good(_fields));
			assert(all_good(_methods));
			assert(all_good(_main_methods));
			sort_ptrs_by_symbol_name(_fields);
			sort_ptrs_by_symbol_name(_methods);
			sort_ptrs_by_symbol_name(_main_methods);
		}

		std::pair<const std::unique_ptr<var_decl>*, const std::unique_ptr<var_decl>*>
		class_declaration::find_fields(symbol name) const noexcept
		{
			return find_in_sorted_vector(name, _fields);
		}

		const var_decl* class_declaration::get_field(const symbol name) const
		{
			return uptr_range_to_ptr_or_null(
				find_fields(name),
				"minijava::ast::class_declaration::get_field"
			);
		}

		std::pair<const std::unique_ptr<instance_method>*, const std::unique_ptr<instance_method>*>
		class_declaration::find_instance_methods(symbol name) const noexcept
		{
			return find_in_sorted_vector(name, _methods);
		}

		const instance_method* class_declaration::get_instance_method(const symbol name) const
		{
			return uptr_range_to_ptr_or_null(
				find_instance_methods(name),
				"minijava::ast::class_declaration::get_instance_method"
			);
		}

		std::pair<const std::unique_ptr<main_method>*, const std::unique_ptr<main_method>*>
		class_declaration::find_main_methods(symbol name) const noexcept
		{
			return find_in_sorted_vector(name, _main_methods);
		}

		const main_method* class_declaration::get_main_method(const symbol name) const
		{
			return uptr_range_to_ptr_or_null(
				find_main_methods(name),
				"minijava::ast::class_declaration::get_main_method"
			);
		}

		program::program(std::vector<std::unique_ptr<class_declaration>> classes)
				: _classes{std::move(classes)}
		{
			assert(all_good(_classes));
			sort_ptrs_by_symbol_name(_classes);
		}

	}  // namespace ast

}  // namespace minijava
