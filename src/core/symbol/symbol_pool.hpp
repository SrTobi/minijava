/**
 * @file symbol_pool.hpp
 *
 * @brief
 *     Pools for canonical string representations.
 *
 */

#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <utility>

#include <boost/unordered_set.hpp>

#include "symbol/symbol.hpp"
#include "symbol/symbol_anchor.hpp"
#include "symbol/symbol_entry.hpp"


namespace minijava
{

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
	 * The pool only uses the provided allocator to allocate memory for
	 * `symbol_entry`s of the normalized symbols.  Additional internal data
	 * structures are always allocated using the global oprator `new`.
	 *
	 * @tparam AllocT
	 *     allocator type used for allocating memory for normalized symbols
	 *
	 */
	template<typename AllocT = std::allocator<symbol_entry>>
	class symbol_pool final : private AllocT
	{
	public:

		/** @brief Type alias for `AllocT`. */
		using allocator_type = AllocT;

	private:

		/** @brief Type alias for `symbol_entry`. */
		using entry_type = symbol_entry;

		/** @brief Type alias for a smart pointer holding an `entry_type`. */
		using entryptr_type = unique_symbol_entr_ptr<allocator_type>;

		/** @brief Has set type. */
		using hash_set_type = boost::unordered_set<
			entryptr_type,
			symbol_entry_ptr_hash,
			symbol_entry_ptr_equal
		>;

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
		 *     allocator used to allocate `symbol_entry`s
		 *
		 */
		symbol_pool(const allocator_type& alloc);

		/**
		 * @brief
		 *     Move constructor.
		 *
		 * The moved-away-from `symbol_pool` is left in an empty state.
		 *
		 * @param other
		 *     `symbol_pool` to move away from
		 *
		 */
		symbol_pool(symbol_pool&& other) noexcept;

		/**
		 * @brief
		 *     Move-assignment operator.
		 *
		 * The moved-away-from `symbol_pool` is left in an empty state.
		 *
		 * @param other
		 *     `symbol_pool` to move away from
		 *
		 * @returns
		 *     a reference to `*this`
		 *
		 */
		symbol_pool& operator=(symbol_pool&& other) noexcept;

		/**
		 * @brief
		 *     `delete`d copy constructor.
		 *
		 * `symbol_pool`s are not copyable.
		 *
		 * @param other
		 *     *N/A*
		 *
		 */
		symbol_pool(const symbol_pool& other) = delete;

		/**
		 * @brief
		 *     `delete`d copy-assignment operator.
		 *
		 * `symbol_pool`s are not copyable.
		 *
		 * @param other
		 *     *N/A*
		 *
		 * @returns
		 *     *N/A*
		 *
		 */
		symbol_pool& operator=(const symbol_pool& other) = delete;

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
		 *     Tests whether a canonical representation of a string already
		 *     exists.
		 *
		 * The empty string always has a canonical representation even if it
		 * was never added to the pool.  If the pool does not contain the
		 * symbol, it will *not* be added.  Use `normalize` if you want to add
		 * a symbol.
		 *
		 * @param text
		 *     text to check
		 *
		 * @returns
		 *     `true` if `text.empty()` or the pool contains `text`
		 *
		 */
		bool is_normalized(const std::string& text) const;

		/**
		 * @brief
		 *     `return` a canonical representation of a string.
		 *
		 * If the pool does not already contain the symbol, it is inserted.
		 * Then a canonical representation is `return`ed.
		 *
		 * @param text
		 *     text value of the symbol
		 *
		 * @returns
		 *     the canonical symbol
		 *
		 */
		symbol normalize(const std::string& text);

		/**
		 * @brief
		 *     `return`s a `const` reference to the stored allocator.
		 *
		 * @returns
		 *     allocator
		 *
		 */
		const allocator_type& get_allocator() const noexcept;

		/**
		 * @brief
		 *     `return`s a mutable reference to the stored allocator.
		 *
		 * @returns
		 *     allocator
		 *
		 */
		allocator_type& get_allocator() noexcept;

		/**
		 * @brief
		 *     Exchanges the states of two `symbol_pool`.
		 *
		 * @param lhs
		 *     first `symbol_pool`
		 *
		 * @param rhs
		 *     second `symbol_pool`
		 *
		 */
		friend void swap(symbol_pool& lhs, symbol_pool& rhs) noexcept
		{
			using std::swap;
			swap(static_cast<AllocT&>(lhs), static_cast<AllocT&>(rhs));
			swap(lhs._pool, rhs._pool);
			swap(lhs._anchor, rhs._anchor);
		}

	private:

		/** @brief Pool of symbols. */
		hash_set_type _pool{};

		/** @brief Pool anchor for some checks */
		std::shared_ptr<symbol_anchor> _anchor{};

	};  // symbol_pool

}  // namespace minijava


#define MINIJAVA_INCLUDED_FROM_SYMBOL_SYMBOL_POOL_HPP
#include "symbol/symbol_pool.tpp"
#undef MINIJAVA_INCLUDED_FROM_SYMBOL_SYMBOL_POOL_HPP
