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
#include <boost/unordered_set.hpp>

#include <boost/core/noncopyable.hpp>

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
			constexpr std::size_t operator()(const symbol_entry* entry) const noexcept
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
			constexpr bool operator()(const symbol_entry* lhs, const symbol_entry* rhs) const noexcept
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
	class symbol_pool final: private boost::noncopyable
	{
	public:

		/** @brief Type alias for `InnerAllocT`. */
		using allocator_type = AllocT;

	private:
		using entry_type = symbol_entry;                                                 ///< symbol::symbol::entry

		using allocator_traits = std::allocator_traits<allocator_type>;             ///< Traits for allocator_type

		static_assert(std::is_same<char, typename allocator_traits::value_type>::value, "Allocator does not allocate char!");

		/** @brief Has set type. */
		using hash_set_type = boost::unordered_set<
			const entry_type*,
			detail::entryptr_hash,
			detail::entryptr_equal
		>;

		struct symbol_entry_string_cmp;

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

		symbol_pool(symbol_pool&&);

		/**
		 * @brief
		 *     Destructs the symbol pool.
		 *
		 * If NDEBUG is not defined this asserts that no symbols created by this pool exist anymore
		 */
		~symbol_pool();

		symbol_pool& operator=(symbol_pool&& old);


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
		void _clear_pool();

	private:
		/** allocator used to allocate memory for symbol_entry  */
		allocator_type _alloc;

		/** @brief Pool of symbols. */
		hash_set_type _pool;

		/** @brief Pool anchor for some checks */
		std::shared_ptr<symbol_debug_pool_anchor> _anchor;
	};

}  // namespace minijava


#define MINIJAVA_INCLUDED_FROM_SYMBOL_POOL_HPP
#include "symbol_pool.tpp"
#undef MINIJAVA_INCLUDED_FROM_SYMBOL_POOL_HPP
