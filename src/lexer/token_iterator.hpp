/**
 * @brief
 *     Standard input iterator interface to token streams produced by a lexer.
 *
 */

#pragma once

#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>


namespace minijava
{

	/**
	 * @brief
	 *     Iterator over the token stream produced by a lexer.
	 *
	 * The iterator before the past-the-end iterator will always dereference to
	 * the end-of-input token.
	 *
	 * The `LexerT` must provide the following interface.
	 *
	 *     struct LexerConcept
	 *     {
	 *         void advance();
	 *         const TokenT& current_token() const;
	 *         bool current_token_is_eof() const;
	 *     };
	 *
	 * There are no requirements on the `TokenT` type except that it must be a
	 * regular type.
	 *
	 * @tparam LexerT
	 *     type of the lexer
	 *
	 */
	template<typename LexerT>
	class token_iterator final
	{
	public:

		/** @brief Type alias for `LexerT`. */
		using lexer_type = LexerT;

		/** @brief Signed integer type good for measuring distances between tokens. */
		using difference_type = std::ptrdiff_t;

		/** @brief Type of the tokens produced by the lexer. */
		using value_type = const std::decay_t<decltype(std::declval<LexerT>().current_token())>;

		/** @brief Pointer type to a token. */
		using pointer = value_type*;

		/** @brief Reference type to a token. */
		using reference = value_type&;

		/** @brief Type alias for `std::input_iterator_tag`. */
		using iterator_category = std::input_iterator_tag;

		/**
		 * @brief
		 *     Creates a past-the-end iterator that can be compared with any
		 *     other `token_iterator`.
		 *
		 */
		token_iterator();

		/**
		 * @brief
		 *     Creates an iterator that points to the current token of the
		 *     given lexer.
		 *
		 * Using the iterator or a copy of it after the `lexer` object has
		 * become invalid will result in undefined behavior.
		 *
		 * @param lexer
		 *     lexer to provide tokens
		 *
		 */
		explicit token_iterator(lexer_type& lexer);

		/**
		 * @brief
		 *     `return`s a reference to the current token.
		 *
		 * Calling this function on a past-the-end iterator or an iterator that
		 * has been invalidated results in undefined behavior.
		 *
		 * @returns
		 *     current token
		 *
		 */
		reference operator*();

		/**
		 * @brief
		 *     `return`s a ointer to the current token.
		 *
		 * Calling this function on a past-the-end iterator or an iterator that
		 * has been invalidated results in undefined behavior.
		 *
		 * @returns
		 *     current token
		 *
		 */
		pointer operator->();

		/**
		 * @brief
		 *     Advances the iterator to the next token.
		 *
		 * Calling this function on a past-the-end iterator or an iterator that
		 * has been invalidated results in undefined behavior.
		 *
		 * @returns
		 *     reference to the advanced iterator
		 *
		 */
		token_iterator& operator++();

		/**
		 * @brief
		 *     Compares two `token_iterator`s.
		 *
		 * Two `token_iterators` are equal if they both refer to the same lexer
		 * or both are past-the-end iterators.  Comparing two `token_iterators`
		 * referring to different lexer objects results in undefined behavior.
		 *
		 * @param lhs
		 *     first `token_iterator` to compare
		 *
		 * @param rhs
		 *     second `token_iterator` to compare
		 *
		 * @returns
		 *     whether the iterators refer to the same token
		 *
		 */
		static bool
		equal(const token_iterator& lhs, const token_iterator& rhs);

	private:

		/** @brief The referred to lexer object or a `nullptr` for past-the-end iterators. */
		lexer_type* _lexer = nullptr;

	};  // class token_iterator


	/**
	 * @brief
	 *     Compares two iterators for equality.
	 *
	 * The same preconditions as for `token_iterator::equal` apply.
	 *
	 * @param lhs
	 *     first `token_iterator` to compare
	 *
	 * @param rhs
	 *     second `token_iterator` to compare
	 *
	 * @returns
	 *     `token_iterator::equal(lhs, rhs)`
	 *
	 */
	template<typename LexerT>
	bool operator==(const token_iterator<LexerT>& lhs,
					const token_iterator<LexerT>& rhs);

	/**
	 * @brief
	 *     Compares two iterators for inequality.
	 *
	 * The same preconditions as for `token_iterator::equal` apply.
	 *
	 * @param lhs
	 *     first `token_iterator` to compare
	 *
	 * @param rhs
	 *     second `token_iterator` to compare
	 *
	 * @returns
	 *     `!token_iterator::equal(lhs, rhs)`
	 *
	 */
	template<typename LexerT>
	bool operator!=(const token_iterator<LexerT>& lhs,
					const token_iterator<LexerT>& rhs);

	/**
	 * @brief
	 *     Convenience function to create a `token_iterator` from a lexer.
	 *
	 * @tparam LexerT
	 *     type of the lexer
	 *
	 * @param lexer
	 *     lexer that provides the tokens
	 *
	 * @returns
	 *     iterator over the tokens produced by the lexer
	 *
	 */
	template <typename LexerT>
	token_iterator<LexerT> token_begin(LexerT& lexer);

	/**
	 * @brief
	 *     Convenience function to create a past-the-end `token_iterator` from
	 *     a lexer.
	 *
	 * @tparam LexerT
	 *     type of the lexer
	 *
	 * @param lexer
	 *     lexer that provides the tokens
	 *
	 * @returns
	 *     past-the-end iterator that can be compared to `token_begin(lexer)`
	 *
	 */
	template <typename LexerT>
	token_iterator<LexerT> token_end(LexerT& lexer);

}  // namespace minijava


#define MINIJAVA_INCLUDED_FROM_LEXER_TOKEN_ITERATOR_HPP
#include "lexer/token_iterator.tpp"
#undef MINIJAVA_INCLUDED_FROM_LEXER_TOKEN_ITERATOR_HPP
