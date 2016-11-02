/**
 * @file symbol_entry.hpp
 *
 * @brief
 *     Low-level data interface for `symbol`s.
 *
 * The stuff in this file should only ever be relevant for authors of symbol
 * pools.
 *
 */

#pragma once

#include <cstddef>
#include <memory>
#include <type_traits>


namespace minijava
{

	/**
	 * @brief
	 *     Underlying entry for a symbol.
	 *
	 * This is a POD type with a `char[]` as its last non-`static` data member
	 * (`data`).  This means that a sufficiently large chunk of memory -- if
	 * properly aligned -- may be interpreted as a pointer to a `symbol_entry`
	 * and the string data can be written into the memory region that begins at
	 * the offet of the `data` member.  Since the allocation and initialization
	 * logic is a bit tricky, a factory function is provided for this.  Because
	 * this type must be a POD type, it cannot have a constructor.
	 *
	 * Only symbol pool implementations and the `symbol` `class` itself should
	 * ever have to deal with this low-level type.
	 *
	 */
	struct symbol_entry final
	{

		/** @brief Precomputed hash of the string. */
		std::size_t hash;

		/** @brief Size of the symbol's string. */
		std::size_t size;

		/** @brief NUL terminated string data. */
		char data[1];

	};

	static_assert(std::is_pod<symbol_entry>{}, "");


	/**
	 * @brief
	 *     Deleter type that can be used to delete `symbol_entry`s created by
	 *     `new_symbol_entry`.
	 *
	 * Of course, the `return`ed smart pointer already takes care of this.
	 *
	 * The `value_type` of `AllocT` must be `symbol_entry`.
	 *
	 * @tparam AllocT
	 *     type of the allocator used to allocate and deallocate memory
	 *
	 */
	template<typename AllocT>
	struct symbol_entry_deleter : private AllocT
	{

		static_assert(std::is_same<symbol_entry, typename std::allocator_traits<AllocT>::value_type>{},
		              "symbol_entry_deleter<AllocT> can only work when AllocT::value_type is symbol_entry");

		/** @brief Type alias for `AllocT`. */
		using allocator_type = AllocT;

		/**
		 * @brief
		 *     `default`ed default constructor.
		 *
		 * This constructor will default-construct the stored allocator.  If
		 * the allocator type does not support this or a thusly constructed
		 * allocator is unable to deallocate memory obtained from another
		 * instance, then this constructor may not be used.
		 *
		 */
		symbol_entry_deleter() = default;

		/**
		 * @brief
		 *     Creates a deleter with the provided allocator.
		 *
		 * @param alloc
		 *     allocator to use for deallocation
		 *
		 */
		symbol_entry_deleter(const AllocT& alloc);

		/**
		 * @brief
		 *     Releases a symbol_entry resource via the internal allocator.
		 *
		 * If `entryptr` is anything but a pointer previously obtained via
		 * `new_symbol_entry` that has not been released yet, the behavior is
		 * undefined.  This includes that messing with the data members is not
		 * allowed as the size information is needed to figure out how much
		 * memory to free again.
		 *
		 * @param entryptr
		 *     `symbol_entry` to delete
		 *
		 */
		void operator()(const symbol_entry * entryptr);

	};


	/**
	 * @brief
	 *     Convenience `typedef` for the kind of smart pointer `return`ed by
	 *     `new_symbol_entry`.
	 *
	 * The `value_type` of `allocT` must be `symbol_entry`.
	 *
	 * @tparam AllocT
	 *     allocator type used for allocating the memory
	 *
	 */
	template <typename AllocT = std::allocator<symbol_entry>>
	using unique_symbol_entr_ptr = std::unique_ptr<const symbol_entry, symbol_entry_deleter<AllocT>>;

	/**
	 * @brief
	 *     Factory function for `symbol_entry`s.
	 *
	 * This function allocates one continuous chunk of memory and copies the
	 * provided data into it.
	 *
	 * @param alloc
	 *     allocator that should be used to allocate the memory
	 *
	 * @param hash
	 *     hash value for the new `symbol_entry`
	 *
	 * @param size
	 *     number of bytes (excluding a terminating NUL byte) in the string
	 *     data
	 *
	 * @param data
	 *     string value of the symbol (need not be NUL-terminated)
	 *
	 * @returns
	 *     newly created `symbol_entry`
	 *
	 */
	template <typename AllocT>
	unique_symbol_entr_ptr<AllocT>
	new_symbol_entry(AllocT& alloc, std::size_t hash, std::size_t size, const char * data);

	/**
	 * @brief
	 *     `return`s a special pointer to the statically allocated
	 *     `symbol_entry` for the empty `symbol` singleton.
	 *
	 * @returns
	 *     the empty `symbol_entry`
	 *
	 */
	const symbol_entry * get_empty_symbol_entry() noexcept;


	/**
	 * @brief
	 *     Hash function to access the hash value of a `symbol_entry`.
	 *
	 */
	struct symbol_entry_ptr_hash
	{

		/**
		 * @brief
		 *     `return`s the hash value of a `symbol_entry`.
		 *
		 * This function uses the pre-computed hash value so it is a
		 * constant-time operation.
		 *
		 * @tparam SmartPtrT
		 *     (smart) pointer type holding the `symbol_entry`
		 *
		 * @param entry
		 *     smart pointer to a `symbol_entry`
		 *
		 * @returns
		 *     `entry->hash`
		 *
		 */
		template<typename SmartPtrT>
		std::size_t operator()(const SmartPtrT& entry) const noexcept;

	};


	/**
	 * @brief
	 *     Equality function to test whether the string values of two
	 *     `symbol_entry`s are equal.
	 *
	 */
	struct symbol_entry_ptr_equal
	{

		/**
		 * @brief
		 *     Tests whether the string values of two `symbol_entry`s are
		 *     equal.
		 *
		 * This function compares the string data so it is a linear-time
		 * operation.
		 *
		 * @tparam SmartPtrT
		 *     (smart) pointer type holding the `symbol_entry`
		 *
		 * @param lhs
		 *     first `symbol_entry`
		 *
		 * @param rhs
		 *     second `symbol_entry`
		 *
		 * @returns
		 *     whether both `symbol_entry` have the same length and data
		 *
		 */
		template <typename SmartPtrT>
		bool operator()(const SmartPtrT& lhs, const SmartPtrT& rhs) const noexcept;

	};

}  // namespace minijava


#define MINIJAVA_INCLUDED_FROM_SYMBOL_ENTRY_HPP
#include "symbol_entry.tpp"
#undef MINIJAVA_INCLUDED_FROM_SYMBOL_ENTRY_HPP
