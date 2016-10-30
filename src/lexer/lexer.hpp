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
		 *     `symbol_pool` to use for identifiers
		 *
		 * @param lit_pool
		 *     `symbol_pool` to use for integer literals
		 *
		 * @throws lexical_error
		 *     if the input does not start with a valid token
		 *
		 */
		lexer(InIterT first, InIterT last, SymPoolT& id_pool, SymPoolT& lit_pool);

		/**
		 * @brief
		 *     `default`ed move constructor.
		 *
		 * The moved-away-from `lexer` is left in an invalid state and calling
		 * any member function except for the destructor or assignment-operator
		 * on it will invoke undefined behavior.
		 *
		 * @param other
		 *     `lexer` object to move away from
		 *
		 */
		lexer(lexer&& other) = default;

		/**
		 * @brief
		 *     `default`ed move-assignment operator.
		 *
		 * The moved-away-from `lexer` is left in an invalid state and calling
		 * any member function except for the destructor or assignment-operator
		 * on it will invoke undefined behavior.
		 *
		 * @param other
		 *     `lexer` object to move away from
		 *
		 * @returns
		 *     a reference to `*this`
		 *
		 */
		lexer& operator=(lexer&& other) = default;

		/**
		 * @brief
		 *     `delete`d copy constructor.
		 *
		 * `lexer` objects are not copyable.
		 *
		 * @param other
		 *     *N/A*
		 *
		 */
		lexer(const lexer& other) = delete;

		/**
		 * @brief
		 *     `delete`d copy-assignment operator.
		 *
		 * `lexer` objects are not copyable.
		 *
		 * @param other
		 *     *N/A*
		 *
		 * @returns
		 *     *N/A*
		 *
		 */
		lexer& operator=(const lexer& other) = delete;

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

		/** @brief Line number of the character referred to by `*_current_it`. */
		size_t _line;

		/** @brief Column number of the character referred to by `*_current_it`. */
		size_t _column;

		/**
		 * @brief
		 *     If the current input character is `c`, sets `_current_token` to
		 *     that token and advances the input iterator.  Otherwise, the
		 *     function has no effect.
		 *
		 * @tparam c
		 *     character that would cause us to lex the token
		 *
		 * @param tt
		 *     `token_type` of the token to scan
		 *
		 * @returns
		 *     whether the token was scanned
		 *
		 */
		bool _maybe_token(const int c, const token_type tt)
		{
			if (_current() != c) {
				return false;
			}
			_current_token = token::create(tt);
			_skip();
			return true;
		}

		/**
		 * @brief
		 *     `return`s the current input character.
		 *
		 * If the input sequence is not yet exhausted, the current character is
		 * `return`ed.  Otherwise, if if the end of the input was already
		 * reached, &minus;1 is `return`ed.
		 *
		 * @returns
		 *     current input character
		 *
		 */
		int _current() const noexcept
		{
			return (_current_it != _last_it)
				? static_cast<unsigned char>(*_current_it)
				: -1;
		}

		/**
		 * @brief
		 *     `return`s the next input character.
		 *
		 * If the input sequence is already exhausted on entry, this function
		 * has no effect.  Otherwise, the input iterator is advanced and the
		 * line and column numbers updated.  Then `_current()` is `return`ed.
		 *
		 * @returns
		 *     next input character
		 *
		 */
		int _next() noexcept
		{
			if (_current_it == _last_it) {
				return -1;
			}
			++_current_it;
			const auto c = _current();
			if (c == '\n') {
				_line += 1;
				_column = 0;
			} else {
				_column += 1;
			}
			return c;
		}

		/**
		 * @brief
		 *     Like the public `advance` function but has to be called in a
		 *     loop until it succeeds.
		 *
		 * @returns
		 *     whether the next token was scanned successfully
		 *
		 */
		bool _maybe_advance();

		/**
		 * @brief
		 *     Discards the current input character.
		 *
		 * This function has the exact same effect as `_next` but a name that
		 * might be samantically more appropriate in certain situations.
		 *
		 */
		void _skip() noexcept
		{
			_next();
		}

		/**
		 * @brief
		 *     Scans an identifier or keyword (word) token.
		 *
		 * On entry, `_current()` must be a valid begin of a word.  Otherwise,
		 * the behavior is undefined.
		 *
		 * The `_current_token` is set to the scanned word and the input
		 * iterator advanced to the character past the last character that was
		 * part of the scanned token.
		 *
		 */
		void _scan_identifier();

		/**
		 * @brief
		 *     Scans an integer literal token.
		 *
		 * On entry, `_current()` must be a valid begin of an integer literal.
		 * Otherwise, the behavior is undefined.
		 *
		 * The `_current_token` is set to the scanned integer literal and the
		 * input iterator advanced to the character past the last character
		 * that was part of the scanned token.
		 *
		 */
		void _scan_integer_literal();

		/**
		 * @brief
		 *     Skips over a block
		 *     <code>&#x2f;&#x2a;&nbsp;&hellip;&nbsp;&hx2a;&#x2f;</code>
		 *     comment.
		 *
		 * The input iterator is advanced to the first character after the next
		 * <code>&#x2a;&#x2f;</code> and its value `return`ed.  This means that
		 * on entry, the current character must already be the character
		 * following the <code>&#x2f;&#x2a;</code> that opened the
		 * block-comment or it will be mis.interpreted as part of a potential
		 * closing <code>&#x2a;&#x2f;</code> sequence.
		 *
		 * @returns
		 *     first character after the skipped region
		 *
		 * @throws lexical_error
		 *     if input ended before <code>&#x2a;&#x2f;</code> was seen
		 *
		 */
		int _skip_block_comment();

		/**
		 * @brief
		 *     Skips over white-space.
		 *
		 * The input iterator is advanced to the next character that is not
		 * considered white-space.  The value of that character is `return`ed.
		 *
		 * If the current input character is not white-space, then this
		 * function has no effect and `return`s `_current()`.
		 *
		 * @returns
		 *     first character after the skipped region
		 *
		 */
		int _skip_white_space() noexcept;

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
