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
		/**
		 * @brief
		 *     Hash function to access the hash value of a symbol_entry
		 */
		struct entryptr_hash
		{
			/**
			 * @brief
			 *     Returns the hash value of a symbol_entry
			 *
			 * @param entry
			 *     A pointer to a symbol_entry for that the hash value should be returned
			 *
			 * @returns
			 *     The hash value of the symbol_entry
			 */
			constexpr std::size_t operator()(const symbol::symbol_entry* entry) const noexcept
			{
				return entry->hash;
			}
		};

		/**
		 * @brief
		 *     Equals function to test wether the string values of two symbol_entrys are equal
		 */
		struct entryptr_equal
		{
			/**
			 * @brief
			 *     Test wether the string values of two symbol_entrys are equal
			 *
			 * @param lhs
			 *     The first entry
			 *
			 * @param rhs
			 *     The second entry
			 *
			 * @returns
			 *     `true` if the two string values are equal, `false` otherwise
			 *
			 */
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
		using entry_type = symbol::symbol_entry;                                                           ///< symbol::symbol::entry

		using char_allocator_type = allocator_type;                                                        ///< Allocator for char chunks
		using char_allocator_traits = std::allocator_traits<char_allocator_type>;                          ///< Traits for char_allocator_type
		using entry_allocator_type = typename char_allocator_traits::template rebind_alloc<entry_type>;    ///< Allocator for symbol_entry
		using entry_allocator_traits = typename char_allocator_traits::template rebind_traits<entry_type>; ///< Traits for entry_allocator_type


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
		 * @param alloc
		 *     Allocator used to allocate internal strings and symbol_entry
		 *
		 */
		symbol_pool(const allocator_type& alloc);

		/**
		 * @brief
		 *     Destructs the symbol pool.
		 *
		 * If NDEBUG is not defined this asserts that no symbols created by this pool exist anymore
		 */
		~symbol_pool();

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
		 *     `return`s a symbol for the given string, creating one if necessary.
		 *
		 * If the pool does not already contain the symbol, it is inserted.
		 *
		 * @param text
		 *     Text value of the symbol
		 *
		 * @returns
		 *     the symbol
		 *
		 */
		symbol normalize(const std::string& text);

		/**
		 * @brief
		 *     `return`s a copy of the allocator used for allocating memory for
		 *     normalized symbols.
		 *
		 * @returns
		 *     copy of the allocator
		 *
		 */
		allocator_type get_allocator() const;

	private:
	    /**
		 * @brief
		 *     Allocates internal memory for the string and copies its content
		 *
		 * @param str
		 *     The string that should be copied to internal memory
		 *
		 * @returns
		 *     a pointer to the newly created memory conaining the string with NUL-termination
		 */
		const char * create_string(const std::string& str);

		/**
		 * @brief
		 *     Allocates and constructs a symbol_entry in the internal memory
		 *
		 * @param str_mem
		 *     The memory containing the string of the symbol
		 *
		 * @param size
		 *     The length of the string
		 *
		 * @param hash
		 *     Hash value of the string
		 *
		 * @returns
		 *     a pointer to the newly created symbol_entry
		 */
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
