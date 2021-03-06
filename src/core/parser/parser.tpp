#ifndef MINIJAVA_INCLUDED_FROM_PARSER_PARSER_HPP
#error "Never `#include <parser/parser.tpp>` directly; `#include <parser/parser.hpp>` instead."
#endif

#include <algorithm>
#include <cstring>
#include <initializer_list>
#include <memory>
#include <utility>
#include <stack>

#include "exceptions.hpp"
#include "lexer/token_type.hpp"
#include "lexer/token.hpp"

#include "parser/ast.hpp"
#include "parser/operator.tpp"


namespace minijava
{
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
		// token with a keyword identifying a primitive type) being used in a new object
		// expression that was detected at `pde` which must be a left
		// parenthesis.
		[[noreturn]]
		void throw_syntax_error_new_primitive(const token& pde, const token& type_token);

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

			parser(InIterT first, InIterT /*last*/, ast_factory& factory)
				: it{first}, factory{&factory}
			{
				token_buffer.push(*it);
			}

			ast_ptr<ast::program> parse_program()
			{
				std::vector<ast_ptr<ast::class_declaration>> classes;
				while (!current_is(token_type::eof)) {
					expect(token_type::kw_class);
					auto decl = parse_class_declaration();
					classes.push_back(std::move(decl));
				}
				return make<ast::program>()(std::move(classes));
			}

			ast_ptr<ast::class_declaration> parse_class_declaration()
			{
				assert(current_is(token_type::kw_class));
				const auto pos = current().position();
				advance();
				const auto id_tok = current();
				auto fields = std::vector<ast_ptr<ast::var_decl>>{};
				auto instance_methods = std::vector<ast_ptr<ast::instance_method>>{};
				auto main_methods = std::vector<ast_ptr<ast::main_method>>{};
				consume(token_type::identifier);
				consume(token_type::left_brace);
				while (!current_is(token_type::right_brace)) {
					expect(token_type::kw_public);
					parse_class_member(fields, instance_methods, main_methods);
				}
				consume(token_type::right_brace);
				return make<ast::class_declaration>().at(pos)(
					id_tok.lexval(),
					std::move(fields),
					std::move(instance_methods),
					std::move(main_methods)
				);
			}

			void parse_class_member(std::vector<ast_ptr<ast::var_decl>>& fields,
			                        std::vector<ast_ptr<ast::instance_method>>& methods,
			                        std::vector<ast_ptr<ast::main_method>>& main_methods)
			{
				assert(current_is(token_type::kw_public));
				const auto pos = current().position();
				advance();
				expect(cat(set_t<token_type::kw_static>{}, type_first));
				if (current_is(token_type::kw_static)) {
					main_methods.push_back(parse_main_method(pos));
				} else {
					// parse field or method
					auto type = parse_type();
					auto id_tok = current();
					consume(token_type::identifier);
					if (consume(token_type::semicolon, token_type::left_paren) == token_type::left_paren) {
						// parse method
						auto params = std::vector<ast_ptr<ast::var_decl>>{};
						if (current_is(parameters_first)) {
							params = parse_parameters();
						}
						consume(token_type::right_paren);
						if (current_is(token_type::kw_throws)) {
							consume_throws_spec();
						}
						expect(token_type::left_brace);
						auto body = parse_block();
						auto method = make<ast::instance_method>().at(pos)(
							id_tok.lexval(),
							std::move(type),
							std::move(params),
							std::move(body)
						);
						methods.push_back(std::move(method));
					} else {
						// field
						auto field = make<ast::var_decl>().at(pos)
							(std::move(type), id_tok.lexval());
						fields.push_back(std::move(field));
					}
				}
			}

			ast_ptr<ast::main_method> parse_main_method(position pos)
			{
				assert(current_is(token_type::kw_static));
				advance();
				consume(token_type::kw_void);
				const auto id_main = current();
				consume(token_type::identifier);
				consume(token_type::left_paren);
				expect(token_type::identifier);
				if (std::strcmp(current().lexval().c_str(), "String") != 0) {
					throw_syntax_error_main_signature(current());
				}
				advance();
				consume(token_type::left_bracket);
				consume(token_type::right_bracket);
				const auto id_args = current();
				consume(token_type::identifier);
				consume(token_type::right_paren);
				if (current_is(token_type::kw_throws)) {
					consume_throws_spec();
				}
				expect(block_first);
				auto body = parse_block();
				return make<ast::main_method>().at(pos)
					(id_main.lexval(), id_args.lexval(), std::move(body));
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
				const auto pos = current().position();
				auto type = parse_type();
				const auto id_tok = current();
				consume(token_type::identifier);
				return make<ast::var_decl>().at(pos)
					(std::move(type), id_tok.lexval());
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

			void consume_throws_spec()
			{
				consume(token_type::kw_throws);
				consume(token_type::identifier);
			}

			static constexpr auto block_first = set_t<
				token_type::left_brace
			>{};

			ast_ptr<ast::block> parse_block()
			{
				assert(current_is(token_type::left_brace));
				const auto pos = current().position();
				advance();
				auto block_statements = std::vector<ast_ptr<ast::block_statement>>{};
				while (!current_is(token_type::right_brace)) {
					block_statements.push_back(parse_block_statement());
				}
				advance();
				return make<ast::block>().at(pos)
					(std::move(block_statements));
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
				const auto pos = current().position();
				auto type = parse_type();
				const auto id_tok = current();
				consume(token_type::identifier);
				auto init = ast_ptr<ast::expression>{};
				if (current_is(token_type::assign)) {
					advance();
					init = parse_expression();
				}
				consume(token_type::semicolon);
				auto decl = make<ast::var_decl>().at(pos)
					(std::move(type), id_tok.lexval());
				return make<ast::local_variable_statement>().at(pos)
					(std::move(decl), std::move(init));
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
				const auto pos = current().position();
				advance();
				return make<ast::empty_statement>().at(pos)();
			}

			ast_ptr<ast::while_statement> parse_while()
			{
				assert(current_is(token_type::kw_while));
				const auto pos = current().position();
				advance();
				consume(token_type::left_paren);
				auto cond = parse_expression();
				consume(token_type::right_paren);
				auto body = parse_statement();
				return make<ast::while_statement>().at(pos)
					(std::move(cond), std::move(body));
			}

			ast_ptr<ast::if_statement> parse_if()
			{
				assert(current_is(token_type::kw_if));
				const auto pos = current().position();
				advance();
				consume(token_type::left_paren);
				auto cond = parse_expression();
				consume(token_type::right_paren);
				auto then_body = parse_statement();
				auto else_body = ast_ptr<ast::statement>{};
				if (current_is(token_type::kw_else)) {
					advance();
					else_body = parse_statement();
				}
				return make<ast::if_statement>().at(pos)
					(std::move(cond), std::move(then_body), std::move(else_body));
			}

			ast_ptr<ast::expression_statement> parse_expression_statement()
			{
				const auto pos = current().position();
				auto expr = parse_expression();
				consume(token_type::semicolon);
				return make<ast::expression_statement>().at(pos)(std::move(expr));
			}

			ast_ptr<ast::return_statement> parse_return()
			{
				assert(current_is(token_type::kw_return));
				const auto pos = current().position();
				advance();
				auto ret_expr = ast_ptr<ast::expression>{};
				if (!current_is(token_type::semicolon)) {
					ret_expr = parse_expression();
				}
				consume(token_type::semicolon);
				return make<ast::return_statement>().at(pos)(std::move(ret_expr));
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
				auto prec_stack = std::stack<std::tuple<int, ast_ptr<ast::expression>, token>>{};
				auto preop_stack = std::stack<token>{};
				auto cur_prec = -1;  // impossible precedence level
				auto min_prec = 0;
			outer_loop:
				assert(preop_stack.empty());
				while (current_is(prefix_ops_first)) {
					preop_stack.push(current());
					advance();
				}
				expect(primary_expr_first);
				auto rhs = parse_primary(preop_stack);
				while (current_is(postfix_ops_first)) {
					rhs = parse_postfix_op(std::move(rhs));
				}
				while (!preop_stack.empty()) {
					const auto op = preop_stack.top();
					preop_stack.pop();
					rhs = make<ast::unary_expression>().at(op.position())
						(to_unary_operation(op.type()), std::move(rhs));
				}
			inner_loop:
				cur_prec = precedence(current_type());
				if (is_binary_op(current_type()) && (cur_prec >= min_prec)) {
					if (is_left_assoc(current_type())) {
						++cur_prec;
					}
					prec_stack.emplace(min_prec, std::move(rhs), current());
					min_prec = cur_prec;
					advance();
					goto outer_loop;
				}
				if (!prec_stack.empty()) {
					assert(rhs);
					auto lhs = ast_ptr<ast::expression>{};
					auto op = token::create(token_type::eof);
					std::tie(min_prec, lhs, op) = std::move(prec_stack.top());
					prec_stack.pop();
					rhs = make<ast::binary_expression>().at(op.position())
						(to_binary_operation(op.type()), std::move(lhs), std::move(rhs));
					goto inner_loop;
				}
				return rhs;
			}

			static ast::unary_operation_type to_unary_operation(const token_type& type)
			{
				using ast::unary_operation_type;
				switch (type) {
					case token_type::logical_not: return unary_operation_type::logical_not;
					case token_type::minus:       return unary_operation_type::minus;
				default:
					MINIJAVA_NOT_REACHED();
				}
			}

			static ast::binary_operation_type to_binary_operation(const token_type& type)
			{
				using ast::binary_operation_type;
				switch (type) {
					case token_type::assign:        return binary_operation_type::assign;
					case token_type::logical_or:    return binary_operation_type::logical_or;
					case token_type::logical_and:   return binary_operation_type::logical_and;
					case token_type::equal:         return binary_operation_type::equal;
					case token_type::not_equal:     return binary_operation_type::not_equal;
					case token_type::less_than:     return binary_operation_type::less_than;
					case token_type::less_equal:    return binary_operation_type::less_equal;
					case token_type::greater_than:  return binary_operation_type::greater_than;
					case token_type::greater_equal: return binary_operation_type::greater_equal;
					case token_type::plus:          return binary_operation_type::plus;
					case token_type::minus:         return binary_operation_type::minus;
					case token_type::multiply:      return binary_operation_type::multiply;
					case token_type::divide:        return binary_operation_type::divide;
					case token_type::modulo:        return binary_operation_type::modulo;
				default:
					MINIJAVA_NOT_REACHED();
				}
			}

			ast_ptr<ast::expression> parse_postfix_op(ast_ptr<ast::expression> inner)
			{
				assert(current_is(postfix_ops_first));
				if (current_is(token_type::left_bracket)) {
					// Array access
					const auto pos = current().position();
					advance();
					auto index_expr = parse_expression();
					consume(token_type::right_bracket);
					return make<ast::array_access>().at(pos)
						(std::move(inner), std::move(index_expr));
				} else {
					// Field access or method invocation
					consume(token_type::dot);
					const auto id_tok = current();
					consume(token_type::identifier);
					if (current_is(token_type::left_paren)) {
						// Method invocation
						advance();
						auto args = parse_arguments();
						consume(token_type::right_paren);
						return make<ast::method_invocation>().at(id_tok.position())
							(std::move(inner), id_tok.lexval(), std::move(args));
					} else {
						return make<ast::variable_access>().at(id_tok.position())
							(std::move(inner), id_tok.lexval());
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

			ast_ptr<ast::expression> parse_primary(std::stack<token>& preop_stack)
			{
				assert(current_is(primary_expr_first));
				const auto pos = current().position();
				switch (current_type()) {
				case token_type::kw_null:
					advance();
					return make<ast::null_constant>().at(pos)();
				case token_type::kw_false:
					advance();
					return make<ast::boolean_constant>().at(pos)(false);
				case token_type::kw_true:
					advance();
					return make<ast::boolean_constant>().at(pos)(true);
				case token_type::integer_literal:
					return parse_integer_literal(preop_stack);
				case token_type::kw_this:
					advance();
					return make<ast::this_ref>().at(pos)();
				case token_type::identifier:
					return parse_variable_or_function_call();
				case token_type::left_paren:
					return parse_parenthesized_expression();
				case token_type::kw_new:
					return parse_new_expression();
				default:
					MINIJAVA_NOT_REACHED();
				}
			}

			ast_ptr<ast::integer_constant> parse_integer_literal(std::stack<token>& preop_stack)
			{
				assert(current_is(token_type::integer_literal));
				auto result = ast_ptr<ast::integer_constant>{};
				const auto lit_tok = current();
				if (!preop_stack.empty() && (preop_stack.top().type() == token_type::minus)) {
					result = make<ast::integer_constant>().at(preop_stack.top().position())
						(lit_tok.lexval(), true);
					preop_stack.pop();
				} else {
					result = make<ast::integer_constant>().at(lit_tok.position())
						(lit_tok.lexval(), false);
				}
				advance();
				return result;
			}

			ast_ptr<ast::expression> parse_variable_or_function_call()
			{
				const auto id_tok = current();
				advance();
				if (current_is(token_type::left_paren)) {
					advance();
					auto args = parse_arguments();
					consume(token_type::right_paren);
					return make<ast::method_invocation>().at(id_tok.position())
						(nullptr, id_tok.lexval(), std::move(args));
				} else {
					return make<ast::variable_access>().at(id_tok.position())
						(nullptr, id_tok.lexval());
				}
			}

			ast_ptr<ast::expression> parse_parenthesized_expression()
			{
				assert(current_is(token_type::left_paren));
				advance();
				auto inner = parse_expression();
				consume(token_type::right_paren);
				return inner;
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
				const auto pos = current().position();
				advance();
				const auto type_tok = current();
				consume(type_first);
				switch (expect(token_type::left_paren, token_type::left_bracket)) {
				case token_type::left_paren:
					if (type_tok.type() != token_type::identifier) {
						throw_syntax_error_new_primitive(current(), type_tok);
					}
					advance();
					consume(token_type::right_paren);
					return make<ast::object_instantiation>().at(pos)
						(type_tok.lexval());
				case token_type::left_bracket:
					{
						std::size_t rank = 1;
						advance();
						auto extent_expr = parse_expression();
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
						return make<ast::array_instantiation>().at(pos)
							(std::move(type), std::move(extent_expr));
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
					[ct = current_type()](const auto tt) { return ct == tt; }
				);
			}

			template <typename NodeT>
			auto make()
			{
				return factory->make<NodeT>();
			}

			auto make_type(const token& tok, const std::size_t rank)
			{
				if (tok.type() == token_type::identifier) {
					return make<ast::type>().at(tok.position())
						(tok.lexval(), rank);
				} else {
					return make<ast::type>().at(tok.position())
						(to_primitive(tok), rank);
				}
			}

			static ast::primitive_type to_primitive(const token& tok)
			{
				using ast::primitive_type;
				switch (tok.type()) {
				case token_type::kw_boolean: return primitive_type::type_boolean;
				case token_type::kw_int:     return primitive_type::type_int;
				case token_type::kw_void:    return primitive_type::type_void;
				default:
					MINIJAVA_NOT_REACHED();
				}
			}

			std::stack<token> token_buffer{};

			InIterT it{};

			ast_factory* factory{};

		};  // struct parser

	}  // namespace detail


	template<typename InIterT>
	std::unique_ptr<ast::program>
	parse_program(const InIterT first, const InIterT last, ast_factory& factory)
	{
		auto parser = detail::parser<InIterT>(first, last, factory);
		return parser.parse_program();
	}

	template<typename InIterT>
	std::unique_ptr<ast::program>
	parse_program(const InIterT first, const InIterT last)
	{
		auto factory = ast_factory{};
		return parse_program(first, last, factory);
	}

}  // namespace minijava
