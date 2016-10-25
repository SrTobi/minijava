/**
 * @file lexer.hpp
 *
 * @brief
 *     Lexer for MiniJava.
 *
 */

#pragma once

#include <iterator>
#include <stdexcept>
#include <type_traits>

#include "lexer/token.hpp"


namespace minijava
{

	/**
	 * @brief
	 *     Exception indicating a lexical error in the input.
	 *
	 */
	struct lexical_error : std::runtime_error
	{
		lexical_error() : std::runtime_error{"invalid input"} {}
		lexical_error(std::string msg) : std::runtime_error{std::move(msg)} {}
	};

	/**
	 * @brief
	 *     Lexer for the MiniJava language.
	 *
	 * @tparam InIterT
	 *     type of the character iterator for reading the source
	 *
	 */
	template<typename InIterT, typename StrPoolT>
	class lexer final
	{

		static_assert(std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<InIterT>::iterator_category>{}
                      and std::is_convertible<typename std::iterator_traits<InIterT>::value_type, char>{},
					  "input iterator over `char` required for `InIterT`");

	public:

		/**
		 * @brief
		 *     Creates a `lexer` that will scan over the character range
		 *     `[first, last)` and use the string pool `pool`.
		 *
		 * The range referred to by the iterators as well as the string pool
		 * must remain valid throughout the life-time of this object.
		 *
		 * @param first
		 *     iterator pointing to the first character of the input
		 *
		 * @param last
		 *     iterator pointing after the last character of the input
		 *
		 * @param pool
		 *     string pool to use for identifiers
		 *
		 */
		lexer(InIterT first, InIterT last, StrPoolT& pool);

		/**
		 * @brief
		 *     Scans the next token.
		 *
		 * If the scanner is already beyon the end of the file, this function
		 * has no effect.
		 *
		 * If an exception is `throw`n, subsequent calls to `current_token()`
		 * will `return` a reference to a token in a valid but unspecified
		 * state.  This lexer does not recover from this.
		 *
		 * @throws lexical_error
		 *     if the following characters do not form a valid token
		 *
		 */
		void advance();

		/**
		 * @brief
		 *     `return`s a reference to the current token.
		 *
		 * @returns
		 *     reference to current token
		 *
		 */
		const token& current_token() const noexcept;

		/**
		 * @brief
		 *     `return`s whether the current token is the end-of-input token.
		 *
		 * @returns
		 *     `current_token().type() == token_type::eof`
		 *
		 */
		bool current_token_is_eof() const noexcept;

		lexer(lexer&&) noexcept = default;
		lexer& operator=(lexer&&) noexcept = default;

		lexer(const lexer&) = delete;
		lexer& operator=(const lexer&) = delete;

	private:

		/** @brief Current token. */
		token _current_token;

		/** @brief Iterator pointing to the next character of the input. */
		InIterT _next;

		/** @brief Iterator pointing after the last character of the input. */
		InIterT _last;

		/** @brief Reference to the string-pool used for identifiers. */
		StrPoolT& _id_pool;

	};  // class lexer

	/**
	 * @brief
	 *     Convenience function for construction a `lexer` object.
	 *
	 * @param first
	 *     iterator pointing to the first character of the input
	 *
	 * @param last
	 *     iterator pointing after the last character of the input
	 *
	 * @param pool
	 *     string pool to use for identifiers
	 *
	 */
	template<typename InIterT, typename StrPoolT>
	lexer<InIterT, StrPoolT> make_lexer(InIterT first,
										InIterT last,
										StrPoolT& pool);

}  // namespace minijava


#define MINIJAVA_INCLUDED_FROM_LEXER_LEXER_HPP
#include "lexer/lexer.tpp"
#undef MINIJAVA_INCLUDED_FROM_LEXER_LEXER_HPP
