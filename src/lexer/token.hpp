/**
 * @file token.hpp
 *
 * @brief
 *     Lexical tokens.
 *
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <iosfwd>

#include <boost/variant.hpp>

#include "lexer/token_type.hpp"
#include "symbol.hpp"


namespace minijava
{

	/**
	 * @brief
	 *     A lexical token.
	 *
	 */
	class token final
	{
	public:

		/** @brief Type used to represent identifier names. */
		using string_type = symbol;

		/** @brief Type used to represent integer literal values. */
		using integer_type = std::uint32_t;

	private:

		/**
		 * @brief
		 *     Unchecked constructor.
		 *
		 * If a token of this type does not actually hold data of that type,
		 * the behavior is undefined.
		 *
		 * @tparam DataT
		 *     type of the data the token holds
		 *
		 * @param type
		 *     token type
		 *
		 * @param data
		 *     data held by this token
		 *
		 */
		template <typename DataT>
		token(token_type type, DataT&& data);

	public:

		/**
		 * @brief
		 *     Creates a token for an identifier.
		 *
		 * @param name
		 *     name of the identifier
		 *
		 */
		static token create_identifier(string_type name);

		/**
		 * @brief
		 *     Creates a token for an integer literal.
		 *
		 * @param value
		 *     value of the literal
		 *
		 */
		static token create_integer_literal(integer_type value);

		/**
		 * @brief
		 *     Creates any other token that holds no data.
		 *
		 * If `tt` is `token_type::identifier` or
		 * `token_type::integer_literal`, the behavior is undefined.
		 *
		 * @param tt
		 *     type of the token
		 *
		 */
		static token create(token_type tt);

		/**
		 * @brief
		 *     `return`s the type of the token.
		 *
		 * @returns
		 *     token type
		 *
		 */
		token_type type() const noexcept;

		/**
		 * @brief
		 *     `return`s the name of an identifier token.
		 *
		 * The behavior is undefined unless `type() == token_type::identifier`.
		 *
		 * @returns
		 *     name of the identifier
		 *
		 */
		string_type name() const;

		/**
		 * @brief
		 *     `return`s the value of an integer literal token.
		 *
		 * The behavior is undefined unless `type() ==
		 * token_type::integer_literal`.
		 *
		 * @returns
		 *     value of the integer
		 *
		 */
		integer_type value() const;

		/**
		 * @brief
		 *     `return`s the line number where the token was found.
		 *
		 * If the value is 0, the line number is unknown.
		 *
		 * @returns
		 *     line number
		 *
		 */
		std::size_t line() const noexcept;

		/**
		 * @brief
		 *     `return`s the column number where the token was found.
		 *
		 * If the value is 0, the column number is unknown.
		 *
		 * @returns
		 *     column number
		 *
		 */
		std::size_t column() const noexcept;

		/**
		 * @brief
		 *     Associates a line number with the token.
		 *
		 * @param line
		 *     line number
		 *
		 */
		void set_line(std::size_t line) noexcept;

		/**
		 * @brief
		 *     Associates a column number with the token.
		 *
		 * @param column
		 *     column number
		 *
		 */
		void set_column(std::size_t column) noexcept;

		/**
		 * @brief
		 *     Tests whether two tokens are equal.
		 *
		 * Two tokens are considered equal if they are of the same type and
		 * their data compares equal, too.  The source code location (line and
		 * column number) does not participate in the comparison.
		 *
		 * @param lhs
		 *     first token to comare
		 *
		 * @param rhs
		 *     second token to comare
		 *
		 * @returns
		 *     whether the tokens are equal
		 *
		 */
		static bool equal(const token& lhs, const token& rhs) noexcept;

	private:

		/** @brief Type of the token. */
		token_type _type {};

		/** @brief Data held by the token. */
		boost::variant<boost::blank, string_type, integer_type> _data {};

		/** @brief Line number where the token was found. */
		std::size_t _line {};

		/** @brief Column number where the token was found. */
		std::size_t _column {};

	};  // class token


	/**
	 * @brief
	 *     Compares two `token`s for equality.
	 *
	 * @param lhs
	 *     first `token` to compare
	 *
	 * @param rhs
	 *     second `token` to compare
	 *
	 * @returns
	 *     `token::equal(lhs, rhs)`
	 *
	 */
	bool operator==(const token& lhs, const token& rhs) noexcept;

	/**
	 * @brief
	 *     Compares two `token`s for inequality.
	 *
	 * @param lhs
	 *     first `token` to compare
	 *
	 * @param rhs
	 *     second `token` to compare
	 *
	 * @returns
	 *     `!token::equal(lhs, rhs)`
	 *
	 */
	bool operator!=(const token& lhs, const token& rhs) noexcept;

	/**
	 * @brief
	 *     Inserts a &ldquo;fancy representation&rdquo; of a token into an
	 *     output stream.
	 *
	 * @param os
	 *     stream to write to
	 *
	 * @param tok
	 *     token to insert
	 *
	 * @returns
	 *     reference to `os`
	 *
	 */
	std::ostream& operator<<(std::ostream& os, const token& tok);


}  // namespace minijava


#define MINIJAVA_INCLUDED_FROM_LEXER_TOKEN_HPP
#include "lexer/token.tpp"
#undef MINIJAVA_INCLUDED_FROM_LEXER_TOKEN_HPP
