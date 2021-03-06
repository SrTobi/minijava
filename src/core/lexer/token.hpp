/**
 * @file token.hpp
 *
 * @brief
 *     Lexical tokens.
 *
 */

#pragma once

#include <cstddef>
#include <iosfwd>

#include "lexer/token_type.hpp"
#include "symbol/symbol.hpp"
#include "position.hpp"

namespace minijava
{

	/**
	 * @brief
	 *     A lexical token.
	 *
	 * A `token` may store a `symbol`.  The behavior of any operation on a
	 * token All operations that are defined on multiple `token`s (such as
	 * comparison) have undefined behavior if the involved `token`s store
	 * `symbol`s from different pools.  The only exception to this rule is the
	 * assignment operator.
	 *
	 */
	class token final
	{
	private:

		/**
		 * @brief
		 *     Unchecked constructor.
		 *
		 * @tparam DataT
		 *     `symbol` or `boost::blank`
		 *
		 * @param type
		 *     token type
		 *
		 * @param lexval
		 *     lexical value of the token or empty symbol
		 *
		 */
		token(token_type type, symbol lexval = symbol{});

	public:

		/**
		 * @brief
		 *     Creates a token for an identifier.
		 *
		 * If `lexval` is not a valid identifier, the behavior is undefined.
		 *
		 * @param lexval
		 *     name of the identifier
		 *
		 * @returns
		 *     identifier token
		 *
		 */
		static token create_identifier(symbol lexval);

		/**
		 * @brief
		 *     Creates a token for an integer literal.
		 *
		 * If `lexval` is not a valid integer literal, the behavior is
		 * undefined.
		 *
		 * @param lexval
		 *     digits of the integer literal
		 *
		 * @returns
		 *     integer literal token
		 *
		 */
		static token create_integer_literal(symbol lexval);

		/**
		 * @brief
		 *     Creates a token without an associated lexical value.
		 *
		 * If tokens of type `tt` actually have an associated lexical value,
		 * the behavior is undefined.
		 *
		 * @param tt
		 *     type of the token
		 *
		 * @returns
		 *     token of the specified type
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
		 *     `return`s the associated lexical value of the token.
		 *
		 * If the token is of a type that does not have an associated lexical
		 * value, the behavior is undefined.
		 *
		 * @returns
		 *     lexical value
		 *
		 */
		symbol lexval() const;

		/**
		 * @brief
		 *     `return`s whether the token has an associated lexical value.
		 *
		 * This function `return`s `true` only if the `token_category` of
		 * `type()` is `identifier` or `literal`.
		 *
		 * @returns
		 *     whether the token has an associated lexical value
		 *
		 */
		bool has_lexval() const noexcept;

		/**
		 * @brief
		 *     `return`s the position where the token was found.
		 *
		 * If the value is 0, the position number is unknown.
		 *
		 * @returns
		 *     position
		 *
		 */
		minijava::position position() const noexcept;

		/**
		 * @brief
		 *     Associates a position with the token.
		 *
		 * @param pos
		 *     position
		 *
		 */
		void set_position(minijava::position pos) noexcept;

		/**
		 * @brief
		 *     Tests whether two tokens are equal.
		 *
		 * Two tokens are considered equal if they are of the same type and --
		 * if tokens of that type have an associated lexical value -- their
		 * associated lexical values compare equal, too.  The source code
		 * location does not participate in the comparison.
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
		token_type _type{};

		/** @brief Lexical value associated with the token. */
		symbol _lexval{};

		/** @brief Position where the token was found. */
		minijava::position _position;


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
	 *     `!#token::equal(lhs, rhs)`
	 *
	 */
	bool operator!=(const token& lhs, const token& rhs) noexcept;

	/**
	 * @brief
	 *     Inserts a textual representation of a token into an output stream.
	 *
	 * The textual representation consists of the textual representation of the
	 * `token`'s type and -- if the token has an associated lexical value -- is
	 * followed by a single space character and the lexical value.
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
