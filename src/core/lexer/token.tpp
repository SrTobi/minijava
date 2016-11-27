#ifndef MINIJAVA_INCLUDED_FROM_LEXER_TOKEN_HPP
#error "Never `#include <lexer/token.tpp>` directly; `#include <token.hpp>` instead."
#endif

#include <cassert>
#include <utility>


namespace minijava
{

	namespace detail
	{

		bool is_valid_identifier(symbol lexval) noexcept;

		bool is_valid_integer_literal(symbol lexval) noexcept;

		constexpr bool has_lexval(const token_type tt) noexcept
		{
			const auto cat = category(tt);
			return (cat == token_category::identifier)
				|| (cat == token_category::literal);
		}

	}

	inline token::token(token_type type, symbol lexval)
		: _type{std::move(type)}
		, _lexval{std::move(lexval)}
	{
	}

	inline token token::create_identifier(symbol lexval)
	{
		assert(detail::is_valid_identifier(lexval));
		return token{token_type::identifier, std::move(lexval)};
	}

	inline token token::create_integer_literal(symbol lexval)
	{
		assert(detail::is_valid_integer_literal(lexval));
		return token{token_type::integer_literal, std::move(lexval)};
	}

	inline token token::create(const token_type tt)
	{
		assert(!detail::has_lexval(tt));
		return token{tt};
	}

	inline token_type token::type() const noexcept
	{
		return _type;
	}

	inline symbol token::lexval() const
	{
		assert(has_lexval());
		return _lexval;
	}

	inline bool token::has_lexval() const noexcept
	{
		return detail::has_lexval(type());
	}

	inline minijava::position token::position() const noexcept
	{
		return _position;
	}

	inline void token::set_position(minijava::position position) noexcept
	{
		_position = position;
	}

	inline bool token::equal(const token& lhs, const token& rhs) noexcept
	{
		return ((lhs._type == rhs._type) && (lhs._lexval == rhs._lexval));
	}

	inline bool operator==(const token& lhs, const token& rhs) noexcept
	{
		return token::equal(lhs, rhs);
	}

	inline bool operator!=(const token& lhs, const token& rhs) noexcept
	{
		return not token::equal(lhs, rhs);
	}

}  // namespace minijava
