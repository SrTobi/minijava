#ifndef MINIJAVA_INCLUDED_FROM_PARSER_PARSER_HPP
#error "Never `#include <parser/parser.tpp>` directly; `#include <parser/parser.hpp>` instead."
#endif

#include <algorithm>
#include <cstring>
#include <initializer_list>
#include <stack>

#include "exceptions.hpp"
#include "lexer/token_type.hpp"
#include "lexer/token.hpp"

#include "parser/operator.tpp"

namespace minijava
{

	inline std::size_t syntax_error::line() const noexcept
	{
		return _line;
	}

	inline std::size_t syntax_error::column() const noexcept
	{
		return _column;
	}

	namespace detail
	{

		// `throw`s a `syntax_error` that complains about the unexpected token
		// `pde` when any of the token types in `expected` was expected
		// instead.  `expected` must not be empty and must not contain
		// `pde.type()`.
		[[noreturn]]
		void throw_syntax_error(
			const token& pde,
			std::initializer_list<token_type> expected
		);

		// `throw`s a `syntax_error` that complains about `main` not having
		// the right signature.
		[[noreturn]]
		void throw_syntax_error_main_signature(const token& pde);

		// `throw`s a `syntax_error` that complains about `tt` (which must be a
		// keyword identifying a primitive type) being used in a new object
		// expression that was detected at `pde` which must be a left
		// parenthesis.
		[[noreturn]]
		void throw_syntax_error_new_primitive(const token& pde, token_type tt);

		// Tag type to statically define a set of token types.  It should be
		// asserted that the type list `TTs` is free of duplicates but doing so
		// without a TMP library is too complicated to be worth the effort.
		template<token_type... TTs>
		struct token_type_set
		{
		};

		// The type meta-function `token_type_set_cat` can be used to
		// concatenate two `token_type_set`s.  It is the responsibility of the
		// programmer to make sure that the two sets are disjoint.  The
		// function may be used like this
		//
		//     using set1st = token_type_set<token_type::plus, token_type::minus>;
		//     using set2nd = token_type_set<token_type::multiply, token_type::divides>;
		//     using set3rd = token_type_set_cat<set1st, set2nd>::type;
		//
		// where `set3rd` would now hold the token types for '+' ans '-' as
		// well as for '*' and '/'.   Since this is quite a moutful to type, the
		// heterogenous convenience function `cat` is provided that infers the
		// types and `return`s a value of the result type.  Using the example
		// from above,
		//
		//     constexpr auto union = cat(set1st{}, set2nd{});
		//
		// will create a `union` with `decltype(union)` identical to `set3rd`.
		//
		// If the two arguments are not instantiations of `token_type_set`, the
		// function `cat` will not participate in overload resolution.

		template<typename T1, typename T2>
		struct token_type_set_cat;

		template<token_type... TT1s, token_type... TT2s>
		struct token_type_set_cat<token_type_set<TT1s...>, token_type_set<TT2s...>>
		{
			using type = token_type_set<TT1s..., TT2s...>;
		};

		template<typename T1, typename T2>
		constexpr auto cat(T1, T2) -> typename token_type_set_cat<T1, T2>::type
		{
			using result_t = typename token_type_set_cat<T1, T2>::type;
			return result_t{};
		}


		template<typename InIterT>
		struct parser
		{

			template<token_type... TTs>
			using set_t = token_type_set<TTs...>;

			parser(InIterT first, InIterT /*last*/)
				: it(first)
			{
				token_buffer.push(*it);
			}

			void parse_program()
			{
				while (!current_is(token_type::eof)) {
					expect(token_type::kw_class);
					parse_class_declaration();
				}
			}

			void parse_class_declaration()
			{
				assert(current_is(token_type::kw_class));
				advance();
				consume(token_type::identifier);
				consume(token_type::left_brace);
				while (!current_is(token_type::right_brace)) {
					expect(token_type::kw_public);
					parse_class_member();
				}
				consume(token_type::right_brace);
			}

			void parse_class_member()
			{
				assert(current_is(token_type::kw_public));
				advance();
				expect(cat(set_t<token_type::kw_static>{}, type_first));
				if (current_is(token_type::kw_static)) {
					return parse_main_method();
				}
				// parse field or method
				parse_type();
				consume(token_type::identifier);
				if (consume(token_type::semicolon, token_type::left_paren) == token_type::left_paren) {
					// parse method
					if (current_is(parameters_first)) {
						parse_parameters();
					}
					consume(token_type::right_paren);
					expect(token_type::left_brace);
					parse_block();
				}
			}

			void parse_main_method()
			{
				assert(current_is(token_type::kw_static));
				advance();
				consume(token_type::kw_void);
				consume(token_type::identifier);
				consume(token_type::left_paren);
				expect(token_type::identifier);
				if (std::strcmp(current().lexval().c_str(), "String") != 0) {
					throw_syntax_error_main_signature(current());
				}
				advance();
				consume(token_type::left_bracket);
				consume(token_type::right_bracket);
				consume(token_type::identifier);
				consume(token_type::right_paren);
				expect(block_first);
				return parse_block();
			}

			static constexpr auto parameters_first = set_t<
				/* type_first */
				token_type::kw_int,
				token_type::kw_boolean,
				token_type::kw_void,
				token_type::identifier
			>{};

			static constexpr auto parameters_follow = set_t<
				token_type::right_paren
			>{};

			void parse_parameters()
			{
				assert(current_is(parameters_first));
				while (true) {
					parse_parameter();
					if (!current_is(token_type::comma)) {
						return;
					}
					advance();
					expect(type_first);
				}
			}

			void parse_parameter()
			{
				assert(current_is(type_first));
				parse_type();
				consume(token_type::identifier);
			}

			static constexpr auto type_first = set_t<
				token_type::identifier,
				token_type::kw_int,
				token_type::kw_boolean,
				token_type::kw_void
			>{};

			void parse_type()
			{
				assert(current_is(type_first));
				advance();
				while (current_is(token_type::left_bracket)) {
					advance();
					consume(token_type::right_bracket);
				}
			}

			static constexpr auto block_first = set_t<
				token_type::left_brace
			>{};

			void parse_block()
			{
				assert(current_is(token_type::left_brace));
				advance();
				while (!current_is(token_type::right_brace)) {
					parse_block_statement();
				}
				advance();
			}

			void parse_block_statement()
			{
				if (!current_is(type_first)) {
					// We see a while / return / if / ...
					return parse_statement();
				}
				// Statement or local variable declaration?
				const auto first_token = current();
				advance();
				if (first_token.type() != token_type::identifier) {
					expect(token_type::identifier, token_type::left_bracket);
				}
				if (current_is(token_type::identifier)) {
					// Type foo
					//      ^^^
					putback(first_token);
				} else if (!current_is(token_type::left_bracket)) {
					// foo +
					//     ^
					putback(first_token);
					return parse_statement();
				} else {
					// foo [
					//     ^
					const auto second_token = current();
					advance();
					if (!current_is(token_type::right_bracket)) {
						// foo [ 1 + 2 ]
						//       ^
						putback(second_token);
						putback(first_token);
						return parse_statement();
					}
					// foo [ ]
					//       ^
					putback(second_token);
					putback(first_token);
				}
				parse_local_variable_decl();
			}

			void parse_local_variable_decl()
			{
				assert(current_is(type_first));
				parse_type();
				consume(token_type::identifier);
				if (current_is(token_type::assign)) {
					advance();
					parse_expression();
				}
				consume(token_type::semicolon);
			}

			void parse_statement()
			{
				switch (current_type()) {
				case token_type::left_brace:
					return parse_block();
				case token_type::semicolon:
					return parse_empty_statement();
				case token_type::kw_if:
					return parse_if();
				default:
					return parse_expression_statement();
				case token_type::kw_while:
					return parse_while();
				case token_type::kw_return:
					return parse_return();
				}
			}

			void parse_empty_statement()
			{
				assert(current_is(token_type::semicolon));
				advance();
			}

			void parse_while()
			{
				assert(current_is(token_type::kw_while));
				advance();
				consume(token_type::left_paren);
				parse_expression();
				consume(token_type::right_paren);
				parse_statement();
			}

			void parse_if()
			{
				assert(current_is(token_type::kw_if));
				advance();
				consume(token_type::left_paren);
				parse_expression();
				consume(token_type::right_paren);
				parse_statement();
				if (current_is(token_type::kw_else)) {
					advance();
					parse_statement();
				}
			}

			void parse_expression_statement()
			{
				parse_expression();
				consume(token_type::semicolon);
			}

			void parse_return()
			{
				assert(current_is(token_type::kw_return));
				advance();
				if (!current_is(token_type::semicolon)) {
					parse_expression();
				}
				consume(token_type::semicolon);
			}

			static constexpr auto prefix_ops_first = set_t<
				token_type::logical_not,
				token_type::minus
			>{};

			static constexpr auto postfix_ops_first = set_t<
				token_type::dot,
				token_type::left_bracket
			>{};

			void parse_expression()
			{
				// This function uses an iterative formulation of the
				// precedence climbing algorithm.
				auto prec_stack = std::stack<int>{};
				auto preop_stack = std::stack<token_type>{};
				auto cur_prec = -1;  // impossible precedence level
				auto min_prec = 0;
			outer_loop:
				assert(preop_stack.empty());
				while (current_is(prefix_ops_first)) {
					preop_stack.push(current_type());
					advance();
				}
				expect(primary_expr_first);
				parse_primary();
				while (current_is(postfix_ops_first)) {
					parse_postfix_op();
				}
				while (!preop_stack.empty()) {
					preop_stack.pop();
				}
			inner_loop:
				cur_prec = precedence(current_type());
				if (is_binary_op(current_type()) && (cur_prec >= min_prec)) {
					if (is_left_assoc(current_type())) {
						++cur_prec;
					}
					prec_stack.push(min_prec);
					min_prec = cur_prec;
					advance();
					goto outer_loop;
				}
				if (!prec_stack.empty()) {
					min_prec = prec_stack.top();
					prec_stack.pop();
					goto inner_loop;
				}
			}

			void parse_postfix_op()
			{
				assert(current_is(postfix_ops_first));
				if (current_is(token_type::left_bracket)) {
					// Array access
					advance();
					parse_expression();
					consume(token_type::right_bracket);
				} else {
					// Field access or method invocation
					consume(token_type::dot);
					consume(token_type::identifier);
					if (current_is(token_type::left_paren)) {
						// Method invocation
						advance();
						parse_arguments();
						consume(token_type::right_paren);
					}
				}
			}

			static constexpr auto primary_expr_first = set_t<
				token_type::kw_null,
				token_type::kw_false,
				token_type::kw_true,
				token_type::integer_literal,
				token_type::identifier,
				token_type::kw_this,
				token_type::left_paren,
				token_type::kw_new
			>{};

			void parse_primary()
			{
				assert(current_is(primary_expr_first));
				switch (current_type()) {
				case token_type::kw_null:
				case token_type::kw_false:
				case token_type::kw_true:
				case token_type::integer_literal:
				case token_type::kw_this:
					advance();
					return;
				case token_type::identifier:
					advance();
					if (current_is(token_type::left_paren)) {
						advance();
						parse_arguments();
						consume(token_type::right_paren);
					}
					return;
				case token_type::left_paren:
					advance();
					parse_expression();
					consume(token_type::right_paren);
					return;
				case token_type::kw_new:
					return parse_new_expression();
				default:
					MINIJAVA_NOT_REACHED();
				}
			}

			void parse_arguments()
			{
				if (current_is(token_type::right_paren)) {
					return;
				}
				while (true) {
					parse_expression();
					if (!current_is(token_type::comma)) {
						return;
					}
					advance();
				}
			}

			void parse_new_expression()
			{
				assert(current_is(token_type::kw_new));
				advance();
				const auto type = consume(type_first);
				switch (expect(token_type::left_paren, token_type::left_bracket)) {
				case token_type::left_paren:
					if (type != token_type::identifier) {
						throw_syntax_error_new_primitive(current(), type);
					}
					advance();
					consume(token_type::right_paren);
					return;
				case token_type::left_bracket:
					advance();
					parse_expression();
					consume(token_type::right_bracket);
					while (current_is(token_type::left_bracket)) {
						const auto left_bracket = current();
						advance();
						if (!current_is(token_type::right_bracket)) {
							// Immediately subscripted array expression
							// new int[length][][expr]
							// -> (new int[length][])[expr]
							putback(left_bracket);
							return;
						}
						advance();
					}
					return;
				default:
					MINIJAVA_NOT_REACHED();
				}
			}

			// A few helper functions to access the current token and advance
			// in the token stream.  Always use these functoins and don't mess
			// with the iterator directly.

			template<token_type... TTs>
			token_type expect(token_type_set<TTs...>)
			{
				return expect(TTs...);
			}

			template<typename... TTs>
			token_type expect(const TTs... tts)
			{
				if (!current_is(tts...)) {
					throw_syntax_error(current(), {tts...});
				}
				return current_type();
			}

			template<typename... TTs>
			token_type consume(const TTs... tts)
			{
				const auto tt = expect(tts...);
				advance();
				return tt;
			}

			token next()
			{
				advance();
				return current();
			}

			void advance()
			{
				token_buffer.pop();
				if (token_buffer.empty()) {
					++it;
					token_buffer.push(*it);
				}
			}

			void putback(token tok)
			{
				token_buffer.push(tok);
			}

			token current()
			{
				assert(!token_buffer.empty());
				return token_buffer.top();
			}

			token_type current_type() noexcept
			{
				return current().type();
			}

			template<token_type... TTs>
			bool current_is(token_type_set<TTs...>)
			{
				return current_is(TTs...);
			}

			template<typename... TTs>
			bool current_is(const TTs... tts)
			{
				const auto candidates = std::initializer_list<token_type>{tts...};
				return std::any_of(
					std::begin(candidates), std::end(candidates),
					[ct = current_type()](const auto tt){ return ct == tt; }
				);
			}

			std::stack<token> token_buffer;

			InIterT it;

		};  // struct parser

	}  // namespace detail


	template<typename InIterT>
	void parse_program(InIterT first, InIterT last)
	{
		auto parser = detail::parser<InIterT>(first, last);
		return parser.parse_program();
	}

}  // namespace minijava
