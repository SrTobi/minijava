#include "semantic/constant.hpp"

#include <cstdint>
#include <cstring>
#include <sstream>
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

			std::int32_t wrap(const std::int64_t value) noexcept
			{
				std::int32_t result;
				std::memcpy(&result, &value, sizeof(result));
				return result;
			}

			[[noreturn, gnu::cold]] void
			throw_literal_overflow(const ast::integer_constant& node)
			{
				std::ostringstream oss{};
				static_assert((std::uint32_t{1} << 31) == UINT32_C(2147483648), "");
				if (node.literal() == "2147483648") {
					assert(!node.negative());
					oss << "The integer literal " << node.literal()
						<< " is only allowed as immediate operand of the"
						<< " unary minus operator";
				} else if (node.negative()) {
					oss << "Value of integer literal too negative for 32 bit two's complement integer: "
						<< node.literal();
				} else {
					oss << "Value of integer literal too large for 32 bit two's complement integer: "
						<< node.literal();
				}
				throw semantic_error{oss.str(), node.position()};
			}


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

				// Our conversion logic only works with character sets that are
				// reasonably close to ASCII.  Since this is not guaranteed by
				// the standard, let's make sure we at least fail loudly on
				// <del>such awkward platforms</del><ins>IBM mainframes</ins>.

				static_assert('1' == '0' + 1, "");
				static_assert('2' == '0' + 2, "");
				static_assert('3' == '0' + 3, "");
				static_assert('4' == '0' + 4, "");
				static_assert('5' == '0' + 5, "");
				static_assert('6' == '0' + 6, "");
				static_assert('7' == '0' + 7, "");
				static_assert('8' == '0' + 8, "");
				static_assert('9' == '0' + 9, "");

				void visit(const ast::integer_constant& node) override
				{
					using working_type = decltype(0 + std::int_fast64_t{});
					const auto maximum = node.negative()
						? (working_type{1} << 31)
						: (working_type{1} << 31) - 1;
					const auto base = working_type{10};
					const auto oh = working_type{'0'};
					auto value = working_type{};
					for (const auto c : node.literal()) {
						const auto digit = working_type{c} - oh;
						value = base * value + digit;
						if (value > maximum) {
							throw_literal_overflow(node);
						}
					}
					if (node.negative()) {
						value = -value;
					}
					_constants[node] = static_cast<std::int32_t>(value);
				}

				void visit(const ast::unary_expression& node) override
				{
					const auto target_ptr = &node.target();
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
							_constants[node] = wrap(std::int64_t{lval} + std::int64_t{rval});
							return;
						case ast::binary_operation_type::minus:
							_constants[node] = wrap(std::int64_t{lval} - std::int64_t{rval});
							return;
						case ast::binary_operation_type::multiply:
							_constants[node] = wrap(std::int64_t{lval} * std::int64_t{rval});
							return;
						case ast::binary_operation_type::divide:
							if (rval == 0) {
								_maybe_call_handler(node);
							} else {
								_constants[node] = wrap(std::int64_t{lval} / std::int64_t{rval});
							}
							return;
						case ast::binary_operation_type::modulo:
							if (rval == 0) {
								_maybe_call_handler(node);
							} else {
								// C++11 changed the formerly
								// implementation-defined behavior of the
								// modulus operator to require the semantics
								// specified by the JLS.
								_constants[node] = wrap(std::int64_t{lval} % std::int64_t{rval});
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

				const ast_problem_handler_type * _handlerptr{};

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
