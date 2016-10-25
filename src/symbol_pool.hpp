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
#include <cstring>
#include <unordered_set>

#include "symbol.hpp"


namespace minijava
{
	namespace detail {
		struct entryptr_hash
		{
			constexpr std::size_t operator()(const symbol::symbol_entry* entry) const noexcept
			{
				return entry->hash;
			}
		};

		struct entryptr_equal
		{
			constexpr bool operator()(const symbol::symbol_entry* lhs, const symbol::symbol_entry* rhs) const noexcept
			{

				return lhs->size == rhs->size && std::strcmp(lhs->cstr, rhs->cstr) == 0;
			}
		};
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
	template<typename AllocT = std::allocator<char>>
	class symbol_pool final
	{
	public:

		/** @brief Type alias for `InnerAllocT`. */
		using allocator_type = AllocT;

	private:
		using entry_type = symbol::symbol_entry;

		using char_allocator_type = allocator_type;
		using char_allocator_traits = std::allocator_traits<char_allocator_type>;
		using entry_allocator_type = typename char_allocator_traits::template rebind_alloc<entry_type>;
		using entry_allocator_traits = typename char_allocator_traits::template rebind_traits<entry_type>;


		/** @brief Has set type. */
		using hash_set_type = std::unordered_set<
			const entry_type*,
			detail::entryptr_hash,
			detail::entryptr_equal
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
		symbol_pool(const allocator_type& alloc);

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
		allocator_type get_allocator() const;

	private:
		const char * create_string(const std::string& str);
		const entry_type * create_entry(const char * str_mem, std::size_t size, std::size_t hash);
	private:
		/** allocator used to allocate memory for the symbol' string */
		char_allocator_type _charAlloc;

		/** allocator used to allocate memory for symbol entries */
		entry_allocator_type _entryAlloc;

		/** @brief Pool of symbols. */
		hash_set_type _pool;

	};

}  // namespace minijava


#define MINIJAVA_INCLUDED_FROM_SYMBOL_POOL_HPP
#include "symbol_pool.tpp"
#undef MINIJAVA_INCLUDED_FROM_SYMBOL_POOL_HPP
