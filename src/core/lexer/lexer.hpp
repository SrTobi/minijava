/**
 * @file lexer.hpp
 *
 * @brief
 *     Lexer for MiniJava.
 *
 */

#pragma once

#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "lexer/token.hpp"
#include "source_error.hpp"


namespace minijava
{

	/**
	 * @brief
	 *     Exception indicating a lexical error in the input.
	 *
	 */
	struct lexical_error : source_error
	{
		/**
		 * @brief
		 *     Creates a new exception object with a generic error message and
		 *     no source location information.
		 *
		 */
		lexical_error();

		/**
		 * @brief
		 *     Creates a new exception object with a custom error message and
		 *     optional source location information.
		 *
		 * @param msg
		 *     custom error message
		 *
		 * @param pos
		 *     parser-defined error location
		 *
		 */
		explicit lexical_error(const std::string& msg, minijava::position pos = {});
	};

	/**
	 * @brief
	 *     Lexer for the MiniJava language.
	 *
	 * @tparam InIterT
	 *     type of the character iterator for reading the source
	 *
	 * @tparam IdPoolT
	 *     type of the symbol pool used for identifiers
	 *
	 * @tparam LitPoolT
	 *     type of the symbol pool used for literals
	 *
	 * @tparam AllocT
	 *     allocator used to allocate internal working buffers
	 *
	 */
	template
	<
		typename InIterT,
		typename IdPoolT,
		typename LitPoolT,
		typename AllocT = std::allocator<char>
	>
	class lexer final
	{

		static_assert(std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<InIterT>::iterator_category>{}
                      and std::is_convertible<typename std::iterator_traits<InIterT>::value_type, char>{},
					  "input iterator over `char` required for `InIterT`");

	public:

		/**
		 * @brief
		 *     Creates a `lexer` that will scan over the character range
		 *     `[first, last)` and use the symbol pools `id_pool` for
		 *     identifiers and `lit_pool` for integer literals.
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
		 * @param alloc
		 *     allocator to use for allocating internal working buffers
		 *
		 * @throws lexical_error
		 *     if the input does not start with a valid token
		 *
		 */
		lexer(InIterT first, InIterT last,
			  IdPoolT& id_pool, LitPoolT& lit_pool,
			  const AllocT& alloc);

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
		/**
		 * @brief
		 *     Returns the current position of the lexer in the input file.
		 *
		 * @returns
		 *     the current position
		 */
		position _position() const;

	private:

		/** @brief Current token. */
		token _current_token;

		/** @brief Iterator pointing to the current character of the input. */
		InIterT _current_it;

		/** @brief Iterator pointing after the last character of the input. */
		InIterT _last_it;

		/** @brief Reference to the symbol pool used for identifiers. */
		IdPoolT& _id_pool;

		/** @brief Reference to the symbol pool used for integer literals. */
		LitPoolT& _lit_pool;

		/** @brief Line number of the character referred to by `*_current_it`. */
		size_t _line;

		/** @brief Column number of the character referred to by `*_current_it`. */
		size_t _column;

		/** @brief Scratch buffer used by some internal lexing routines. */
		std::basic_string<char, std::char_traits<char>, AllocT> _lexbuf;

		/** @brief Helper-`struct` with private implementation details. */
		struct lexer_impl;

	};  // class lexer


	/**
	 * @brief
	 *     Convenience function for construction a `lexer` object.
	 *
	 * @tparam InIterT
	 *     type of the character iterator for reading the source
	 *
	 * @tparam IdPoolT
	 *     type of the symbol pool used for identifiers
	 *
	 * @tparam LitPoolT
	 *     type of the symbol pool used for literals
	 *
	 * @tparam AllocT
	 *     allocator used to allocate internal working buffers
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
	 * @param alloc
	 *     allocator to use for allocating internal working buffers
	 *
	 * @returns
	 *     lexer for the given character range which uses the given symbol pools
	 *
	 * @throws lexical_error
	 *     if the input does not start with a valid token
	 *
	 */
	template
	<
		typename InIterT,
		typename IdPoolT,
		typename LitPoolT,
		typename AllocT = std::allocator<char>
	>
	lexer<InIterT, IdPoolT, LitPoolT, AllocT>
	make_lexer(
		InIterT first, InIterT last,
		IdPoolT& id_pool, LitPoolT& lit_pool,
		const AllocT& alloc = AllocT{}
	);

}  // namespace minijava


#define MINIJAVA_INCLUDED_FROM_LEXER_LEXER_HPP
#include "lexer/lexer.tpp"
#undef MINIJAVA_INCLUDED_FROM_LEXER_LEXER_HPP
