/**
 * @file symbol_pool.hpp
 *     Pools for canonical string representations.
 *
 */

#pragma once

#include <cstddef>
#include <memory>
#include <scoped_allocator>
#include <string>
#include <unordered_set>

#include "symbol.hpp"


namespace minijava
{

	namespace detail
	{
		template <typename AllocT>
		using std_char_string_t = std::basic_string<char, std::char_traits<char>, AllocT>;
	}

	/**
	 * @brief
	 *     A pool for canonical string representations via symbols.
	 *
	 * Symbols can be added into the pool.  This process is called
	 * *normalization*.  After a symbol has been normalized, it is contained in
	 * the bool for as long as it exists and has a canonical address.  Further
	 * calls to `normalize` will `return` that address.  All canonical
	 * addresses become invalid, once the pool is destroyed.
	 *
	 * Note that after copying a `symbol_pool` canonical pointers from the
	 * original and the copy will mismatch.  Moving a `symbol_pool` does not
	 * invalidate canonical pointers.  In a sense, canonical pointers can be
	 * thought of as iterators of node-based containers.
	 *
	 * @tparam InnerAllocT
	 *     allocator type used for allocating memory for normalized symbols
	 *
	 * @tparam OuterAllocT
	 *     allocator type used for allocating internal data structures
	 *
	 */
	// work version!: do not use any specific template parameters as they are subject to change! only <>
	template
	<
		typename InnerAllocT = std::allocator<char>,
		typename OuterAllocT = std::allocator<detail::std_char_string_t<InnerAllocT>>
	>
	class symbol_pool final
	{
	public:

		/** @brief Type alias for `InnerAllocT`. */
		using inner_allocator_type = InnerAllocT;

		/** @brief Type alias for `OuterAllocT`. */
		using outer_allocator_type = OuterAllocT;

	private:

		/** @brief Internal symbol type. */
		using std_string_type = detail::std_char_string_t<inner_allocator_type>;

		/** @brief Allocator type for the hash set. */
		using scoped_allocator_type = std::scoped_allocator_adaptor<
			outer_allocator_type,
			inner_allocator_type
		>;

		/** @brief Has set type. */
		using hash_set_type = std::unordered_set<
			std_string_type,
			std::hash<std_string_type>,
			std::equal_to<std_string_type>,
			scoped_allocator_type
		>;

	public:

		/**
		 * @brief
		 *     Constructs an empty pool with default-constructed allocators.
		 *
		 */
		symbol_pool();

		/**
		 * @brief
		 *     Constructs an empty pool with the provided allocators.
		 *
		 * @tparam inner
		 *     allocator used for allocating memory for normalized symbols
		 *
		 * @tparam outer
		 *     allocator used for allocating internal data structures
		 *
		 *
		 */
		// this is subject to changes
		/*symbol_pool(const inner_allocator_type& inner,
		            const outer_allocator_type& outer);*/

		/**
		 * @brief
		 *     `return`s the number of symbols in the pool.
		 *
		 * @returns
		 *     number of symbols in the pool
		 *
		 */
		std::size_t size() const noexcept;

		/**
		 * @brief
		 *     Tests whether the pool is empty.
		 *
		 * @returns
		 *     whether the pool is empty
		 *
		 */
		bool empty() const noexcept;

		/**
		 * @brief
		 *     Tests whether the pool contains a given symbol.
		 *
		 * If the pool does not contain the symbol, it will *not* be added.
		 * Use `normalize` if you want to add a symbol.
		 *
		 * @param text
		 *     symbol to look up
		 *
		 * @returns
		 *     whether the pool contains the symbol
		 *
		 */
		bool contains(const std::string& text) const;

		/**
		 * @brief
		 *     `return`s a canonical representation of a symbol, creating one
		 *     if necessary.
		 *
		 * If the pool does not already contain the symbol, it is inserted.
		 * Then its canonical address is `return`ed.
		 *
		 * @param text
		 *     symbol to canonicalize
		 *
		 * @returns
		 *     canonical representation of the symbol
		 *
		 */
		symbol normalize(const std::string& text);

		/**
		 * @brief
		 *     `return`s a copy of the allocator used for allocating memory for
		 *     normalized symbols.
		 *
		 * @returns
		 *     copy of the inner allocator
		 *
		 */
		// this is subject to changes
		//inner_allocator_type get_inner_allocator() const;

		/**
		 * @brief
		 *     `return`s a copy of the allocator used for allocating internal
		 *     data structures.
		 *
		 * @returns
		 *     copy of the outer allocator
		 *
		 */
		// this is subject to changes
		//outer_allocator_type get_outer_allocator() const;

	private:

		/** @brief Pool of symbols. */
		hash_set_type _pool{};

	};

}  // namespace minijava


#define MINIJAVA_INCLUDED_FROM_SYMBOL_POOL_HPP
#include "symbol_pool.tpp"
#undef MINIJAVA_INCLUDED_FROM_SYMBOL_POOL_HPP
