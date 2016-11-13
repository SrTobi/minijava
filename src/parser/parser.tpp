#ifndef MINIJAVA_INCLUDED_FROM_PARSER_PARSER_HPP
#error "Never `#include <parser/parser.tpp>` directly; `#include <parser/parser.hpp>` instead."
#endif

#include <algorithm>
#include <cstring>
#include <initializer_list>
#include <stack>
#include <memory>

#include "exceptions.hpp"
#include "lexer/token_type.hpp"
#include "lexer/token.hpp"

#include "parser/operator.tpp"
#include "parser/ast.hpp"

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
			template<typename Type>
			using ast_ptr = std::unique_ptr<Type>;

			template<token_type... TTs>
			using set_t = token_type_set<TTs...>;

			parser(InIterT first, InIterT /*last*/)
				: it(first)
			{
				token_buffer.push(*it);
			}

			ast_ptr<ast::program> parse_program()
			{
				auto prog = make<ast::program>();
				while (!current_is(token_type::eof)) {
					expect(token_type::kw_class);
					auto decl = parse_class_declaration();
					prog->add_class(std::move(decl));
				}

				return prog;
			}

			ast_ptr<ast::class_declaration> parse_class_declaration()
			{
				assert(current_is(token_type::kw_class));
				advance();
				auto id_tok = current();
				consume(token_type::identifier);
				auto class_decl = make<ast::class_declaration>(id_tok.lexval());
				consume(token_type::left_brace);
				while (!current_is(token_type::right_brace)) {
					expect(token_type::kw_public);
					parse_class_member(*class_decl);
				}
				consume(token_type::right_brace);
				return class_decl;
			}

			void parse_class_member(ast::class_declaration& decl)
			{
				assert(current_is(token_type::kw_public));
				advance();
				expect(cat(set_t<token_type::kw_static>{}, type_first));
				if (current_is(token_type::kw_static)) {
					auto main_method = parse_main_method();
					decl.add_main_method(std::move(main_method));
				}else{
					// parse field or method
					auto type = parse_type();
					auto id_tok = current();
					consume(token_type::identifier);
					if (consume(token_type::semicolon, token_type::left_paren) == token_type::left_paren) {
						// parse method
						std::vector<ast_ptr<ast::var_decl>> params;
						if (current_is(parameters_first)) {
							params = parse_parameters();
						}
						consume(token_type::right_paren);
						expect(token_type::left_brace);
						auto body = parse_block();
						auto method = make<ast::method>(id_tok.lexval(), std::move(type), std::move(params), std::move(body));
						decl.add_method(std::move(method));
					}else{
						// field
						auto field = make<ast::var_decl>(std::move(type), id_tok.lexval());
						decl.add_field(std::move(field));
					}
				}
			}

			ast_ptr<ast::main_method> parse_main_method()
			{
				assert(current_is(token_type::kw_static));
				advance();
				consume(token_type::kw_void);
				auto id_tok = current();
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
				auto body = parse_block();
				return make<ast::main_method>(id_tok.lexval(), std::move(body));
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

			std::vector<ast_ptr<ast::var_decl>> parse_parameters()
			{
				assert(current_is(parameters_first));
				std::vector<ast_ptr<ast::var_decl>> params;
				while (true) {
					auto param = parse_parameter();
					params.push_back(std::move(param));
					if (!current_is(token_type::comma)) {
						return params;
					}
					advance();
					expect(type_first);
				}
			}

			ast_ptr<ast::var_decl> parse_parameter()
			{
				assert(current_is(type_first));
				auto type = parse_type();
				auto id_tok = current();
				consume(token_type::identifier);
				return make<ast::var_decl>(std::move(type), id_tok.lexval());
			}

			static constexpr auto type_first = set_t<
				token_type::identifier,
				token_type::kw_int,
				token_type::kw_boolean,
				token_type::kw_void
			>{};

			ast_ptr<ast::type> parse_type()
			{
				assert(current_is(type_first));
				auto type_tok = current();
				advance();
				std::size_t rank = 0;
				while (current_is(token_type::left_bracket)) {
					advance();
					consume(token_type::right_bracket);
					++rank;
				}
				return make_type(type_tok, rank);
			}

			static constexpr auto block_first = set_t<
				token_type::left_brace
			>{};

			ast_ptr<ast::block> parse_block()
			{
				assert(current_is(token_type::left_brace));
				advance();
				auto block = make<ast::block>();
				while (!current_is(token_type::right_brace)) {
					auto stmt = parse_block_statement();
					block->add_block_statement(std::move(stmt));
				}
				advance();
				return block;
			}

			ast_ptr<ast::block_statement> parse_block_statement()
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
				return parse_local_variable_decl();
			}

			ast_ptr<ast::local_variable_statement> parse_local_variable_decl()
			{
				assert(current_is(type_first));
				auto type = parse_type();
				auto id_tok = current();
				consume(token_type::identifier);
				ast_ptr<ast::expression> init = nullptr;
				if (current_is(token_type::assign)) {
					advance();
					init = parse_expression();
				}
				consume(token_type::semicolon);
				auto decl = make<ast::var_decl>(std::move(type), id_tok.lexval());
				return make<ast::local_variable_statement>(std::move(decl), std::move(init));
			}

			ast_ptr<ast::statement> parse_statement()
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

			ast_ptr<ast::empty_statement> parse_empty_statement()
			{
				assert(current_is(token_type::semicolon));
				advance();
				return make<ast::empty_statement>();
			}

			ast_ptr<ast::while_statement> parse_while()
			{
				assert(current_is(token_type::kw_while));
				advance();
				consume(token_type::left_paren);
				auto cond = parse_expression();
				consume(token_type::right_paren);
				auto body = parse_statement();
				return make<ast::while_statement>(std::move(cond), std::move(body));
			}

			ast_ptr<ast::if_statement> parse_if()
			{
				assert(current_is(token_type::kw_if));
				advance();
				consume(token_type::left_paren);
				auto cond = parse_expression();
				consume(token_type::right_paren);
				auto then_body = parse_statement();
				ast_ptr<ast::statement> else_body = nullptr;
				if (current_is(token_type::kw_else)) {
					advance();
					else_body = parse_statement();
				}
				return make<ast::if_statement>(std::move(then_body), std::move(else_body), std::move(cond));
			}

			ast_ptr<ast::expression_statement> parse_expression_statement()
			{
				auto expr = parse_expression();
				consume(token_type::semicolon);
				return make<ast::expression_statement>(std::move(expr));
			}

			ast_ptr<ast::return_statement> parse_return()
			{
				assert(current_is(token_type::kw_return));
				advance();
				ast_ptr<ast::expression> ret_expr = nullptr;
				if (!current_is(token_type::semicolon)) {
					ret_expr = parse_expression();
				}
				consume(token_type::semicolon);
				return make<ast::return_statement>(std::move(ret_expr));
			}

			static constexpr auto prefix_ops_first = set_t<
				token_type::logical_not,
				token_type::minus
			>{};

			static constexpr auto postfix_ops_first = set_t<
				token_type::dot,
				token_type::left_bracket
			>{};

			ast_ptr<ast::expression> parse_expression()
			{
				// This function uses an iterative formulation of the
				// precedence climbing algorithm.
				auto prec_stack = std::stack<std::tuple<int, ast_ptr<ast::expression>, token_type>>{};
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
				auto rhs = parse_primary();
				while (current_is(postfix_ops_first)) {
					rhs = parse_postfix_op(std::move(rhs));
				}
				while (!preop_stack.empty()) {
					auto op = preop_stack.top();
					preop_stack.pop();
					rhs = make<ast::unary_expression>(to_unary_operation(op), std::move(rhs));
				}
			inner_loop:
				cur_prec = precedence(current_type());
				if (is_binary_op(current_type()) && (cur_prec >= min_prec)) {
					if (is_left_assoc(current_type())) {
						++cur_prec;
					}
					prec_stack.emplace(min_prec, std::move(rhs), current_type());
					min_prec = cur_prec;
					advance();
					goto outer_loop;
				}
				if (!prec_stack.empty()) {
					assert(rhs);
					ast_ptr<ast::expression> lhs;
					token_type op_type;
					std::tie(min_prec, lhs, op_type) = std::move(prec_stack.top());
					prec_stack.pop();
					if(op_type == token_type::assign)
						rhs = make<ast::assignment_expression>(std::move(lhs), std::move(rhs));
					else
						rhs = make<ast::binary_expression>(to_binary_operation(op_type), std::move(lhs), std::move(rhs));
					goto inner_loop;
				}
				return rhs;
			}

			ast::unary_operation_type to_unary_operation(const token_type& type)
			{
				using ast::unary_operation_type;
				switch(type)
				{
					case token_type::logical_not: return unary_operation_type::type_not;
					case token_type::minus:       return unary_operation_type::type_negate;
				default:
					MINIJAVA_NOT_REACHED();
				}
			}

			ast::binary_operation_type to_binary_operation(const token_type& type)
			{
				using ast::binary_operation_type;
				switch(type)
				{
					case token_type::logical_or:        return binary_operation_type::type_logical_or;
					case token_type::logical_and:       return binary_operation_type::type_logical_and;
					case token_type::equal:             return binary_operation_type::type_equality;
					case token_type::not_equal:         return binary_operation_type::type_unequality;
					case token_type::less_than:         return binary_operation_type::type_lower;
					case token_type::less_equal:        return binary_operation_type::type_lower_equal;
					case token_type::greater_than:      return binary_operation_type::type_greater;
					case token_type::greater_equal:     return binary_operation_type::type_greater_equal;
					case token_type::plus:              return binary_operation_type::type_add;
					case token_type::minus:             return binary_operation_type::type_subtract;
					case token_type::multiply:          return binary_operation_type::type_multiplay;
					case token_type::divides:           return binary_operation_type::type_divide;
					case token_type::modulo:            return binary_operation_type::type_modulo;
				default:
					MINIJAVA_NOT_REACHED();
				}
			}


			ast_ptr<ast::expression> parse_postfix_op(ast_ptr<ast::expression> inner)
			{
				assert(current_is(postfix_ops_first));
				if (current_is(token_type::left_bracket)) {
					// Array access
					advance();
					auto index_expr = parse_expression();
					consume(token_type::right_bracket);
					return make<ast::array_access>(std::move(inner), std::move(index_expr));
				} else {
					// Field access or method invocation
					consume(token_type::dot);
					auto id_tok = current();
					consume(token_type::identifier);
					if (current_is(token_type::left_paren)) {
						// Method invocation
						advance();
						auto args = parse_arguments();
						consume(token_type::right_paren);
						return make<ast::method_invocation>(std::move(inner), id_tok.lexval(), std::move(args));
					}else{
						return make<ast::variable_access>(std::move(inner), id_tok.lexval());
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

			ast_ptr<ast::expression> parse_primary()
			{
				assert(current_is(primary_expr_first));
				switch (current_type()) {
				case token_type::kw_null:
					advance();
					return make<ast::null_constant>();
				case token_type::kw_false:
					advance();
					return make<ast::boolean_constant>(false);
				case token_type::kw_true:
					advance();
					return make<ast::boolean_constant>(true);
				case token_type::integer_literal:
					{
						auto lit_tok = current();
						advance();
						return make<ast::integer_constant>(lit_tok.lexval());
					}
				case token_type::kw_this:
					advance();
					return make<ast::this_ref>();
				case token_type::identifier:
					{
						// variable or function call
						auto id_tok = current();
						advance();
						if (current_is(token_type::left_paren)) {
							advance();
							auto args = parse_arguments();
							consume(token_type::right_paren);
							return make<ast::method_invocation>(nullptr, id_tok.lexval(), std::move(args));
						}else{
							return make<ast::variable_access>(nullptr, id_tok.lexval());
						}
					}
				case token_type::left_paren:
					{
						advance();
						auto inner = parse_expression();
						consume(token_type::right_paren);
						return inner;
					}
				case token_type::kw_new:
					return parse_new_expression();
				default:
					MINIJAVA_NOT_REACHED();
				}
			}

			std::vector<ast_ptr<ast::expression>> parse_arguments()
			{
				std::vector<ast_ptr<ast::expression>> args{};
				if (current_is(token_type::right_paren)) {
					return args;
				}
				while (true) {
					auto expr = parse_expression();
					args.push_back(std::move(expr));
					if (!current_is(token_type::comma)) {
						return args;
					}
					advance();
				}
			}

			ast_ptr<ast::expression> parse_new_expression()
			{
				assert(current_is(token_type::kw_new));
				advance();
				const auto type_tok = current();
				const auto type = consume(type_first);
				switch (expect(token_type::left_paren, token_type::left_bracket)) {
				case token_type::left_paren:
					if (type != token_type::identifier) {
						throw_syntax_error_new_primitive(current(), type);
					}
					advance();
					consume(token_type::right_paren);
					return make<ast::object_instantiation>(type_tok.lexval());
				case token_type::left_bracket:
					{
						std::size_t rank = 1;
						advance();
						auto extend_expr = parse_expression();
						consume(token_type::right_bracket);
						while (current_is(token_type::left_bracket)) {
							const auto left_bracket = current();
							advance();
							if (!current_is(token_type::right_bracket)) {
								// Immediately subscripted array expression
								// new int[length][][expr]
								// -> (new int[length][])[expr]
								putback(left_bracket);
								break;
							}
							++rank;
							advance();
						}
						auto type = make_type(type_tok, rank);
						return make<ast::array_instantiation>(std::move(type), std::move(extend_expr));
					}
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

			template<typename Type, typename... ParamsT>
			ast_ptr<Type> make(ParamsT&&... params)
			{
				return std::make_unique<Type>(std::forward<ParamsT>(params)...);
			}

			ast::primitive_type to_primitive(const token& tok)
			{
				using ast::primitive_type;
				switch(tok.type())
				{
				case token_type::kw_boolean:    return {primitive_type::type_boolean};
				case token_type::kw_int:        return {primitive_type::type_int};
				case token_type::kw_void:       return {primitive_type::type_void};
				default:
					MINIJAVA_NOT_REACHED();
				}
			}

			ast_ptr<ast::type> make_type(const token& tok, const std::size_t rank)
			{
				if(tok.type() == token_type::identifier)
					return make<ast::type>(tok.lexval(), rank);
				else
					return make<ast::type>(to_primitive(tok), rank);
			}

			std::stack<token> token_buffer;

			InIterT it;

		};  // struct parser

	}  // namespace detail


	template<typename InIterT>
	std::unique_ptr<ast::program> parse_program(InIterT first, InIterT last)
	{
		auto parser = detail::parser<InIterT>(first, last);
		return parser.parse_program();
	}

}  // namespace minijava
