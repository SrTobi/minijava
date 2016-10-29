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
	 *     A structure to ensure the connection beween a symbol and it's pool.
	 *
	 * Should be created via a shared_ptr by a pool and given to symbols created by the same pool.
	 * The symbols will then hold a reference to this anchor as long as they exist.
	 * If the pool is destroyed it sets the pool_available property to false.
	 * This way a symbol can check wether it's pool still exists.
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

		/** @brief Tag to compare if two symbols come from the same pool */
		const void* tag;

		/** @brief Indicates wether the pool is still constructed */
		bool pool_available = true;
	};

	/**
	 * @brief
	 *     Underlying entry for a symbol.
	 *
	 * Must be provided and owned by an external factory.
	 * If a symbol used after the factory has been deconstructed the behaviour is undefined.
	 */
	struct symbol_entry final: private boost::noncopyable
	{
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
		static const symbol_entry * allocate(AllocT& alloc, const std::string& str)
		{
			using alloc_traits = std::allocator_traits<AllocT>;
			static_assert(std::is_same<char, typename alloc_traits::value_type>::value, "Allocator does not allocate char!");

			std::hash<std::string> hash_fn;
			auto hash = hash_fn(str);

			const auto mem_size = _struct_size(str.size());
			symbol_entry * entry = reinterpret_cast<symbol_entry*>(alloc_traits::allocate(alloc, mem_size));

			entry->hash = hash;
			entry->size = str.size();
			std::copy(str.begin(), str.end(), entry->cstr);
			entry->cstr[str.size()] = '\0';

			return entry;
		}

		/**
		 * @brief
		 *     Deallocates a given symbol_entry
		 *
		 * @param alloc
		 *     The allocator that should be used to deallocate the memory.
		 *
		 * This should be the same that was used to create the symbol_entry.
		 *
		 * @param entry
		 *     The entry that should be deallocated.
		 *
		 */
		template<typename AllocT>
		static void deallocate(AllocT& alloc, const symbol_entry* entry)
		{
			using alloc_traits = std::allocator_traits<AllocT>;
			static_assert(std::is_same<char, typename alloc_traits::value_type>::value, "Allocator does not allocate char!");

			const char * mem = reinterpret_cast<const char*>(entry);
			const auto mem_size = _struct_size(entry->size);
			alloc_traits::deallocate(alloc, const_cast<char*>(mem), mem_size);
		}

		/**
		 * @brief
		 *     Returns a spcial entry for the empty symbol.
		 *
		 * @returns
		 *     Entry to the empty symbol
		 *
		 */
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
	static_assert(std::is_pod<symbol_entry>::value, "");

	namespace detail {

		struct symbol_assertion_base
		{
			symbol_assertion_base(const std::shared_ptr<const symbol_debug_pool_anchor>& anchor)
				: _debugAnchor(anchor)
			{
				assert(anchor != nullptr);
			}

			static inline bool have_compatible_pool(const symbol_assertion_base& lhs, const symbol_assertion_base& rhs) noexcept
			{
				return lhs._debugAnchor->tag == rhs._debugAnchor->tag;
			}

			inline bool is_pool_available() const
			{
				return _debugAnchor->pool_available;
			}

		private:
			/** @brief Reference to a pool anchor for some checks */
			std::shared_ptr<const symbol_debug_pool_anchor> _debugAnchor;
		};

		struct symbol_release_base
		{
			symbol_release_base(const std::shared_ptr<const symbol_debug_pool_anchor>& anchor)
			{
				(void) anchor;
			}

			static inline bool have_compatible_pool(const symbol_release_base& lhs, const symbol_release_base& rhs) noexcept
			{
				(void) lhs;
				(void) rhs;
				return true;
			}

			inline bool is_pool_available() const noexcept
			{
				return true;
			}
		};

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
		 *     An iterator to the beginning of the symbol.
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
		 *     An iterator to the beginning of the symbol.
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
			static const symbol_entry * empty_symbol_entry_ptr = symbol_entry::get_empty_symbol_entry();
			static const auto anchor = std::make_shared<symbol_debug_pool_anchor>(nullptr);

			return symbol(empty_symbol_entry_ptr, anchor);
		}

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

	/**
	 * @brief
	 *     Used to allocate one symbol.
	 *
	 * The static_symbol_pool can be used to create a single symbol.
	 * It's special property however is, that it's created symbols,
	 * are allowed to be compared with one another.
	 * The programmer has to take care that only one static_symbol_pool
	 * is created per possible string value! Two symbols with the same
	 * content from different static_symbol_pools will NOT compare equal!
	 * The only exception to this rule is the empty symbol.
	 */
	class static_symbol_pool: private boost::noncopyable
	{
	private:
		using entryptr_type = std::unique_ptr<const symbol_entry, std::function<void(const symbol_entry*)>>;
	public:
		/**
		 * @brief
		 *     Constructs the static_symbol_pool with a given string
		 *
		 * @param str
		 *     The content for the symbol that is created by this pool.
		 */
		static_symbol_pool(const std::string& str)
		{
			std::function<void(const symbol_entry*)> deleter = [this](const symbol_entry* entry)
			{
				symbol_entry::deallocate(_allocator, entry);
			};

			_entry = entryptr_type(symbol_entry::allocate(_allocator, str), deleter);

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
