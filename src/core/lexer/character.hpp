/**
 * @file character.hpp
 *
 * @brief
 *     Character classification functions similar to those in `<ctype.h>` but
 *     targeted at the MiniJava language.
 *
 * All functions provided in this header take an `int` as argument and `return`
 * `bool`.  The argument is interpreted as the integral value of a byte cast to
 * `unsigned char`.  That is, meaningful inputs are values in the range 0x00 to
 * 0xff (both inclusive).  For all other inputs, these funtions wil always
 * `return` `false`.
 *
 * A *word* as used in the documentation of some functions, is either an
 * identifier or a keyword as defined by the MiniJava lexical rules.
 *
 * The functions provided by this header have underscores in their names (eg
 * `is_digit`) to avoid calling the standard library functions (eg `isdigit`)
 * by accident from inside the `minijava` `namespace`.
 *
 */

#pragma once


namespace minijava
{

	/**
	 * @brief
	 *     Tests whether a byte is a space character as defined by the MiniJava
	 *     lexical rules.
	 *
	 * @param c
	 *     byte cast to `unsigned char`
	 *
	 * @returns
	 *     whether `c` is a space character
	 *
	 */
	bool is_space(int c) noexcept;

	/**
	 * @brief
	 *     Tests whether a byte is a decimal digit as defined by the MiniJava
	 *     lexical rules.
	 *
	 * @param c
	 *     byte cast to `unsigned char`
	 *
	 * @returns
	 *     whether `c` is a decimal digit
	 *
	 */
	bool is_digit(int c) noexcept;

	/**
	 * @brief
	 *     Tests whether a byte is a valid first character of a word as defined
	 *     by the MiniJava lexical rules.
	 *
	 * @param c
	 *     byte cast to `unsigned char`
	 *
	 * @returns
	 *     whether `c` is a valid first character of a word
	 *
	 */
	bool is_word_head(int c) noexcept;

	/**
	 * @brief
	 *     Tests whether a byte is a valid following character of a word as
	 *     defined by the MiniJava lexical rules.
	 *
	 * @param c
	 *     byte cast to `unsigned char`
	 *
	 * @returns
	 *     whether `c` is a valid following character of a word
	 *
	 */
	bool is_word_tail(int c) noexcept;

}  // namespace minijava


#define MINIJAVA_INCLUDED_FROM_LEXER_CHARACTER_HPP
#include "lexer/character.tpp"
#undef MINIJAVA_INCLUDED_FROM_LEXER_CHARACTER_HPP
