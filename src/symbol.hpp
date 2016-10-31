/**
 * @file symbol.hpp
 *
 * @brief
 *     Canonical symbol representations.
 *
 */

#pragma once

#include <cassert>
#include <iosfwd>
#include <functional>
#include <cstring>
#include <type_traits>
#include <memory>

#include <boost/core/noncopyable.hpp>

#include "global.hpp"

namespace minijava
{
	/**
	 * @brief
	 *     A structure to ensure the connection beween a symbol and its pool.
	 *
	 * Should be created via a shared_ptr by a pool and given to symbols created by the same pool.
	 * The symbols will then hold a reference to this anchor as long as they exist.
	 * If the pool is destroyed it sets the pool_available property to false.
	 * This way a symbol can check whether its pool still exists.
	 *
	 * The `tag` is used to compare if two symbols come from the same pool.
	 */
	struct symbol_debug_pool_anchor: private boost::noncopyable
	{
		/**
		 * @brief
		 *     Creates an anchor with a unique tag.
		 */
		symbol_debug_pool_anchor()
			: tag(this)
		{}

		/**
		 * @brief
		 *     Creates an anchor with a custom tag.
		 *
		 * @param tag
		 *     The tag of this anchor
		 */
		symbol_debug_pool_anchor(const void* tag)
			: tag(tag)
		{
		}

		// 5gon12eder: Why is this `tag` data member needed?  Can't you just
		// compare the address of the object directly?

		/** @brief Tag to compare if two symbols come from the same pool */
		void const * const tag;

		// 5gon12eder: This data member isn't needed either.  You can get this
		// information from the (strong) reference count of the
		// `std::shared_ptr`.

		/** @brief Indicates whether the pool is still constructed */
		bool pool_available = true;
	};

	/**
	 * @brief
	 *     Underlying entry for a symbol.
	 *
	 * Must be provided and owned by an external factory.
	 * If a symbol used after the factory has been deconstructed the behaviour is undefined.
	 */

	// 5gon12eder: Why are pools now called "factories" in the above DocString?
	//
	// I find this allocation and deallocation logic here an unfortunate mixing
	// of concerns.  This logic belongs into the pool implementation.  It also
	// clutters this public header file with implementation details (that
	// should go into a `*.tpp` file).  Implementing this here also needlessly
	// ties all pool implementations to `std::hash<std::string>` which is
	// particularly unfortunate as it means you always have to construct a
	// `std::string` just to get the hash value.  I don't see any downside in
	// making the allocation and deallocation functions free helper function in
	// a `detail` `namespace` in `symbol_pool.tpp` ans this `struct` just three
	// data members.

	struct symbol_entry final: private boost::noncopyable
	{
	public:
		/**
		 * @brief
		 *     Deleter functor for symbol_entrys
		 */
		template<typename AllocT>
		struct entry_deleter
		{
			using allocator_type = AllocT;
			using allocator_traits = std::allocator_traits<allocator_type>;

			// 5gon12eder: I think this `static_assert`ion is overly
			// restrictive.  It would make sense to use a
			// `std::allocator<symbol_entry>` and it would just work fine.  All
			// you have to to is rebind it to `char`.

			static_assert(std::is_same<char, typename allocator_traits::value_type>::value, "Allocator does not allocate char!");

			entry_deleter(const allocator_type& alloc)
				: _alloc(alloc)
			{
			}

			void operator()(const symbol_entry* entry)
			{
				const char * mem = reinterpret_cast<const char*>(entry);
				const auto mem_size = _struct_size(entry->size);
				allocator_traits::deallocate(_alloc, const_cast<char*>(mem), mem_size);
			}
		private:
			allocator_type _alloc;
		};

		template<typename AllocT>
		using ptr = std::unique_ptr<const symbol_entry, entry_deleter<AllocT>>;

	public:

		/** @brief The precomputed hash of the symbol */
		std::size_t hash;

		/** @brief Size of the symbol's string */
		std::size_t size;

		/** @brief The actual string. Must be NUL-terminated */
		char cstr[1];

		/**
		 * @brief
		 *     Allocates a symbol_entry
		 *
		 * This will allocate one continuous chunk of memory and copy
		 * all necessary data into it.
		 *
		 * @param alloc
		 *     The allocator that should be used to allocate the memory
		 *
		 * @param str
		 *     The string value of the symbol.
		 *
		 * @returns
		 *     The newly created symbol_entry
		 */
		template<typename AllocT>
		static ptr<AllocT> allocate(AllocT& alloc, const std::string& str)
		{
			using alloc_traits = std::allocator_traits<AllocT>;
			using deleter = entry_deleter<AllocT>;
			using non_const_ptr = std::unique_ptr<symbol_entry, deleter>;
			static_assert(std::is_same<char, typename alloc_traits::value_type>::value, "Allocator does not allocate char!");

			std::hash<std::string> hash_fn;
			auto hash = hash_fn(str);

			const auto mem_size = _struct_size(str.size());
			auto entry = non_const_ptr(reinterpret_cast<symbol_entry*>(alloc_traits::allocate(alloc, mem_size)), deleter(alloc));

			entry->hash = hash;
			entry->size = str.size();
			// 5gon12eder: Could be done in just one line:
			// std::copy(str.c_str(), str.c_str() + str.size() + 1, entry->cstr);
			std::copy(str.begin(), str.end(), entry->cstr);
			entry->cstr[str.size()] = '\0';

			return entry;
		}


		/**
		 * @brief
		 *     Returns a spcial entry for the empty symbol.
		 *
		 * @returns
		 *     Entry to the empty symbol
		 *
		 */

		// 5gon12eder: The DocString should say that the `symbol_entry` is
		// statically allocated and also, that pools *must* use this special
		// value for the empty string.
		//
		// As already discussed, I would prefer to define the hash of the empty
		// string as 0 and leave the hashes of all other values unspecified.
		// Then this function could also be marked `noexcept` which is
		// important if you want to restore an invariant and need just some
		// symbol for it.

		static const symbol_entry * get_empty_symbol_entry()
		{
			const auto hash = std::hash<std::string>()(std::string(""));
			static symbol_entry entry;
			entry.cstr[0] = '\0';
			entry.size = 0;
			entry.hash = hash;
			return &entry;
		}

	private:

		// 5gon12eder: This function is called exactly once and all it does is
		// adding two numbers.  That's quite a few lines of source code for
		// that.

		/**
		 * @brief
		 *     Calculates the needed size for an entry.
		 *
		 * @param length
		 *     The length of the symbol string
		 *
		 * @returns
		 *     The needed size for an entry with the given string.
		 */
		static std::size_t _struct_size(std::size_t length) noexcept
		{
			return sizeof(symbol_entry) + length;
		}
	};
	// 5gon12eder: You can use the implicit conversion operator of
	// `std::bool_constant` to `bool`.  Then you don't have to spell out
	// `::value` each time.  It's also more straight-forward, IMHO.
	static_assert(std::is_pod<symbol_entry>::value, "");

	namespace detail {

		// To be consistent with the nomenclature, it should be
		// `symbol_debug_base`, though.  This `class` doesn't even do the
		// `assert`ions.

		struct symbol_assertion_base
		{
			symbol_assertion_base(const std::shared_ptr<const symbol_debug_pool_anchor>& anchor)
				: _debugAnchor(anchor)
			{
				// 5gon12eder: `anchor.get() == nullptr` should be allowed for
				// when there is no pool or it doesn't support refcounting.
				// Minor: Anyway, I'd prefer to just use the contextual
				// conversion of `std::shared_ptr` to `bool`.  Comparing with a
				// `std::nullptr_t` doesn't make things clearer, in my opinion.
				// (And it's more typing.)
				assert(anchor != nullptr);
			}

			static bool have_compatible_pool(const symbol_assertion_base& lhs, const symbol_assertion_base& rhs) noexcept
			{
				// 5gon12eder: Could just compare the addresses as mentioned
				// above.  But as far as I can tell, you also have to handle
				// the case where either pool is the static "pool" for the
				// empty string.  If the static pool's tag is identified by
				// the `nullptr` (as suggested above) then this check becomes
				//
				// const auto lp = lhs._debugAnchor.get();
				// const auto rp = rhs._debugAnchor.get();
				// return (lp == nullptr) || (rp == nullptr) || (lp == rp);
				return lhs._debugAnchor->tag == rhs._debugAnchor->tag;
			}

			// 5gon12eder: Should be marked `noexcept`.
			bool is_pool_available() const
			{
				// 5gon12eder: Could just ask the reference count as mentioned
				// above.
				return _debugAnchor->pool_available;
			}

		private:
			/** @brief Reference to a pool anchor for some checks */
			// 5gon12eder: Should be a `std::weak_ptr`.
			std::shared_ptr<const symbol_debug_pool_anchor> _debugAnchor;
		};

		// 5gon12eder: I dislike casting to `void` just to silence a compiler
		// warning about unused variables.  Just don't give the parameter a
		// name.  It's also less to type.

		struct symbol_release_base
		{
			symbol_release_base(const std::shared_ptr<const symbol_debug_pool_anchor>& anchor)
			{
				(void) anchor;
			}

			static bool have_compatible_pool(const symbol_release_base& lhs, const symbol_release_base& rhs) noexcept
			{
				(void) lhs;
				(void) rhs;
				return true;
			}

			bool is_pool_available() const noexcept
			{
				return true;
			}
		};

		// 5gon12eder: Good refactoring.  ;-)
		// But could save typing by using `std::conditional_t`.

		using symbol_base = std::conditional<MINIJAVA_ASSERT_ACTIVE, symbol_assertion_base, symbol_release_base>::type;
	}
	/**
	 * @brief
	 *     A non-owning read-only wrapper around a char sequence.
	 *
	 * Instances of this type have exactly one non-`static` data member which
	 * is a pointer to an internal string reperesentation.  The pointed-to
	 * internal structure is not owned by the symbol itself, but by a so called
	 * pool. Symbols can only be created by pools and remain only valid for use as long
	 * as the pool exists. Equality on `symbol` objects is defined in terms
	 * of pointer identity.  Two symbols created by different pools must not be
	 * compared in any way, if not stated anyway.
	 * The properties of a symbol are stored inside the internal
	 * symbol structure and are accessible by various methods.
	 */
	class symbol final: protected detail::symbol_base
	{
		friend struct std::hash<symbol>;
	public:
		/**
		 * @brief
		 *     Creates an empty symbol.
		 *
		 * The default constructed empty symbol is a special case,
		 * that lives in a special static pool.
		 *
		 * It is allowed to compare two empty symbols regardless of
		 * the pool they were created in.
		 *
		 */

		// 5gon12eder: With a little refactoring (as explained above) this
		// constructor could be `noexcept` which would make it perfect.

		symbol()
			: symbol(_get_empty_symbol())
		{
		}

		/**
		 * @brief
		 *     Creates a `symbol` from a symbol_entry.
		 *
		 * The symbol_entry must be created and owned by some kind of pool.
		 *
		 * @param entry
		 *     The entry, this symbol will point to
		 *
		 * @param anchor
		 *     An anchor that is used to check some pool properties.
		 */

		// 5gon12eder: If you take the `anchor` by-value and `std::move` it
		// into its destination, you can save two atomic operations.  But you
		// should't store the `std::shared_ptr` anyway but a `std::weak_ptr`
		// obtained from it as explained above.
		//
		// The DocString for `anchor` doesn't really tell me what parameter I
		// should pass.

		explicit symbol(const symbol_entry * entry, const std::shared_ptr<const symbol_debug_pool_anchor>& anchor)
			: detail::symbol_base(anchor)
			, _entry(entry)
		{
			assert(entry != nullptr);
		}

	public:
		using value_type      = char;                   ///< char
		using traits_type     = std::char_traits<char>; ///< char_traits<char>
		using const_reference = const char&;            ///< const char&
		using reference       = const_reference;        ///< const char&
		using const_pointer   = const char *;           ///< const char *
		using pointer         = const_pointer;          ///< const char *
		using const_iterator  = const_pointer;          ///< const char *
		using iterator        = const_iterator;         ///< const char *
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<iterator>;
		using difference_type = std::ptrdiff_t;         ///< std::ptrdiff_t
		using size_type       = std::size_t;            ///< std::size_t

	public:

		/**
		 * @brief
		 *     `return`s the canonical pointer to the underlying NUL-terminated
		 *     character sequence.
		 *
		 * @returns
		 *     canonical pointer
		 *
		 */
		const char * c_str() const noexcept
		{
			return entry()->cstr;
		}

		/**
		 * @brief
		 *     Returns the length of the string, in terms of bytes.
		 *
		 * Note that string objects handle bytes without knowledge of the encoding
		 * that may eventually be used to encode the characters it contains.
		 * Therefore, the value returned may not correspond to the actual number of
		 * encoded characters in sequences of multi-byte or variable-length characters (such as UTF-8).
		 *
		 * Both symbol::size and symbol::length are synonyms and return the same value.
		 *
		 * @returns
		 *     The number of bytes in the string.
		 *
		 */
		size_type size() const noexcept
		{
			return entry()->size;
		}

		/**
		 * @brief
		 *     Returns the length of the string, in terms of bytes.
		 *
		 * Note that string objects handle bytes without knowledge of the encoding
		 * that may eventually be used to encode the characters it contains.
		 * Therefore, the value returned may not correspond to the actual number of
		 * encoded characters in sequences of multi-byte or variable-length characters (such as UTF-8).
		 *
		 * Both symbol::size and symbol::length are synonyms and return the same value.
		 *
		 * @returns
		 *     The number of bytes in the string.
		 *
		 */
		size_type length() const noexcept
		{
			return size();
		}

		/**
		 * @brief
		 *     Get C string equivalent.
		 *
		 * Returns a pointer to an array that contains a null-terminated sequence
		 * of characters (i.e., a C-string) representing the value of the symbol.
		 *
		 * This array includes the same sequence of characters that make up the
		 * value of the symbol plus an additional terminating null-character ('\0') at the end.
		 *
		 * @returns
		 *     A pointer to the c-string representation of the symbol's value.
		 *
		 */
		const char * data() const noexcept
		{
			return entry()->cstr;
		}

		/**
		 * @brief
		 *     Return iterator to beginning
		 *
		 * Returns an iterator pointing to the first character of the symbol.
		 *
		 * @returns
		 *     An iterator to the beginning of the symbol.
		 *
		 */
		const_iterator begin() const noexcept
		{
			return cbegin();
		}

		/**
		 * @brief
		 *     Return iterator to end
		 *
		 * Returns an iterator pointing to the past-the-end character of the symbol.
		 *
		 * @returns
		 *     An iterator to the end of the symbol.
		 *
		 */
		const_iterator end() const noexcept
		{
			return cend();
		}

		/**
		 * @brief
		 *     Return iterator to beginning
		 *
		 * Returns an iterator pointing to the first character of the symbol.
		 *
		 * @returns
		 *     An iterator to the beginning of the symbol.
		 *
		 */
		const_iterator cbegin() const noexcept
		{
			return entry()->cstr;
		}

		/**
		 * @brief
		 *     Return iterator to end
		 *
		 * Returns an iterator pointing to the past-the-end character of the symbol.
		 *
		 * @returns
		 *     An iterator to the end of the symbol.
		 *
		 */
		const_iterator cend() const noexcept
		{
			return cbegin() + size();
		}

		/**
		 * @brief
		 *     Return a reverse iterator to end
		 *
		 * Returns a reverse iterator pointing to the last character of the symbol.
		 *
		 * @returns
		 *     A reverse iterator to the end of the symbol.
		 *
		 */
		const_reverse_iterator rbegin() const noexcept
		{
			return crbegin();
		}

		/**
		 * @brief
		 *     Return a reverse iterator to the beginning
		 *
		 * Returns an iterator pointing to the before-the-begin character of the symbol.
		 *
		 * @returns
		 *     A reverse iterator to the beginning of the symbol.
		 *
		 */
		const_reverse_iterator rend() const noexcept
		{
			return crend();
		}

		/**
		 * @brief
		 *     Return reverse iterator to end
		 *
		 * Returns a reverse iterator pointing to the last character of the symbol.
		 *
		 * @returns
		 *     A reverse iterator to the end of the symbol.
		 *
		 */
		const_reverse_iterator crbegin() const noexcept
		{
			return const_reverse_iterator(cend());
		}

		/**
		 * @brief
		 *     Return a reverse iterator to the beginning
		 *
		 * Returns an iterator pointing to the before-the-begin character of the symbol.
		 *
		 * @returns
		 *     A reverse iterator to the beginning of the symbol.
		 *
		 */
		const_reverse_iterator crend() const noexcept
		{
			return const_reverse_iterator(cbegin());
		}

		/**
		 * @brief
		 *     Test if symbol is empty.
		 *
		 * Returns whether the symbol is the empty symbol (i.e. whether its length is 0).
		 *
		 * @returns
		 *     `true` if the symbol length is 0, `false` otherwise.
		 *
		 */
		bool empty() const noexcept
		{
			return entry()->size == 0;
		}

		/**
		 * @brief
		 *     Get character in symbol.
		 *
		 * Returns the character at position `pos` in the symbol.
		 *
		 * If `pos` is less or equal to the symbols length,
		 * the function never throws exceptions (no-throw guarantee).
		 * Otherwise, it causes undefined behavior.
		 *
		 * @param pos
		 *     Value with the position of a character within the symbol.
		 *
		 * @returns
		 *     The character at the specified position in the symbol.
		 *
		 */
		char operator[](std::size_t pos) const
		{
			assert(pos < size());
			return entry()->cstr[pos];
		}

		/**
		 * @brief
		 *     Get character in symbol.
		 *
		 * Returns the character at position `pos` in the symbol.
		 *
		 * The function automatically checks whether `pos` is a valid
		 * position of a character in the string (i.e., whether `pos` is less than the symbol length),
		 * throwing an out_of_range exception if it is not.
		 *
		 * @param pos
		 *     Value with the position of a character within the symbol.
		 *
		 * @returns
		 *     The character at the specified position in the symbol.
		 *
		 * @throws std::out_of_range
		 *     If `pos` is not less than the symbol's length.
		 *
		 */
		char at(std::size_t pos) const
		{
			if(pos >= size())
				throw std::out_of_range("invalid position in symbol");
			return entry()->cstr[pos];
		}

		/**
		 * @brief
		 *     Returns the first character.
		 *
		 * This function shall not be called on the empty symbol.
		 * Doing so causes undefined behavior.
		 *
		 * @returns
		 *     The first charactor of the symbol.
		 *
		 */
		char front() const
		{
			assert(size() > 0);
			return *cbegin();
		}

		/**
		 * @brief
		 *     Returns the last character.
		 *
		 * This function shall not be called on the empty symbol.
		 * Doing so causes undefined behavior.
		 *
		 * @returns
		 *     The last charactor of the symbol.
		 *
		 */
		char back() const
		{
			assert(size() > 0);
			// 5gon12eder: I hope that the compiler can see through all this
			// indirection...
			return *crbegin();
		}

		/**
		* @brief
		*     Compares two `string`s for equality.
		*
		* @param lhs
		*     first `string` to compare
		*
		* @param rhs
		*     second `string` to compare
		*
		* @returns
		*     whether `lhs` and `rhs` are equal
		*
		*/
		friend bool operator==(const symbol& lhs, const symbol& rhs) noexcept
		{
			// 5gon12eder: I don't think that this check is needed.  If it is
			// meant to handle the case that empty `symbol`s can be compared
			// with `symbol`s from any pool, then it is not sufficient because
			// it only handles the case where both are empty.  So if this was
			// the intention, then
			//
			//     if (lhs.empty() != lhs.empty()) {
			//         return false;
			//     }
			//
			// would have been the correct condition.  But I'd find it more
			// elegant to handle this in the `have_compatible_pool` function.
			// Well, not only more elegant but also more performant because it
			// wouldn't bother us in release mode.
			if(lhs.empty() && rhs.empty())
				return true;

			assert(have_compatible_pool(lhs, rhs));
			return (lhs._entry == rhs._entry);
		}

		/**
		* @brief
		*     Compares two `string`s for inequality.
		*
		* @param lhs
		*     first `string` to compare
		*
		* @param rhs
		*     second `string` to compare
		*
		* @returns
		*     whether `lhs` and `rhs` are unequal
		*
		*/
		friend bool operator!=(const symbol& lhs, const symbol& rhs) noexcept
		{
			return !(lhs == rhs);
		}


	private:
		/**
		 * @brief
		 *     Returns a static allocated empty symbol
		 *
		 * @returns
		 *     The empty symbol
		 */
		static symbol _get_empty_symbol()
		{

			// 5gon12eder: If applying the above suggestions then you can just
			// use an empty (default-constructed) `std::weak_ptr` here and need
			// no memory allocation.  And then this function can also be
			// `noexcept` as it should.

			static const symbol_entry * empty_symbol_entry_ptr = symbol_entry::get_empty_symbol_entry();
			static const auto anchor = std::make_shared<symbol_debug_pool_anchor>(nullptr);

			return symbol(empty_symbol_entry_ptr, anchor);
		}

		// 5gon12eder: This DocString didn't make sense to me.

		/**
		 * @brief
		 *     Returns if the internal entry.
		 *
		 * In debug mode this checks if the pool is still constructed.
		 *
		 * @returns
		 *     The internal entry.
		 */
		inline const symbol_entry * entry() const
		{
			assert(is_pool_available());
			return _entry;
		}

		/** @brief The internal entry. */
		const symbol_entry * _entry;
	};  // class symbol


	// 5gon12eder: The `static_symbol_pool` really shouldn't be in this header
	// file.  It should have its own and -- since there is no valid reason to
	// ever use it in the program -- this header file should be in `testaux`.

	/**
	 * @brief
	 *     Used to allocate one symbol.
	 *
	 * The static_symbol_pool can be used to create a single symbol.
	 * Its special property however is, that its created symbols,
	 * are allowed to be compared with one another.
	 * The programmer has to take care that only one static_symbol_pool
	 * is created per possible string value! Two symbols with the same
	 * content from different static_symbol_pools will NOT compare equal!
	 * The only exception to this rule is the empty symbol.
	 */
	class static_symbol_pool: private boost::noncopyable
	{
	private:
		using entryptr_type = symbol_entry::ptr<std::allocator<char>>;
	public:
		/**
		 * @brief
		 *     Constructs the static_symbol_pool with a given string
		 *
		 * @param str
		 *     The content for the symbol that is created by this pool.
		 */
		static_symbol_pool(const std::string& str)
			: _entry(symbol_entry::allocate(_allocator, str))
		{
			// create anchor with nullptr tag, so all symbols from a static_symbol_pool have the same tag
			_anchor = std::make_shared<symbol_debug_pool_anchor>(nullptr);
		}

		/**
		 * @brief
		 *     Destructs the static_symbol_pool.
		 */
		~static_symbol_pool()
		{
			_anchor->pool_available = false;
		}

		/**
		 * @brief
		 *     Returns the pool's symbol
		 *
		 * @returns
		 *     the one and only symbol of this pool.
		 */
		symbol get() const
		{
			return symbol(_entry.get(), _anchor);
		}

	private:
		std::allocator<char> _allocator;
		std::shared_ptr<symbol_debug_pool_anchor> _anchor;
		entryptr_type _entry;
	};


	/**
	 * @brief
	 *     Inserts the character sequence referred to by the symbol
	 *     into an output stream.
	 *
	 * @param os
	 *     stream to write to
	 *
	 * @param str
	 *     `string` to stream out
	 *
	 * @returns
	 *     a reference to `os`
	 *
	 */
	std::ostream&
	operator<<(std::ostream& os, const symbol& str);

}  // namespace minijava

namespace std {

	// 5gon12eder: s/calculate/obtain/ since it is O(1).

	/**
	 * @brief
	 *     Hash function to calculate the hash value of a symbol
	 */
    template<>
    struct hash<minijava::symbol>
    {
		/**
		 * @brief
		 *     Returns the hash value of a symbol
		 *
		 * @param symbol
		 *     A symbol for that the hash value should be returned
		 *
		 * @returns
		 *     The hash value of the symbol
		 */
        constexpr std::size_t operator()(const minijava::symbol& symbol) const noexcept
        {
            return symbol._entry->hash;
        }
    };
}
