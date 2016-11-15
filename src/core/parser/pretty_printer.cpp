#include "parser/pretty_printer.hpp"

#include <algorithm>
#include <cstring>
#include <iterator>
#include <utility>
#include <vector>

#include "exceptions.hpp"

using namespace std::string_literals;

namespace minijava
{
	namespace /* anonymous */
	{

		// TODO: This is copy-pasta from `/tests/testaux/syntaxgen.tpp`.  Maybe
		// we should put it into a utility header.  Maybe I should suibmit a
		// proposal to add this to the standard library.  Maybe I should sit by
		// a lake.

		template <typename T>
		class restore_finally final
		{
		public:

			restore_finally() noexcept = default;

			restore_finally(T& dest, const T update) noexcept
				: _destp{&dest}, _prev{dest}
			{
				dest = update;
			}

			~restore_finally() noexcept
			{
				if (_destp != nullptr) {
					*_destp = _prev;
				}
			}

			restore_finally(const restore_finally&) = delete;

			restore_finally(restore_finally&& other) noexcept : restore_finally{}
			{
				swap(*this, other);
			}

			restore_finally& operator=(const restore_finally&) = delete;

			restore_finally& operator=(restore_finally&& other) noexcept
			{
				restore_finally temp{};
				swap(*this, temp);
				swap(*this, other);
				return *this;
			}

			friend void swap(restore_finally& lhs, restore_finally& rhs) noexcept
			{
				std::swap(lhs._destp, rhs._destp);
				std::swap(lhs._prev, rhs._prev);
			}

		private:

			T * _destp{};
			T _prev{};

		};  // class restore_finally

		template <typename T>
		restore_finally<T> make_guard(T& dest, const T update) noexcept
		{
			return restore_finally<T>{dest, update};
		}

		template <typename T>
		restore_finally<T> make_guard_incr(T& dest) noexcept
		{
			return restore_finally<T>{dest, dest + T{1}};
		}

		template <typename FuncT>
		void repeat(const std::size_t start, const std::size_t stop, FuncT&& func)
		{
			for (auto i = start; i < stop; ++i) {
				func();
			}
		}

		template <typename FuncT>
		void repeat(const std::size_t n, FuncT&& func)
		{
			repeat(std::size_t{0}, n, std::forward<FuncT>(func));
		}

		template <typename InIterT, typename UnaryFuncT, typename GlueFuncT>
		void for_each_glue(const InIterT first, const InIterT last, UnaryFuncT&& func, GlueFuncT&& glue)
		{
			if (first != last) {
				auto current = first;
				func(*current);
				++current;
				while (current != last) {
					glue();
					func(*current);
					++current;
				}
			}
		}

		template <typename ContainerT, typename UnaryFuncT, typename GlueFuncT>
		void for_each_glue_c(ContainerT&& c, UnaryFuncT&& func, GlueFuncT&& glue)
		{
			for_each_glue(
				std::begin(c),
				std::end(c),
				std::forward<UnaryFuncT>(func),
				std::forward<GlueFuncT>(glue)
			);
		}

	}  // namespace /* anonymous */

	namespace ast
	{

		void pretty_printer::visit(type& node)
		{
			_output << _type_name(node.name());
			repeat(node.rank(), [this](){ _output << "[]"; });
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

		void pretty_printer::visit(binary_expression& node)
		{
			const auto parens = _print_expression_parens;
			const auto pep_guard = make_guard(_print_expression_parens, true);
			if (parens) {
				_output << "(";
			}
			node.lhs().accept(*this);
			switch (node.type()) {
				case ast::binary_operation_type::assign:
					_output << " = ";
					break;
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
			const auto parens = _print_expression_parens;
			const auto pep_guard = make_guard(_print_expression_parens, true);
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
			const auto parens = _print_expression_parens;
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
			const auto parens = _print_expression_parens;
			if (parens) {
				_output << "(";
			}
			_output << "new " << _type_name(node.array_type().name());
			_output << "[";
			{
				const auto pep_guard = make_guard(_print_expression_parens, false);
				node.extent().accept(*this);
			}
			_output << "]";
			repeat(1, node.array_type().rank(), [this](){ _output << "[]"; });
			if (parens) {
				_output << ")";
			}
		}

		void pretty_printer::visit(array_access& node)
		{
			const auto parens = _print_expression_parens;
			const auto pep_guard = make_guard(_print_expression_parens, true);
			if (parens) {
				_output << "(";
			}
			node.target().accept(*this);
			_output << "[";
			{
				const auto inner_pep_guard = make_guard(_print_expression_parens, false);
				node.index().accept(*this);
			}
			_output << "]";
			if (parens) {
				_output << ")";
			}
		}

		void pretty_printer::visit(variable_access& node)
		{
			const auto parens = (_print_expression_parens && node.target());
			const auto pep_guard = make_guard(_print_expression_parens, true);
			if (parens) {
				_output << "(";
			}
			if (node.target()) {
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
			const auto parens = _print_expression_parens;
			const auto pep_guard = make_guard(_print_expression_parens, true);
			if (parens) {
				_output << "(";
			}
			if (node.target()) {
				node.target()->accept(*this);
				_output << ".";
			}
			_output << node.name() << "(";
			{
				const auto inner_pep_guard = make_guard(_print_expression_parens, false);
				for_each_glue_c(
					node.arguments(),
					[this](auto&& arg){ arg->accept(*this); },
					[this](){ _output << ", "; }
				);
			}
			_output << ")";
			if (parens) {
				_output << ")";
			}
		}

		void pretty_printer::visit(this_ref& /* node */)
		{
			_output << "this";
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
				{
					const auto pep_guard = make_guard(_print_expression_parens, false);
					node.initial_value()->accept(*this);
				}
			}
			_output << ";\n";
		}

		void pretty_printer::visit(expression_statement& node)
		{
			_start_block_statement();
			_print("");
			{
				const auto pep_guard = make_guard(_print_expression_parens, false);
				node.inner_expression().accept(*this);
			}
			_output << ";\n";
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
					return !std::all_of(
							std::begin(p->body()),
							std::end(p->body()),
					        [](const std::unique_ptr<block_statement>& s)
									{ return is_empty_statement(s.get()); }
					);
				}
				return false;
			}
		}

		void pretty_printer::visit(block& node)
		{
			const auto is_conditional = _start_if || _start_else;
			const auto is_empty = !is_nonempty_block(&node);

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
			{
				const auto il_guard = make_guard_incr(_indentation_level);
				std::for_each(std::begin(node.body()), std::end(node.body()),
				              [this](auto&& bs){ bs->accept(*this); });
			}
			_print("}");
			if (!is_conditional) {
				_output << '\n';
			}
		}

		void pretty_printer::visit(if_statement& node)
		{
			const auto then_is_block = is_nonempty_block(&node.then_statement());
			const auto else_is_block = is_nonempty_block(node.else_statement());
			const auto else_is_chain = is_if_statement(node.else_statement());

			if (_start_else) {
				_output << " if (";
			} else {
				if (_start_if || _start_loop) {
					_output << '\n';
				}
				_print("if (");
			}
			_start_if = _start_else = _start_loop = false;
			{
				const auto pep_guard = make_guard(_print_expression_parens, false);
				node.condition().accept(*this);
			}
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
			const auto body_is_block = is_nonempty_block(&node.body());

			_start_block_statement();

			_print("while (");
			{
				const auto pep_guard = make_guard(_print_expression_parens, false);
				node.condition().accept(*this);
			}
			_output << ")";
			if (!body_is_block) {
				_indentation_level++;
			}
			_start_loop = true;
			node.body().accept(*this);
			if (!body_is_block) {
				_indentation_level--;
			}
		}

		void pretty_printer::visit(return_statement& node)
		{
			_start_block_statement();

			if (node.value() == nullptr) {
				_print("return;");
			} else {
				_print("return ");
				{
					const auto pep_guard = make_guard(_print_expression_parens, false);
					node.value()->accept(*this);
				}
				_output << ';';
			}
			_output << '\n';
		}

		void pretty_printer::visit(empty_statement&)
		{
			const auto print = _start_if || _start_else || _start_loop;
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
			{
				const auto ip_guard = make_guard(_in_parameters, true);
				for_each_glue_c(
					node.parameters(),
					[this](auto&& prm){ prm->accept(*this); },
					[this](){ _output << ", "; }
				);
			}
			_output << ")";
			_start_method = true;
			node.body().accept(*this);
		}

		void pretty_printer::visit(class_declaration& node)
		{
			if (node.main_methods().empty() && node.methods().empty()
					&& node.fields().empty()) {
				_println("class "s + node.name().c_str() + " { }");
				return;
			}

			using member_pair = std::pair<symbol, ast::node*>;
			const auto ext = [](auto&& p){
				return std::make_pair(p->name(), p.get());
			};
			const auto cmp = [](auto&& p1, auto&& p2){
				return std::strcmp(p1.first.c_str(), p2.first.c_str()) < 0;
			};
			const auto vst = [this](auto&& p){ p.second->accept(*this); };
			_println("class "s + node.name().c_str() + " {");
			{
				const auto on_the_level_guard = make_guard_incr(_indentation_level);
				auto members = std::vector<member_pair>{};
				members.reserve(std::max(
					node.methods().size() + node.main_methods().size(),
					node.fields().size()
				));
				std::transform(
					std::begin(node.methods()), std::end(node.methods()),
					std::back_inserter(members), ext
				);
				std::transform(
					std::begin(node.main_methods()), std::end(node.main_methods()),
					std::back_inserter(members), ext
				);
				std::stable_sort(std::begin(members), std::end(members), cmp);
				std::for_each(std::begin(members), std::end(members), vst);
				members.clear();  // recycle the storage
				const auto in_the_fields_guard = make_guard(_in_fields, true);
				std::transform(
					std::begin(node.fields()), std::end(node.fields()),
					std::back_inserter(members), ext
				);
				std::stable_sort(std::begin(members), std::end(members), cmp);
				std::for_each(std::begin(members), std::end(members), vst);
			}
			_println("}");
		}

		void pretty_printer::visit(program& node)
		{
			for_each_glue_c(
				node.classes(),
				[this](auto&& cls){ cls->accept(*this); },
				[this](){ _output << '\n'; }
			);
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
			if (auto p = boost::get<symbol>(&type)) {
				return p->c_str();
			}
			switch (boost::get<ast::primitive_type>(type)) {
			case ast::primitive_type::type_int:
				return "int";
			case ast::primitive_type::type_boolean:
				return "boolean";
			case ast::primitive_type::type_void:
				return "void";
			}
			MINIJAVA_NOT_REACHED();
		}

	}  // namespace ast

}  // namespace minijava
