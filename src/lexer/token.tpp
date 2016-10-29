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

	template <typename DataT>
	token::token(const token_type type, DataT&& data) :
		_type{type}, _data{std::forward<DataT>(data)}
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
		return token{tt, boost::blank{}};
	}

	inline token_type token::type() const noexcept
	{
		return _type;
	}

	inline symbol token::lexval() const
	{
		assert(has_lexval());
		return boost::get<symbol>(_data);
	}

	inline bool token::has_lexval() const noexcept
	{
		return detail::has_lexval(type());
	}

	inline std::size_t token::line() const noexcept
	{
		return _line;
	}

	inline std::size_t token::column() const noexcept
	{
		return _column;
	}

	inline void token::set_line(const std::size_t line) noexcept
	{
		_line = line;
	}

	inline void token::set_column(const std::size_t column) noexcept
	{
		_column = column;
	}

	inline bool token::equal(const token& lhs, const token& rhs) noexcept
	{
		return ((lhs._type == rhs._type) && (lhs._data == rhs._data));
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
