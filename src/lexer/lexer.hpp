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
	 * @tparam SymPoolT
	 *     type of the symbol pool
	 *
	 */
	template<typename InIterT, typename SymPoolT>
	class lexer final
	{

		static_assert(std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<InIterT>::iterator_category>{}
                      and std::is_convertible<typename std::iterator_traits<InIterT>::value_type, char>{},
					  "input iterator over `char` required for `InIterT`");

	public:

		/**
		 * @brief
		 *     Creates a `lexer` that will scan over the character range
		 *     `[first, last)` and use the symbol pool `pool`.
		 *
		 * The range referred to by the iterators as well as the symbol pool
		 * must remain valid throughout the life-time of this object.
		 *
		 * @param first
		 *     iterator pointing to the first character of the input
		 *
		 * @param last
		 *     iterator pointing after the last character of the input
		 *
		 * @param id_pool
		 *     symbol pool to use for identifiers
		 *
		 * @param lit_pool
		 *     symbol pool to use for integer literals
		 *
		 */
		lexer(InIterT first, InIterT last, SymPoolT& id_pool, SymPoolT& lit_pool);

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

		lexer(lexer&&) = default;
		lexer& operator=(lexer&&) = default;

		lexer(const lexer&) = delete;
		lexer& operator=(const lexer&) = delete;

	private:

		/** @brief Current token. */
		token _current_token;

		/** @brief Iterator pointing to the current character of the input. */
		InIterT _current_it;

		/** @brief Iterator pointing after the last character of the input. */
		InIterT _last_it;

		/** @brief Reference to the symbol pool used for identifiers. */
		SymPoolT& _id_pool;

		/** @brief Reference to the symbol pool used for integer literals. */
		SymPoolT& _lit_pool;

		/** @brief Stores the current line number. */
		size_t _line;

		/** @brief Stores the current column of the current line. */
		size_t _column;

		/**
		 * @brief Moves the iterator to the next value and returns the char.
		 * @return The char at the new iterator position.
		 * */
		int _next()
		{
			if (_current_is_last()) {
				return -1;
			}
			_column++;
			_current_it++;
			auto c = _current_is_last() ? -1 : *_current_it;
			if (c == '\n') {
				_column = 1;
				_line++;
			}
			return c;
		}

		/**
		 * @brief
		 *     If the current char is equal to `c`, the current_token is set
		 *     to the token_type `type` and the iterator moves to the next char
		 *
		 * @returns
		 *     true, if the current char is equal to `c`
		 */
		bool _maybe_token(char c, token_type type) {
			if (_current_is_last() || _current() != c) {
				return false;
			}

			_current_token = token::create(type);
			_skip();
			return true;
		}

		/** @brief Moves the iterator to the next value. */
		void _skip() {
			if (_current_is_last()) return;
			_column++;
			_current_it++;
			if (_current_it != _last_it && *_current_it == '\n') {
				_column = 1;
				_line++;
			}
		}

		/** @brief Returns true, if the given char is a valid whitespace for minij */
		bool _isspace(char c) {
			return c == ' ' || c == '\r' || c == '\n' || c == '\t';
		}

		/**
		 * @brief Returns the current char of the iterator.
		 * @return The current char.
		 */
		char _current() {
			return *_current_it;
		}

		void _scan_identifier();

		void _scan_integer();

		void _consume_block_comment();

		bool _current_is_last();

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
	 *     symbol pool to use for identifiers
	 *
	 */
	template<typename InIterT, typename SymPoolT>
	lexer<InIterT, SymPoolT> make_lexer(InIterT first,
										InIterT last,
										SymPoolT& pool);

}  // namespace minijava


#define MINIJAVA_INCLUDED_FROM_LEXER_LEXER_HPP
#include "lexer/lexer.tpp"
#undef MINIJAVA_INCLUDED_FROM_LEXER_LEXER_HPP
