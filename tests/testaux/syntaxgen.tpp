#ifndef TESTAUX_INCLUDED_FROM_SYNTAXGEN_HPP
#error "Never `#include <syntaxgen.tpp>` directly; `#include <syntaxgen.hpp>` instead."
#endif

#include <cmath>
#include <random>
#include <string>
#include <utility>

#include "lexer/token_type.hpp"

#include "random_tokens.hpp"


namespace testaux
{

	namespace detail
	{

		template <typename T>
		class restore_finally final
		{
		public:

			restore_finally() = default;

			restore_finally(T& dest, const T update) : _destp{&dest}, _prev{dest}
			{
				dest = update;
			}

			~restore_finally()
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


		template <typename RndEngT, typename PoolT>
		class syntaxgen final
		{
		private:

			using tt = minijava::token_type;

		public:

			syntaxgen(RndEngT& engine, PoolT& pool, const std::size_t limit)
				: _engine{engine}
				, _pool{pool}
				, _nest_depth{0}
				, _nest_limit{limit}
			{
			}

			std::vector<minijava::token> operator()()
			{
				_tokens.clear();
				_gen_program();
				return std::move(_tokens);
			}

		private:

			void _gen_program()
			{
				const auto guard = _enter_nested();
				while (_nest_deeper_eh()) {
					_gen_class_decl();
				}
				_tokens.push_back(minijava::token::create(tt::eof));
			}

			void _gen_class_decl()
			{
				const auto guard = _enter_nested();
				_push(tt::kw_class);
				_push_Id();
				_push(tt::left_brace);
				while (_nest_deeper_eh()) {
					_gen_class_member();
				}
				_push(tt::right_brace);
			}

			void _gen_class_member()
			{
				const auto guard = _enter_nested();
				std::uniform_int_distribution<int> dist{0, 5};
				switch(dist(_engine)) {
				case 0:
				case 1:
					_gen_field();
					return;
				case 2:
				case 3:
					_gen_method();
					return;
				default:
					_gen_main_method();
					return;
				}
			}

			void _gen_field()
			{
				const auto guard = _enter_nested();
				_push(tt::kw_public);
				_gen_type();
				_push_id();
				_push(tt::semicolon);
			}

			void _gen_main_method()
			{
				const auto guard = _enter_nested();
				_push(tt::kw_public);
				_push(tt::kw_static);
				_push(tt::kw_void);
				_push_id();
				_push(tt::left_paren);
				_push_id("String");
				_push(tt::left_bracket);
				_push(tt::right_bracket);
				_push_id();
				_push(tt::right_paren);
				_gen_block();
			}

			void _gen_method()
			{
				const auto guard = _enter_nested();
				_push(tt::kw_public);
				_gen_type();
				_push_id();
				_push(tt::left_paren);
				if (_nest_deeper_eh()) {
					_gen_parameters();
				}
				_push(tt::right_paren);
				_gen_block();
			}

			void _gen_parameters()
			{
				const auto guard = _enter_nested();
				_gen_parameter();
				if (_nest_deeper_eh()) {
					_push(tt::comma);
					_gen_parameters();
				}
			}

			void _gen_parameter()
			{
				const auto guard = _enter_nested();
				_gen_type();
				_push_id();
			}

			void _gen_type()
			{
				const auto guard = _enter_nested();
				if (_nest_deeper_eh()) {
					_gen_type();
					_push(tt::left_bracket);
					_push(tt::right_bracket);
				} else {
					_gen_basic_type();
				}
			}

			void _gen_basic_type()
			{
				const auto guard = _enter_nested();
				auto dist = std::uniform_int_distribution<int>{0, 4};
				switch(dist(_engine)) {
				case 0:
					_push(tt::kw_int);
					return;
				case 1:
					_push(tt::kw_boolean);
					return;
				case 2:
					_push(tt::kw_void);
					return;
				default:
					_push_Id();
					return;
				}
			}

			void _gen_statement()
			{
				const auto guard = _enter_nested();
				auto dist = std::uniform_int_distribution<int>{0, 5};
				switch(dist(_engine)) {
				case 0:
					_gen_block();
					return;
				case 1:
					_gen_empty_statement();
					return;
				case 2:
					_gen_if_statement();
					return;
				case 3:
					_gen_expression_statement();
					return;
				case 4:
					_gen_while_statement();
					return;
				case 5:
					_gen_return_statement();
					return;
				}
			}

			void _gen_block()
			{
				const auto guard = _enter_nested();
				_push(tt::left_brace);
				while (_nest_deeper_eh()) {
					_gen_block_statement();
				}
				_push(tt::right_brace);
			}

			void _gen_block_statement()
			{
				const auto guard = _enter_nested();
				auto dist = std::bernoulli_distribution{0.7};
				if (dist(_engine)) {
					_gen_statement();
				} else {
					_gen_local_variable_statement();
				}
			}

			void _gen_local_variable_statement()
			{
				const auto guard = _enter_nested();
				_gen_type();
				_push_id();
				if (_nest_deeper_eh()) {
					_push(tt::assign);
					_gen_expression();
				}
				_push(tt::semicolon);
			}

			void _gen_empty_statement()
			{
				const auto guard = _enter_nested();
				_push(tt::semicolon);
			}

			void _gen_while_statement()
			{
				const auto guard = _enter_nested();
				_push(tt::kw_while);
				_push(tt::left_paren);
				_gen_expression();
				_push(tt::right_paren);
				_gen_statement();
			}

			void _gen_if_statement()
			{
				const auto guard = _enter_nested();
				_push(tt::kw_if);
				_push(tt::left_paren);
				_gen_expression();
				_push(tt::right_paren);
				_gen_statement();
				if (_nest_deeper_eh()) {
					_push(tt::kw_else);
					_gen_statement();
				}
			}

			void _gen_expression_statement()
			{
				const auto guard = _enter_nested();
				_gen_expression();
				_push(tt::semicolon);
			}

			void _gen_return_statement()
			{
				const auto guard = _enter_nested();
				_push(tt::kw_return);
				if (_nest_deeper_eh()) {
					_gen_expression();
				}
				_push(tt::semicolon);
			}

			void _gen_expression()
			{
				const auto guard = _enter_nested();
				_gen_assignment_expression();
			}

			void _gen_assignment_expression()
			{
				const auto guard = _enter_nested();
				_gen_logical_or_expression();
				if (_nest_deeper_eh()) {
					_push(tt::assign);
					_gen_assignment_expression();
				}
			}

			void _gen_logical_or_expression()
			{
				const auto guard = _enter_nested();
				if (_nest_deeper_eh()) {
					_gen_logical_or_expression();
					_push(tt::logical_or);
				}
				_gen_logical_and_expression();
			}

			void _gen_logical_and_expression()
			{
				const auto guard = _enter_nested();
				if (_nest_deeper_eh()) {
					_gen_logical_and_expression();
					_push(tt::logical_and);
				}
				_gen_equality_expression();
			}

			void _gen_equality_expression()
			{
				const auto guard = _enter_nested();
				if (_nest_deeper_eh()) {
					_gen_equality_expression();
					_push_one(tt::equal, tt::not_equal);
				}
				_gen_relational_expression();
			}

			void _gen_relational_expression()
			{
				const auto guard = _enter_nested();
				if (_nest_deeper_eh()) {
					_gen_relational_expression();
					_push_one(tt::less_than, tt::less_equal, tt::greater_than, tt::greater_equal);
				}
				_gen_additive_expression();
			}

			void _gen_additive_expression()
			{
				const auto guard = _enter_nested();
				if (_nest_deeper_eh()) {
					_gen_additive_expression();
					_push_one(tt::plus, tt::minus);
				}
				_gen_multiplicative_expression();
			}

			void _gen_multiplicative_expression()
			{
				const auto guard = _enter_nested();
				if (_nest_deeper_eh()) {
					_gen_multiplicative_expression();
					_push_one(tt::multiply, tt::divides, tt::modulo);
				}
				_gen_unary_expression();
			}

			void _gen_unary_expression()
			{
				const auto guard = _enter_nested();
				if (_nest_deeper_eh()) {
					_push_one(tt::logical_not, tt::minus);
					_gen_unary_expression();
				} else {
					_gen_postfix_expression();
				}
			}

			void _gen_postfix_expression()
			{
				const auto guard = _enter_nested();
				_gen_primary_expression();
				while (_nest_deeper_eh()) {
					_gen_postfix_op();
				}
			}

			void _gen_postfix_op()
			{
				const auto guard = _enter_nested();
				auto dist = std::uniform_int_distribution<int>{0, 2};
				switch (dist(_engine)) {
				case 0:
					_gen_method_invokation();
					return;
				case 1:
					_gen_field_access();
					return;
				case 2:
					_gen_array_access();
					return;
				default:
					MINIJAVA_NOT_REACHED();
				}
			}

			void _gen_method_invokation()
			{
				const auto guard = _enter_nested();
				_push(tt::dot);
				_push_id();
				_push(tt::left_paren);
				_gen_arguments();
				_push(tt::right_paren);
			}

			void _gen_field_access()
			{
				const auto guard = _enter_nested();
				_push(tt::dot);
				_push_id();
			}

			void _gen_array_access()
			{
				const auto guard = _enter_nested();
				_push(tt::left_bracket);
				_gen_expression();
				_push(tt::right_bracket);
			}

			void _gen_arguments()
			{
				const auto guard = _enter_nested();
				if (_nest_deeper_eh()) {
					_gen_expression();
					while (_nest_deeper_eh()) {
						_push(tt::comma);
						_gen_expression();
					}
				}
			}

			void _gen_primary_expression()
			{
				const auto guard = _enter_nested();
				auto dist = std::uniform_int_distribution<int>{0, 9};
				switch (dist(_engine)) {
				case 0:
					_push(tt::kw_null);
					return;
				case 1:
					_push(tt::kw_false);
					return;
				case 2:
					_push(tt::kw_true);
					return;
				case 3:
					_push_lit();
					return;
				case 4:
					_push_id();
					return;
				case 5:
					_push_id();
					_push(tt::left_paren);
					_gen_arguments();
					_push(tt::right_paren);
					return;
				case 6:
					_push(tt::kw_this);
					return;
				case 7:
					_push(tt::left_paren);
					_gen_expression();
					_push(tt::right_paren);
					return;
				case 8:
					_gen_new_obj_expression();
					return;
				case 9:
					_gen_new_array_expression();
					return;
				}
			}

			void _gen_new_obj_expression()
			{
				const auto guard = _enter_nested();
				_push(tt::kw_new);
				_push_Id();
				_push(tt::left_paren);
				_push(tt::right_paren);
			}

			void _gen_new_array_expression()
			{
				const auto guard = _enter_nested();
				auto dist = std::geometric_distribution<int>{1.0};
				auto rank = dist(_engine);
				_push(tt::kw_new);
				_gen_basic_type();
				_push(tt::left_bracket);
				_gen_expression();
				_push(tt::right_bracket);
				while (rank--) {
					_push(tt::left_bracket);
					_push(tt::right_bracket);
				}
			}

			template<typename... T>
			void _push_one(T... args)
			{
				static_assert(sizeof...(args) > 0, "");
				const tt tts[] = { args... };
				auto dist = std::uniform_int_distribution<std::size_t>{0, sizeof...(args) - 1};
				_push(tts[dist(_engine)]);
			}

			void _push_id()
			{
				const auto id = get_random_identifier(_engine);
				_push_id(id);
			}

			void _push_Id()
			{
				const auto id = get_random_identifier(_engine);
				_push_id(id);
			}

			void _push_id(const std::string& id)
			{
				const auto canon = _pool.normalize(id);
				_tokens.push_back(minijava::token::create_identifier(canon));
			}

			void _push_lit()
			{
				const auto lit = get_random_integer_literal(_engine);
				const auto canon = _pool.normalize(lit);
				_tokens.push_back(minijava::token::create_integer_literal(canon));
			}

			void _push(const tt type)
			{
				_tokens.push_back(minijava::token::create(type));
			}

			bool _nest_deeper_eh()
			{
				if (_nest_depth >= _nest_limit) {
					return false;
				}
				const auto p = 1.0 - std::sqrt(
					static_cast<double>(_nest_depth) / static_cast<double>(_nest_limit)
				);
				auto dist = std::bernoulli_distribution{p};
				return dist(_engine);
			}

			restore_finally<std::size_t> _enter_nested()
			{
				return {_nest_depth, _nest_depth + 1};
			}

			RndEngT& _engine;

			PoolT& _pool;

			std::vector<minijava::token> _tokens{};

			std::size_t _nest_depth{};

			std::size_t _nest_limit{};

		};  // struct syntaxgen

	}  // namespace detail

	template <typename RndEngT, typename PoolT>
	std::vector<minijava::token> generate_valid_program(RndEngT& engine, PoolT& pool, const std::size_t depth)
	{
		auto gen = detail::syntaxgen<RndEngT, PoolT>{engine, pool, depth};
		return gen();
	}

}  // namespace minijava
