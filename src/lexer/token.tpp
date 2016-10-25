#ifndef MINIJAVA_INCLUDED_FROM_LEXER_TOKEN_HPP
#error "Never `#include <lexer/token.tpp>` directly; `#include <token.hpp>` instead."
#endif

#include <cassert>
#include <utility>


namespace minijava
{

	template <typename DataT>
	token::token(const token_type type, DataT&& data) :
		_type{type}, _data{std::forward<DataT>(data)}
	{
	}

	inline token token::create_identifier(string_type name)
	{
		return token{token_type::identifier, std::move(name)};
	}

	inline token token::create_integer_literal(integer_type value)
	{
		return token{token_type::integer_literal, std::move(value)};
	}

	inline token token::create(const token_type tt)
	{
		assert(tt != token_type::identifier);
		assert(tt != token_type::integer_literal);
		return token{tt, boost::blank{}};
	}

	inline token_type token::type() const noexcept
	{
		return _type;
	}

	inline token::string_type token::name() const
	{
		assert(_type == token_type::identifier);
		return boost::get<string_type>(_data);
	}

	inline token::integer_type token::value() const
	{
		assert(_type == token_type::integer_literal);
		return boost::get<integer_type>(_data);
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
		return (lhs._type == rhs._type) && (lhs._data == rhs._data);
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
