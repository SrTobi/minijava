#include "semantic/constant.hpp"

#include <cstdint>
#include <string>

#include "exceptions.hpp"
#include "parser/ast.hpp"
#include "parser/for_each_node.hpp"
#include "semantic/semantic_error.hpp"


namespace minijava
{

	namespace sem
	{

		namespace /* anonymous */
		{

			class extractor : public for_each_node
			{
			public:

				extractor(const ast_problem_handler_type * handlerptr)
					: _handlerptr{handlerptr}
				{
				}

				const_attributes&& get()
				{
					return std::move(_constants);
				}

			protected:

				using for_each_node::visit;

				void visit(const ast::boolean_constant& node) override
				{
					_constants[node] = node.value();
				}

				void visit(const ast::integer_constant& node) override
				{
					using namespace std::string_literals;
					const auto maximum = _is_operand_of_unary_minus(node)
						? (ast_int_type{1} << 31)
						: (ast_int_type{1} << 31) - 1;
					const auto base = ast_int_type{10};
					const auto oh = ast_int_type{'0'};
					auto value = ast_int_type{};
					for (const auto c : node.literal()) {
						const auto digit = ast_int_type{c} - oh;
						value = base * value + digit;
						if (value > maximum) {
							throw semantic_error{
								"Value of integer literal too large for 32 bit integer: "s
									+ node.literal().c_str()
							};
						}
					}
					_constants[node] = value;
				}

				void visit(const ast::unary_expression& node) override
				{
					const auto target_ptr = &node.target();
					_operand_of_last_unary_minus = target_ptr;
					node.target().accept(*this);
					const auto tit = _constants.find(target_ptr);
					if (tit != _constants.end()) {
						const auto tval = tit->second;
						auto res = std::int32_t{};
						switch (node.type()) {
						case ast::unary_operation_type::minus:
							if (!__builtin_sub_overflow(0, tval, &res)) {
								_constants[node] = res;
							} else {
								_maybe_call_handler(node);
							}
							return;
						case ast::unary_operation_type::logical_not:
							_constants[node] = !tval;
							return;
						}
						MINIJAVA_NOT_REACHED();
					}
				}

				void visit(const ast::binary_expression& node) override
				{
					node.lhs().accept(*this);
					node.rhs().accept(*this);
					const auto lit = _constants.find(&node.lhs());
					const auto rit = _constants.find(&node.rhs());
					if ((lit != _constants.end()) && (rit != _constants.end())) {
						const auto lval = lit->second;
						const auto rval = rit->second;
						auto res = std::int32_t{};
						switch (node.type()) {
						case ast::binary_operation_type::logical_or:
							_constants[node] = (lval || rval);
							return;
						case ast::binary_operation_type::logical_and:
							_constants[node] = (lval && rval);
							return;
						case ast::binary_operation_type::equal:
							_constants[node] = (lval == rval);
							return;
						case ast::binary_operation_type::not_equal:
							_constants[node] = (lval != rval);
							return;
						case ast::binary_operation_type::less_than:
							_constants[node] = (lval < rval);
							return;
						case ast::binary_operation_type::less_equal:
							_constants[node] = (lval <= rval);
							return;
						case ast::binary_operation_type::greater_equal:
							_constants[node] = (lval >= rval);
							return;
						case ast::binary_operation_type::greater_than:
							_constants[node] = (lval > rval);
							return;
						case ast::binary_operation_type::plus:
							if (!__builtin_add_overflow(lval, rval, &res)) {
								_constants[node] = res;
							} else {
								_maybe_call_handler(node);
							}
							return;
						case ast::binary_operation_type::minus:
							if (!__builtin_sub_overflow(lval, rval, &res)) {
								_constants[node] = res;
							} else {
								_maybe_call_handler(node);
							}
							return;
						case ast::binary_operation_type::multiply:
							if (!__builtin_mul_overflow(lval, rval, &res)) {
								_constants[node] = res;
							} else {
								_maybe_call_handler(node);
							}
							return;
						case ast::binary_operation_type::divide:
							if (rval != 0) {
								_constants[node] = lval / rval;
							} else {
								_maybe_call_handler(node);
							}
							return;
						case ast::binary_operation_type::modulo:
							if (rval != 0) {
								_constants[node] = lval % rval;  // ok since C++11
							} else {
								_maybe_call_handler(node);
							}
							return;
						case ast::binary_operation_type::assign:
							return;  // Not our business
						}
						MINIJAVA_NOT_REACHED();
					}
				}

			private:

				const_attributes _constants{};

				const ast::expression* _operand_of_last_unary_minus{};

				const ast_problem_handler_type * _handlerptr{};

				bool _is_operand_of_unary_minus(const ast::expression& node)
				{
					return (&node == _operand_of_last_unary_minus);
				}

				void _maybe_call_handler(const ast::node& node)
				{
					if (_handlerptr != nullptr) {
						(*_handlerptr)(node);
					}
				}

			};  // struct extractor

		}  // namespace /* anonymous */


		const_attributes extract_constants(const ast::node& ast, const ast_problem_handler_type& handler)
		{
			auto exvis = extractor{&handler};
			ast.accept(exvis);
			return exvis.get();
		}

		const_attributes extract_constants(const ast::node& ast)
		{
			auto exvis = extractor{nullptr};
			ast.accept(exvis);
			return exvis.get();
		}

	}  // namespace sem

}  // namespace minijava
