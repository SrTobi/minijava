/**
 * @brief
 *     Lexical tokens types.
 *
 */

#pragma once

#include <iosfwd>


namespace minijava
{

	/**
	 * @brief
	 *     Types of recognized tokens.
	 *
	 */
	enum class token_type : short
	{
		// TODO @Moritz Klammler: Complete this list.
		identifier,
		integer_literal,
		kw_if,
		kw_else,
		left_paren,
		right_paren,
		eof,
        unknown,
		// ...
	};

	/**
	 * @brief
	 *     `return`s a &ldquo;fancy name&rdquo; for a token type as required in
	 *     the `--lextest` output.
	 *
	 * The `return`ed pointer is the address of a statically allocated
	 * read-only buffer and must not be modified or deallocated.
	 *
	 * @param tt
	 *     token type to get the name of
	 *
	 * @returns
	 *     fancy name or `nullptr` if the type is invalid
	 *
	 */
	constexpr const char * fancy_name(token_type tt) noexcept;

	/**
	 * @brief
	 *     Inserts the &ldquo;fancy name&rdquo; of a token type into an output
	 *     stream.
	 *
	 * If the token type is invalid, a not-so-fancy string will be inserted.
	 *
	 * @param os
	 *     stream to write to
	 *
	 * @param tt
	 *     token type to insert
	 *
	 * @returns
	 *     reference to `os`
	 *
	 */
	std::ostream& operator<<(std::ostream& os, token_type tt);

}  // namespace minijava


#define MINIJAVA_INCLUDED_FROM_LEXER_TOKEN_TYPE_HPP
#include "lexer/token_type.tpp"
#undef MINIJAVA_INCLUDED_FROM_LEXER_TOKEN_TYPE_HPP
