/**
 * @file token_type.hpp
 *
 * @brief
 *     Lexical tokens types.
 *
 */

#pragma once

#include <array>
#include <cstdint>
#include <iosfwd>


namespace minijava
{

	/**
	 * @brief
	 *     Categories for token types.
	 *
	 */
	enum class token_category : std::uint16_t
	{
		identifier    = 1,  ///< identifiers
		literal       = 2,  ///< literals
		keyword       = 3,  ///< keywords
		punctuation   = 4,  ///< operators and other punctuation symbols
		synthetic     = 5,  ///< end-of-input, error, and the like
	};

	/**
	 * @brief
	 *     Types of recognized tokens.
	 *
	 */
	enum class token_type : std::uint16_t
	{
		// The four most significant bits are used to encode the token category.
		identifier          = 0x1000,  ///< identifier (for example &ldquo;`foo`&rdquo;)
		integer_literal     = 0x2000,  ///< integer literal (for example &ldquo;`42`&rdquo;)
		kw_abstract         = 0x3000,  ///< keyword &ldquo;`abstract`&rdquo;
		kw_assert,                     ///< keyword &ldquo;`assert`&rdquo;
		kw_boolean,                    ///< keyword &ldquo;`boolean`&rdquo;
		kw_break,                      ///< keyword &ldquo;`break`&rdquo;
		kw_byte,                       ///< keyword &ldquo;`byte`&rdquo;
		kw_case,                       ///< keyword &ldquo;`case`&rdquo;
		kw_catch,                      ///< keyword &ldquo;`catch`&rdquo;
		kw_char,                       ///< keyword &ldquo;`char`&rdquo;
		kw_class,                      ///< keyword &ldquo;`class`&rdquo;
		kw_const,                      ///< keyword &ldquo;`const`&rdquo;
		kw_continue,                   ///< keyword &ldquo;`continue`&rdquo;
		kw_default,                    ///< keyword &ldquo;`default`&rdquo;
		kw_double,                     ///< keyword &ldquo;`double`&rdquo;
		kw_do,                         ///< keyword &ldquo;`do`&rdquo;
		kw_else,                       ///< keyword &ldquo;`else`&rdquo;
		kw_enum,                       ///< keyword &ldquo;`enum`&rdquo;
		kw_extends,                    ///< keyword &ldquo;`extends`&rdquo;
		kw_false,                      ///< keyword &ldquo;`false`&rdquo;
		kw_finally,                    ///< keyword &ldquo;`finally`&rdquo;
		kw_final,                      ///< keyword &ldquo;`final`&rdquo;
		kw_float,                      ///< keyword &ldquo;`float`&rdquo;
		kw_for,                        ///< keyword &ldquo;`for`&rdquo;
		kw_goto,                       ///< keyword &ldquo;`goto`&rdquo;
		kw_if,                         ///< keyword &ldquo;`if`&rdquo;
		kw_implements,                 ///< keyword &ldquo;`implements`&rdquo;
		kw_import,                     ///< keyword &ldquo;`import`&rdquo;
		kw_instanceof,                 ///< keyword &ldquo;`instanceof`&rdquo;
		kw_interface,                  ///< keyword &ldquo;`interface`&rdquo;
		kw_int,                        ///< keyword &ldquo;`int`&rdquo;
		kw_long,                       ///< keyword &ldquo;`long`&rdquo;
		kw_native,                     ///< keyword &ldquo;`native`&rdquo;
		kw_new,                        ///< keyword &ldquo;`new`&rdquo;
		kw_null,                       ///< keyword &ldquo;`null`&rdquo;
		kw_package,                    ///< keyword &ldquo;`package`&rdquo;
		kw_private,                    ///< keyword &ldquo;`private`&rdquo;
		kw_protected,                  ///< keyword &ldquo;`protected`&rdquo;
		kw_public,                     ///< keyword &ldquo;`public`&rdquo;
		kw_return,                     ///< keyword &ldquo;`return`&rdquo;
		kw_short,                      ///< keyword &ldquo;`short`&rdquo;
		kw_static,                     ///< keyword &ldquo;`static`&rdquo;
		kw_strictfp,                   ///< keyword &ldquo;`strictfp`&rdquo;
		kw_super,                      ///< keyword &ldquo;`super`&rdquo;
		kw_switch,                     ///< keyword &ldquo;`switch`&rdquo;
		kw_synchronized,               ///< keyword &ldquo;`synchronized`&rdquo;
		kw_this,                       ///< keyword &ldquo;`this`&rdquo;
		kw_throws,                     ///< keyword &ldquo;`throws`&rdquo;
		kw_throw,                      ///< keyword &ldquo;`throw`&rdquo;
		kw_transient,                  ///< keyword &ldquo;`transient`&rdquo;
		kw_true,                       ///< keyword &ldquo;`true`&rdquo;
		kw_try,                        ///< keyword &ldquo;`try`&rdquo;
		kw_void,                       ///< keyword &ldquo;`void`&rdquo;
		kw_volatile,                   ///< keyword &ldquo;`volatile`&rdquo;
		kw_while,                      ///< keyword &ldquo;`while`&rdquo;
		not_equal,                     ///< inequality operator &ldquo;`!=`&rdquo;
		logical_not,                   ///< logical not &ldquo;`!`&rdquo; operator &ldquo;`!`&rdquo;
		left_paren,                    ///< left parenthesis &ldquo;`(`&rdquo;
		right_paren,                   ///< right parenthesis &ldquo;`)`&rdquo;
		multiply_assign     = 0x4000,  ///< multiply-assignment operator &ldquo;`*=`&rdquo;
		multiply,                      ///< multiplication operator &ldquo;`*`&rdquo;
		increment,                     ///< increment operator &ldquo;`++`&rdquo;
		plus_assign,                   ///< plus-assignment operator &ldquo;`+=`&rdquo;
		plus,                          ///< plus operator &ldquo;`+`&rdquo;
		comma,                         ///< comma operator &ldquo;`,`&rdquo;
		minus_assign,                  ///< minus-assignment operator &ldquo;`-=`&rdquo;
		decrement,                     ///< decrement operator &ldquo;`--`&rdquo;
		minus,                         ///< minus operator &ldquo;`-`&rdquo;
		dot,                           ///< dot operator &ldquo;`.`&rdquo;
		divides_assign,                ///< divide-assignment operator &ldquo;`/=`&rdquo;
		divides,                       ///< division operator &ldquo;`/`&rdquo;
		colon,                         ///< colon operator &ldquo;`:`&rdquo;
		semicolon,                     ///< semicolon &ldquo;`;`&rdquo;
		left_shift_assign,             ///< arithmetic left-shift-assignemt operator &ldquo;`<<=`&rdquo;
		left_shift,                    ///< arithmetic left-shift operator &ldquo;`<<`&rdquo;
		less_equal,                    ///< less-than-or-equal operator &ldquo;`<=`&rdquo;
		less_than,                     ///< less-than operator &ldquo;`<`&rdquo;
		equal,                         ///< equality operator &ldquo;`==`&rdquo;
		assign,                        ///< assignment operator &ldquo;`=`&rdquo;
		greater_equal,                 ///< greater-than-or-equal operator &ldquo;`>=`&rdquo;
		right_shift_assign,            ///< arithmetic right-shift-assignment operator &ldquo;`>>=`&rdquo;
		right_shift,                   ///< arithmetic right-shift operator &ldquo;`>>`&rdquo;
		unsigned_right_shift_assign,   ///< logical right-shift-assignment operator &ldquo;`>>>=`&rdquo;
		unsigned_right_shift,          ///< logical right-shift operator &ldquo;`>>>`&rdquo;
		greater_than,                  ///< greater-than operator &ldquo;`>`&rdquo;
		conditional,                   ///< conditional operator &ldquo;`?`&rdquo;
		modulo_assign,                 ///< modulus-assignment operator &ldquo;`%=`&rdquo;
		modulo,                        ///< modulus operator &ldquo;`%`&rdquo;
		bit_and_assign,                ///< bit-wise and-assignemt operator &ldquo;`&=`&rdquo;
		logical_and,                   ///< logical and operator &ldquo;`&&`&rdquo;
		bit_and,                       ///< bit-wise and operator &ldquo;`&`&rdquo;
		left_bracket,                  ///< left square bracket &ldquo;`[`&rdquo;
		right_bracket,                 ///< right square bracket &ldquo;`]`&rdquo;
		bit_xor_assign,                ///< bit-wise exclusive-or-assignment operator &ldquo;`^=`&rdquo;
		bit_xor,                       ///< bit-wise exclusive-or operator &ldquo;`^`&rdquo;
		left_brace,                    ///< left curly brace &ldquo;`{`&rdquo;
		right_brace,                   ///< right curly brace &ldquo;`}`&rdquo;
		bit_not,                       ///< bit-wise negation operator &ldquo;`~`&rdquo;
		bit_or_assign,                 ///< bit-wise or-assignment-operator operator &ldquo;`|=`&rdquo;
		logical_or,                    ///< logical or operator operator &ldquo;`||`&rdquo;
		bit_or,                        ///< bit-wise or operator &ldquo;`|`&rdquo;
		eof                 = 0x5000,  ///< end of input
	};


	/**
	 * @brief
	 *     `return`s the category of a token type.
	 *
	 * If `tt` is not a valid enumerator of `token_type`, then the `return`ed
	 * value won't be a valid enumerator of `token_category` either.  The
	 * behavior is well-defined in any case.
	 *
	 * @param tt
	 *     `token_type` to test
	 *
	 * @returns
	 *     token category of `tt`
	 *
	 */
	constexpr token_category category(token_type tt) noexcept;


	/**
	 * @brief
	 *     `return`s a reference to an array with all values of the
	 *     `token_type` `enum`.
	 *
	 * The array is statically allocated so the reference remains valid
	 * throughout the life-time of the program.
	 *
	 * @returns
	 *     reference to array with all enumerator values
	 *
	 */
	const std::array<token_type, 102>& all_token_types() noexcept;


	/**
	 * @brief
	 *     `return`s a textual representation of a token type.
	 *
	 * The representation will be
	 *
	 *  - the string `identifier` for identifiers,
	 *  - the string `integer literal` for integer literals,
	 *  - the name of the keyword or operator for keywords and operators,
	 *  - the string `EOF` for end-of-input and
	 *  - the `nullptr` for invalid token types.
	 *
	 * The `return`ed pointer is the address of a statically allocated
	 * read-only buffer and must not be modified or deallocated.
	 *
	 * @param tt
	 *     token type to get the name of
	 *
	 * @returns
	 *     textual representation or `nullptr`
	 *
	 */
	constexpr const char * name(token_type tt) noexcept;

	/**
	 * @brief
	 *     `return`s a textual representation of a token category.
	 *
	 * The `return`ed pointer is the address of a statically allocated
	 * read-only buffer and must not be modified or deallocated.  If the input
	 * is not a valid enumerator, the `nullptr` will be `return`ed.
	 *
	 * @param cat
	 *     token category to get the name of
	 *
	 * @returns
	 *     textual representation or `nullptr`
	 *
	 */
	constexpr const char * name(token_category cat) noexcept;

	/**
	 * @brief
	 *     Inserts the textual representation of a token type into an output
	 *     stream.
	 *
	 * If the token type is invalid, a special string will be inserted that
	 * includes the numeric value of the argument.  The `failbit` won't be set.
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
