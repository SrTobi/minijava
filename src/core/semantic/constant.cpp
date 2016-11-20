#include "semantic/constant.hpp"

#include <cstdint>
#include <string>

#include "exceptions.hpp"
#include "parser/ast.hpp"
#include "parser/for_each_node.hpp"
#include "semantic/semantic_error.hpp"

// Clang issues a strange warning about the visitor defined below that, to my
// best knowledge, is a false positive.  Kudos for anyone who can provide a
// better fix than disabling the diagnostic.
#ifdef __clang__
#  pragma GCC diagnostic ignored "-Woverloaded-virtual"
#endif

namespace minijava
{

	namespace /* anonymous */
	{

		using const_map = std::unordered_map<std::size_t, ast_int_type>;

		class extractor : public for_each_node
		{
		public:

			extractor(const ast_problem_handler_type * handlerptr)
				: _handlerptr{handlerptr}
			{
			}

			const_map&& get()
			{
				return std::move(_constants);
			}

		protected:

			void visit(const ast::boolean_constant& node) override
			{
				_constants[node.id()] = node.value();
			}

			void visit(const ast::integer_constant& node) override
			{
				using namespace std::string_literals;
				const auto maximum = _is_operand_of_unary_minus(node.id())
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
				_constants[node.id()] = value;
			}

			void visit(const ast::unary_expression& node) override
			{
				const auto tid = node.target().id();
				_id_of_operand_of_last_unary_minus = tid;
				node.target().accept(*this);
				const auto tit = _constants.find(tid);
				if (tit != _constants.end()) {
					const auto tval = tit->second;
					auto res = std::int32_t{};
					switch (node.type()) {
					case ast::unary_operation_type::minus:
						if (!__builtin_sub_overflow(0, tval, &res)) {
							_constants[node.id()] = res;
						} else {
							_maybe_call_handler(node);
						}
						return;
					case ast::unary_operation_type::logical_not:
						_constants[node.id()] = !tval;
						return;
					}
					MINIJAVA_NOT_REACHED();
				}
			}

			void visit(const ast::binary_expression& node) override
			{
				node.lhs().accept(*this);
				node.rhs().accept(*this);
				const auto lit = _constants.find(node.lhs().id());
				const auto rit = _constants.find(node.rhs().id());
				if ((lit != _constants.end()) && (rit != _constants.end())) {
					const auto lval = lit->second;
					const auto rval = rit->second;
					auto res = std::int32_t{};
					switch (node.type()) {
					case ast::binary_operation_type::logical_or:
						_constants[node.id()] = (lval || rval);
						return;
					case ast::binary_operation_type::logical_and:
						_constants[node.id()] = (lval && rval);
						return;
					case ast::binary_operation_type::equal:
						_constants[node.id()] = (lval == rval);
						return;
					case ast::binary_operation_type::not_equal:
						_constants[node.id()] = (lval != rval);
						return;
					case ast::binary_operation_type::less_than:
						_constants[node.id()] = (lval < rval);
						return;
					case ast::binary_operation_type::less_equal:
						_constants[node.id()] = (lval <= rval);
						return;
					case ast::binary_operation_type::greater_equal:
						_constants[node.id()] = (lval >= rval);
						return;
					case ast::binary_operation_type::greater_than:
						_constants[node.id()] = (lval > rval);
						return;
					case ast::binary_operation_type::plus:
						if (!__builtin_add_overflow(lval, rval, &res)) {
							_constants[node.id()] = res;
						} else {
							_maybe_call_handler(node);
						}
						return;
					case ast::binary_operation_type::minus:
						if (!__builtin_sub_overflow(lval, rval, &res)) {
							_constants[node.id()] = res;
						} else {
							_maybe_call_handler(node);
						}
						return;
					case ast::binary_operation_type::multiply:
						if (!__builtin_mul_overflow(lval, rval, &res)) {
							_constants[node.id()] = res;
						} else {
							_maybe_call_handler(node);
						}
						return;
					case ast::binary_operation_type::divide:
						if (rval != 0) {
							_constants[node.id()] = lval / rval;
						} else {
							_maybe_call_handler(node);
						}
						return;
					case ast::binary_operation_type::modulo:
						if (rval != 0) {
							_constants[node.id()] = lval % rval;  // ok since C++11
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

			const_map _constants{};

			std::size_t _id_of_operand_of_last_unary_minus{};

			const ast_problem_handler_type * _handlerptr{};

			bool _is_operand_of_unary_minus(const std::size_t id)
			{
				return id == _id_of_operand_of_last_unary_minus;
			}

			void _maybe_call_handler(const ast::node& node)
			{
				if (_handlerptr != nullptr) {
					(*_handlerptr)(node);
				}
			}

		};  // struct extractor

	}  // namespace /* anonymous */


	const_map extract_constants(const ast::node& ast, const ast_problem_handler_type& handler)
	{
		auto exvis = extractor{&handler};
		ast.accept(exvis);
		return exvis.get();
	}

	const_map extract_constants(const ast::node& ast)
	{
		auto exvis = extractor{nullptr};
		ast.accept(exvis);
		return exvis.get();
	}


}  // namespace minijava
