/**
 * @file symbol_pool.hpp

 // 5gon12eder: Missing @brief tag.

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
			template<typename DeleterT>
			constexpr std::size_t operator()(const std::unique_ptr<const symbol_entry, DeleterT>& entry) const noexcept
			{
				return entry->hash;
			}
		};

		/**
		 * @brief
		 *     Equals function to test whether the string values of two symbol_entrys are equal
		 */
		struct entryptr_equal
		{
			/**
			 * @brief
			 *     Test whether the string values of two symbol_entrys are equal
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
			template<typename DeleterT>
			constexpr bool operator()(const std::unique_ptr<const symbol_entry, DeleterT>& lhs, const std::unique_ptr<const symbol_entry, DeleterT>& rhs) const noexcept
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
	 * the pool for as long as it exists and has a canonical address.  Further
	 * calls to `normalize` will `return` that address.  All canonical
	 * addresses become invalid, once the pool is destroyed.
	 *
	 * Moving a `symbol_pool` does not invalidate canonical pointers.
	 *
	 * @tparam AllocT
	 *     allocator type used for allocating memory for normalized symbols
	 *
	 */
	template<typename AllocT = std::allocator<char>>

	// 5gon12eder: Is inheriting from `boost::noncopyable` really needed these
	// days where we can just say `= delete` for the copy operations?

	class symbol_pool final: private boost::noncopyable
	{
	public:

		/** @brief Type alias for `InnerAllocT`. */
		using allocator_type = AllocT;

	private:
		using entry_type = symbol_entry;                                                 ///< symbol::symbol::entry

		using allocator_traits = std::allocator_traits<allocator_type>;             ///< Traits for allocator_type

		// 5gon12eder: Again, the `static_assert`ion is probably overly
		// restrictive.

		static_assert(std::is_same<char, typename allocator_traits::value_type>::value, "Allocator does not allocate char!");

		using entryptr_type = symbol_entry::ptr<allocator_type>;

		/** @brief Has set type. */
		using hash_set_type = boost::unordered_set<
			entryptr_type,
			detail::entryptr_hash,
			detail::entryptr_equal
		>;

		struct symbol_entry_string_cmp;

	public:

		/**
		 * @brief
		 *     Constructs an empty pool with default-constructed allocator.
		 *
		 */
		symbol_pool();

		/**
		 * @brief
		 *     Constructs an empty pool with the provided allocator.
		 *
		 * @param alloc
		 *     Allocator used to allocate internal strings and symbol_entry
		 *
		 */
		symbol_pool(const allocator_type& alloc);

		/**
		 * @brief
		 *     Move constructs this symbol pool
		 */

		// 5gon12eder: Move operations should be `noexcept`.

		symbol_pool(symbol_pool&&);

		/**
		 * @brief
		 *     Destructs the symbol pool.
		 *
		 * If NDEBUG is not defined this asserts that no symbols created by this pool exist anymore
		 */
		~symbol_pool();

		/**
		 * @brief
		 *     Move assigns this symbol pool
		 */
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
		void _invalidate_pool();

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
