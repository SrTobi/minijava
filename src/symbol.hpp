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

#include "global.hpp"

namespace minijava
{
	struct symbol_debug_pool_anchor
	{
		symbol_debug_pool_anchor(const void* tag)
			: tag(tag)
		{
		}

		const void* tag;
	};

	/**
		* @brief
		*     Underlying entry for a symbol.
		*
		* Must be provided and owned by an external factory.
		* If the symbols lifetime exceeds the factory the behaviour is undefined.
		*/
	struct symbol_entry
	{
		/**
			* @brief
			*     Constructs a symbol_entry
			*
			* @param cstr
			*     A pointer to a NUL-terminated string
			*
			* @param size
			*     Lenght of the NUL-terminated string (without the NUL)
			*
			* @param hash
			*     Hash value of the NUL-terminated string.
			*
			* Must be the same as the hash of the std::string version of the same string.
			*
			* @param pool
			*     Pointer to the pool this entry lives in.
			*
			* In debug mode it is used to verify that symbols which are about to be compared
			* were created by the same pool.
			*/
		explicit symbol_entry(const char * cstr, std::size_t size, std::size_t hash)
			: cstr(cstr)
			, size(size)
			, hash(hash)
		{
			assert(size > 0);
			assert(std::strlen(cstr) == size);
			assert(hash == std::hash<std::string>()(std::string(cstr)));
		}


		/** Pointer to the actual string. Must be NUL-terminated */
		char const * const cstr;

		/** Size of the symbol's string */
		const std::size_t size;

		/** The precomputed hash of the symbol */
		const std::size_t hash;
	};

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
	 * pool. Symbols can only be created by pools and remain only valid as long
	 * as the pool exists. Equality on `symbol` objects is defined in terms
	 * of pointer identity.  Two symbols created by different pools must not be
	 * compared in any way. The properties of a symbol are stored inside the internal
	 * symbol structure and are accessible by various methods.
	 */
	class symbol final: protected detail::symbol_base
	{
		friend struct std::hash<symbol>;
	public:

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
			return _entry->cstr;
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
			return _entry->size;
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
			return _entry->cstr;
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
			return _entry->cstr;
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
			return _entry->size == 0;
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
			return _entry->cstr[pos];
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
			return _entry->cstr[pos];
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
			return *crend();
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
		*     whether `lhs` and `rhs` point to the same internal symbol entry
		*
		*/
		friend bool operator==(const symbol& lhs, const symbol& rhs) noexcept
		{
			if(lhs.empty() || rhs.empty())
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
		*     whether `lhs` and `rhs` wrap different canonical pointers
		*
		*/
		friend bool operator!=(const symbol& lhs, const symbol& rhs) noexcept
		{
			return !(lhs == rhs);
		}


	private:
		static symbol _get_empty_symbol()
		{
			const auto cstr = "";
			const auto hash = std::hash<std::string>()(std::string(cstr));
			const static symbol_entry empty_symbol_entry(cstr, 0, hash);
			const static auto anchor = std::make_shared<symbol_debug_pool_anchor>(nullptr);

			return symbol(&empty_symbol_entry, anchor);
		}

		/** @brief The internal entry. */
		const symbol_entry * _entry;
	};  // class symbol


	class static_symbol_pool
	{
	public:
		static_symbol_pool(std::string str)
			: _symbolString(std::move(str))
		{
			const auto cstr = _symbolString.c_str();
			const auto size = _symbolString.size();
			const auto hash = std::hash<std::string>()(_symbolString);

			_entry = std::make_unique<symbol_entry>(cstr, size, hash);

			// create anchor with nullptr tag, so all symbols from a static_symbol_pool have the same tag
			_anchor = std::make_shared<symbol_debug_pool_anchor>(nullptr);
		}

		~static_symbol_pool()
		{
			assert(_anchor.unique());
		}

		symbol get() const
		{
			return symbol(_entry.get(), _anchor);
		}

	private:
		std::shared_ptr<symbol_debug_pool_anchor> _anchor;
		std::unique_ptr<symbol_entry> _entry;
		const std::string _symbolString;
	};


	/**
	 * @brief
	 *     Inserts the character sequence referred to by the wrapped pointer
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
