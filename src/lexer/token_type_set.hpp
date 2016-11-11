#pragma once

#include <climits>
#include <initializer_list>
#include <utility>

#include "lexer/token_type.hpp"

namespace minijava
{

	/**
	 * @brief
	 *     A set (math) of `token_type`s.
	 *
	 * All operations on this type that involve the user passing `token_type`s
	 * invoke undefined behavior if one or more of the given `token_type`s are
	 * not declared enumerators of the `token_type` `enum`.  This precondition
	 * is implicit for all interface functions and not repeated in their
	 * respective documentation.
	 *
	 */
	class token_type_set final
	{
	public:

		/**
		 * @brief
		 *     Constructs an empty `token_type_set`.
		 *
		 */
		constexpr token_type_set() noexcept = default;

		/**
		 * @brief
		 *     Constructs a `token_type_set` that contains the given set of
		 *     tokens.
		 *
		 * Duplicate entries in the initializer list are allowed and will have
		 * no effect.
		 *
		 * @param tts
		 *     `token_type`s that should initially be in the set
		 *
		 */
		constexpr token_type_set(std::initializer_list<token_type> tts);

		/**
		 * @brief
		 *     Tests whether the `token_type_set` contains zero `token_type`s.
		 *
		 * @returns
		 *     whether the set is empty
		 *
		 */
		constexpr bool empty() const noexcept;

		/**
		 * @brief
		 *     Tests whether the `token_type_set` contains a given
		 *     `token_type`.
		 *
		 * @param tt
		 *     `token_type` to look up
		 *
		 * @returns
		 *     whether the set contains `tt`
		 *
		 */
		constexpr bool contains(token_type tt) const;

		/**
		 * @brief
		 *     Adds a value into the set.
		 *
		 * If the set already contains the value, this function has no effect.
		 *
		 * @param tt
		 *     `token_type` to add
		 *
		 */
		constexpr void add(token_type tt);

		/**
		 * @brief
		 *     Removed a value from the set.
		 *
		 * If the set didn't contain the value in the first place, this
		 * function has no effect.
		 *
		 * @param tt
		 *     `token_type` to remove
		 *
		 */
		constexpr void remove(token_type tt);

		/**
		 * @brief
		 *     Adds all values from `other` to this `token_type_set`.
		 *
		 * @param other
		 *     `token_type_set` with values to add
		 *
		 * @returns
		 *     reference to `*this`
		 *
		 */
		constexpr token_type_set& operator|=(const token_type_set& other) noexcept;

		/**
		 * @brief
		 *     Removes all values that are not also in `other` from this
		 *     `token_type_set`.
		 *
		 * @param other
		 *     `token_type_set` with values to keep
		 *
		 * @returns
		 *     reference to `*this`
		 *
		 */
		constexpr token_type_set& operator&=(const token_type_set& other) noexcept;

		/**
		 * @brief
		 *     Tests whether two `token_type_set`s contain the same elements.
		 *
		 * @param lhs
		 *     first `token_type_set` to compare
		 *
		 * @param rhs
		 *     second `token_type_set` to compare
		 *
		 * @returns
		 *     whether `lhs` and `rhs` contain the same set of `token_type`s
		 *
		 */
		static constexpr bool equal(const token_type_set& lhs, const token_type_set& rhs) noexcept;

	private:

		/** @brief `unsigned` integer type to use for storing bits internally. */
		using word_type = unsigned long;

		/** @brief Number of bytes in a word. */
		static constexpr auto word_size = sizeof(word_type);

		/** @brief Number of bits in a word. */
		static constexpr auto word_bits = CHAR_BIT * word_size;

		/** @brief Size of the internal array of words. */
		static constexpr auto word_count = (total_token_type_count + word_bits - 1) / word_bits;

		/** @brief Bitset that has bit *i* set iff the set contains the `token_type` with index *i*. */
		word_type _bits[word_count] = {};

		/**
		 * @brief
		 *     Helper-function that computes the index of the bit that stores
		 *     the information whether `tt` is in this set.
		 *
		 * @param tt
		 *     `token_type` to compute the index for
		 *
		 * @returns
		 *     pair `idx` of indices such that the word at `idx.first` contains
		 *     the information about `tt` at bit `idx.second`
		 *
		 */
		constexpr static std::pair<std::size_t, std::size_t> _get_index(token_type tt);

	};  // class token_type_set


	/**
	 * @brief
	 *     Computes the union of two `token_type_set`s.
	 *
	 * @param lhs
	 *     first `token_type_set`
	 *
	 * @param rhs
	 *     second `token_type_set`
	 *
	 * @returns
	 *     a `token_type_set` with all elements that are in `lhs` or in `rhs`
	 *
	 */
	constexpr token_type_set operator|(const token_type_set& lhs, const token_type_set& rhs) noexcept;

	/**
	 * @brief
	 *     Computes the intersection of two `token_type_set`s.
	 *
	 * @param lhs
	 *     first `token_type_set`
	 *
	 * @param rhs
	 *     second `token_type_set`
	 *
	 * @returns
	 *     a `token_type_set` with all elements that are in `lhs` and in `rhs`
	 *
	 */
	constexpr token_type_set operator&(const token_type_set& lhs, const token_type_set& rhs) noexcept;

	/**
	 * @brief
	 *     Compares two `token_type_set`s for equality.
	 *
	 * @param lhs
	 *     first `token_type_set` to compare
	 *
	 * @param rhs
	 *     second `token_type_set` to compare
	 *
	 * @returns
	 *     `token_type_set::equal(lhs, rhs)`
	 *
	 */
	constexpr bool operator==(const token_type_set& lhs, const token_type_set& rhs) noexcept;

	/**
	 * @brief
	 *     Compares two `token_type_set`s for inequality.
	 *
	 * @param lhs
	 *     first `token_type_set` to compare
	 *
	 * @param rhs
	 *     second `token_type_set` to compare
	 *
	 * @returns
	 *     `!#token_type_set::equal(lhs, rhs)`
	 *
	 */
	constexpr bool operator!=(const token_type_set& lhs, const token_type_set& rhs) noexcept;

}  // namespace minijava


#define MINIJAVA_INCLUDED_FROM_LEXER_TOKEN_TYPE_SET_HPP
#include "lexer/token_type_set.tpp"
#undef MINIJAVA_INCLUDED_FROM_LEXER_TOKEN_TYPE_SET_HPP
