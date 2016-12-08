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
	 * The numeric values of the enumerator constants should be regarded as an
	 * implementation detail that might change.
	 *
	 */
	enum class token_category : std::uint16_t
	{
		identifier    = 0x1000,  ///< identifiers
		literal       = 0x2000,  ///< literals
		keyword       = 0x3000,  ///< keywords
		punctuation   = 0x4000,  ///< operators and other punctuation symbols
		synthetic     = 0x5000,  ///< end-of-input, error, and the like
	};

	/**
	 * @brief
	 *     Types of recognized tokens.
	 *
	 * The numeric values of the enumerator constants should be regarded as an
	 * implementation detail that might change.
	 *
	 */
	enum class token_type : std::uint16_t
	{
		identifier                     = 0x1000,  ///< identifier (for example &ldquo;`foo`&rdquo;)
		integer_literal                = 0x2001,  ///< integer literal (for example &ldquo;`42`&rdquo;)
		kw_abstract                    = 0x3002,  ///< keyword &ldquo;`abstract`&rdquo;
		kw_assert                      = 0x3003,  ///< keyword &ldquo;`assert`&rdquo;
		kw_boolean                     = 0x3004,  ///< keyword &ldquo;`boolean`&rdquo;
		kw_break                       = 0x3005,  ///< keyword &ldquo;`break`&rdquo;
		kw_byte                        = 0x3006,  ///< keyword &ldquo;`byte`&rdquo;
		kw_case                        = 0x3007,  ///< keyword &ldquo;`case`&rdquo;
		kw_catch                       = 0x3008,  ///< keyword &ldquo;`catch`&rdquo;
		kw_char                        = 0x3009,  ///< keyword &ldquo;`char`&rdquo;
		kw_class                       = 0x300a,  ///< keyword &ldquo;`class`&rdquo;
		kw_const                       = 0x300b,  ///< keyword &ldquo;`const`&rdquo;
		kw_continue                    = 0x300c,  ///< keyword &ldquo;`continue`&rdquo;
		kw_default                     = 0x300d,  ///< keyword &ldquo;`default`&rdquo;
		kw_double                      = 0x300e,  ///< keyword &ldquo;`double`&rdquo;
		kw_do                          = 0x300f,  ///< keyword &ldquo;`do`&rdquo;
		kw_else                        = 0x3010,  ///< keyword &ldquo;`else`&rdquo;
		kw_enum                        = 0x3011,  ///< keyword &ldquo;`enum`&rdquo;
		kw_extends                     = 0x3012,  ///< keyword &ldquo;`extends`&rdquo;
		kw_false                       = 0x3013,  ///< keyword &ldquo;`false`&rdquo;
		kw_finally                     = 0x3014,  ///< keyword &ldquo;`finally`&rdquo;
		kw_final                       = 0x3015,  ///< keyword &ldquo;`final`&rdquo;
		kw_float                       = 0x3016,  ///< keyword &ldquo;`float`&rdquo;
		kw_for                         = 0x3017,  ///< keyword &ldquo;`for`&rdquo;
		kw_goto                        = 0x3018,  ///< keyword &ldquo;`goto`&rdquo;
		kw_if                          = 0x3019,  ///< keyword &ldquo;`if`&rdquo;
		kw_implements                  = 0x301a,  ///< keyword &ldquo;`implements`&rdquo;
		kw_import                      = 0x301b,  ///< keyword &ldquo;`import`&rdquo;
		kw_instanceof                  = 0x301c,  ///< keyword &ldquo;`instanceof`&rdquo;
		kw_interface                   = 0x301d,  ///< keyword &ldquo;`interface`&rdquo;
		kw_int                         = 0x301e,  ///< keyword &ldquo;`int`&rdquo;
		kw_long                        = 0x301f,  ///< keyword &ldquo;`long`&rdquo;
		kw_native                      = 0x3020,  ///< keyword &ldquo;`native`&rdquo;
		kw_new                         = 0x3021,  ///< keyword &ldquo;`new`&rdquo;
		kw_null                        = 0x3022,  ///< keyword &ldquo;`null`&rdquo;
		kw_package                     = 0x3023,  ///< keyword &ldquo;`package`&rdquo;
		kw_private                     = 0x3024,  ///< keyword &ldquo;`private`&rdquo;
		kw_protected                   = 0x3025,  ///< keyword &ldquo;`protected`&rdquo;
		kw_public                      = 0x3026,  ///< keyword &ldquo;`public`&rdquo;
		kw_return                      = 0x3027,  ///< keyword &ldquo;`return`&rdquo;
		kw_short                       = 0x3028,  ///< keyword &ldquo;`short`&rdquo;
		kw_static                      = 0x3029,  ///< keyword &ldquo;`static`&rdquo;
		kw_strictfp                    = 0x302a,  ///< keyword &ldquo;`strictfp`&rdquo;
		kw_super                       = 0x302b,  ///< keyword &ldquo;`super`&rdquo;
		kw_switch                      = 0x302c,  ///< keyword &ldquo;`switch`&rdquo;
		kw_synchronized                = 0x302d,  ///< keyword &ldquo;`synchronized`&rdquo;
		kw_this                        = 0x302e,  ///< keyword &ldquo;`this`&rdquo;
		kw_throws                      = 0x302f,  ///< keyword &ldquo;`throws`&rdquo;
		kw_throw                       = 0x3030,  ///< keyword &ldquo;`throw`&rdquo;
		kw_transient                   = 0x3031,  ///< keyword &ldquo;`transient`&rdquo;
		kw_true                        = 0x3032,  ///< keyword &ldquo;`true`&rdquo;
		kw_try                         = 0x3033,  ///< keyword &ldquo;`try`&rdquo;
		kw_void                        = 0x3034,  ///< keyword &ldquo;`void`&rdquo;
		kw_volatile                    = 0x3035,  ///< keyword &ldquo;`volatile`&rdquo;
		kw_while                       = 0x3036,  ///< keyword &ldquo;`while`&rdquo;
		not_equal                      = 0x4037,  ///< inequality operator &ldquo;`!=`&rdquo;
		logical_not                    = 0x4038,  ///< logical not &ldquo;`!`&rdquo; operator &ldquo;`!`&rdquo;
		left_paren                     = 0x4039,  ///< left parenthesis &ldquo;`(`&rdquo;
		right_paren                    = 0x403a,  ///< right parenthesis &ldquo;`)`&rdquo;
		multiply_assign                = 0x403b,  ///< multiply-assignment operator &ldquo;`*=`&rdquo;
		multiply                       = 0x403c,  ///< multiplication operator &ldquo;`*`&rdquo;
		increment                      = 0x403d,  ///< increment operator &ldquo;`++`&rdquo;
		plus_assign                    = 0x403e,  ///< plus-assignment operator &ldquo;`+=`&rdquo;
		plus                           = 0x403f,  ///< plus operator &ldquo;`+`&rdquo;
		comma                          = 0x4040,  ///< comma operator &ldquo;`,`&rdquo;
		minus_assign                   = 0x4041,  ///< minus-assignment operator &ldquo;`-=`&rdquo;
		decrement                      = 0x4042,  ///< decrement operator &ldquo;`--`&rdquo;
		minus                          = 0x4043,  ///< minus operator &ldquo;`-`&rdquo;
		dot                            = 0x4044,  ///< dot operator &ldquo;`.`&rdquo;
		divides_assign                 = 0x4045,  ///< divide-assignment operator &ldquo;`/=`&rdquo;
		divide                         = 0x4046,  ///< division operator &ldquo;`/`&rdquo;
		colon                          = 0x4047,  ///< colon operator &ldquo;`:`&rdquo;
		semicolon                      = 0x4048,  ///< semicolon &ldquo;`;`&rdquo;
		left_shift_assign              = 0x4049,  ///< arithmetic left-shift-assignemt operator &ldquo;`<<=`&rdquo;
		left_shift                     = 0x404a,  ///< arithmetic left-shift operator &ldquo;`<<`&rdquo;
		less_equal                     = 0x404b,  ///< less-than-or-equal operator &ldquo;`<=`&rdquo;
		less_than                      = 0x404c,  ///< less-than operator &ldquo;`<`&rdquo;
		equal                          = 0x404d,  ///< equality operator &ldquo;`==`&rdquo;
		assign                         = 0x404e,  ///< assignment operator &ldquo;`=`&rdquo;
		greater_equal                  = 0x404f,  ///< greater-than-or-equal operator &ldquo;`>=`&rdquo;
		right_shift_assign             = 0x4050,  ///< arithmetic right-shift-assignment operator &ldquo;`>>=`&rdquo;
		right_shift                    = 0x4051,  ///< arithmetic right-shift operator &ldquo;`>>`&rdquo;
		unsigned_right_shift_assign    = 0x4052,  ///< logical right-shift-assignment operator &ldquo;`>>>=`&rdquo;
		unsigned_right_shift           = 0x4053,  ///< logical right-shift operator &ldquo;`>>>`&rdquo;
		greater_than                   = 0x4054,  ///< greater-than operator &ldquo;`>`&rdquo;
		conditional                    = 0x4055,  ///< conditional operator &ldquo;`?`&rdquo;
		modulo_assign                  = 0x4056,  ///< modulus-assignment operator &ldquo;`%=`&rdquo;
		modulo                         = 0x4057,  ///< modulus operator &ldquo;`%`&rdquo;
		bit_and_assign                 = 0x4058,  ///< bit-wise and-assignemt operator &ldquo;`&=`&rdquo;
		logical_and                    = 0x4059,  ///< logical and operator &ldquo;`&&`&rdquo;
		bit_and                        = 0x405a,  ///< bit-wise and operator &ldquo;`&`&rdquo;
		left_bracket                   = 0x405b,  ///< left square bracket &ldquo;`[`&rdquo;
		right_bracket                  = 0x405c,  ///< right square bracket &ldquo;`]`&rdquo;
		bit_xor_assign                 = 0x405d,  ///< bit-wise exclusive-or-assignment operator &ldquo;`^=`&rdquo;
		bit_xor                        = 0x405e,  ///< bit-wise exclusive-or operator &ldquo;`^`&rdquo;
		left_brace                     = 0x405f,  ///< left curly brace &ldquo;`{`&rdquo;
		right_brace                    = 0x4060,  ///< right curly brace &ldquo;`}`&rdquo;
		bit_not                        = 0x4061,  ///< bit-wise negation operator &ldquo;`~`&rdquo;
		bit_or_assign                  = 0x4062,  ///< bit-wise or-assignment-operator operator &ldquo;`|=`&rdquo;
		logical_or                     = 0x4063,  ///< logical or operator operator &ldquo;`||`&rdquo;
		bit_or                         = 0x4064,  ///< bit-wise or operator &ldquo;`|`&rdquo;
		eof                            = 0x5065,  ///< end of input
	};


	/** @brief Number of defined `token_type`s. */
	constexpr auto total_token_type_count = std::size_t{102};


	/**
	 * @brief
	 *     `return`s a reference to a statically allocated array with all
	 *     declared `token_type` enumerators.
	 *
	 * The order of the enumerators is the declaration order in the `enum`.  It
	 * is guaranteed that the array is sorted with respect to operator `<`.
	 *
	 * The array is statically allocated so the reference remains valid
	 * throughout the life-time of the program but must not be written to.
	 *
	 * @returns
	 *     reference to array with all declared enumerator values
	 *
	 */
	const std::array<token_type, total_token_type_count>& all_token_types() noexcept;


	/**
	 * @brief
	 *     `return`s the category of a token type.
	 *
	 * If `tt` is not a declared enumerator of `token_type`, an unspecified
	 * value is `return`ed that might or might not be a declared enumerator of
	 * `token_category`.
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
	 *     `return`s the index of a `token_type` in the array `return`ed by
	 *     the `all_token_types` function.
	 *
	 * If `tt` is not a declared enumerator (and therefore not present in the
	 * array), the value `total_token_type_count` is `return`ed.
	 *
	 * @param tt
	 *     enumerator to get the index for
	 *
	 * @returns
	 *     index of `tt`
	 *
	 */
	constexpr std::size_t index(token_type tt) noexcept;


	/**
	 * @brief
	 *     `return`s the `token_type` at the specified index if it exists.
	 *
	 * For `idx` in the range `[0, total_token_type_count)`,
	 * `token_type_at_index(idx)` is equivalent to `all_token_types()[idx]` but
	 * can be used in constant expressions.  Additionally, if `idx` is out of
	 * range, this function is still well-behaved and reliably `return`s
	 * `token_type(0)`, which is not a declared enumerator.
	 *
	 * @param idx
	 *     index of the `token_type` to get
	 *
	 * @returns
	 *     `token_type` at the given index
	 *
	 */
	constexpr token_type token_type_at_index(const std::size_t idx) noexcept;


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
	 *  - the `nullptr` for undeclared token types.
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
	 * is not a declared enumerator, the `nullptr` will be `return`ed.
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
	 * If `tt` is not a declared enumerator, a special string will be inserted
	 * that includes the numeric value of the argument.  The `failbit` won't be
	 * set.
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

	/**
	 * @brief
	 *     Inserts the textual representation of a token category into an output
	 *     stream.
	 *
	 * If `cat` is not a declared enumerator, a special string will be inserted
	 * that includes the numeric value of the argument.  The `failbit` won't be
	 * set.
	 *
	 * @param os
	 *     stream to write to
	 *
	 * @param cat
	 *     token category to insert
	 *
	 * @returns
	 *     reference to `os`
	 *
	 */
	std::ostream& operator<<(std::ostream& os, token_category cat);

}  // namespace minijava


#define MINIJAVA_INCLUDED_FROM_LEXER_TOKEN_TYPE_HPP
#include "lexer/token_type.tpp"
#undef MINIJAVA_INCLUDED_FROM_LEXER_TOKEN_TYPE_HPP
