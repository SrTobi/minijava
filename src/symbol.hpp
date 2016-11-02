/**
 * @file symbol.hpp
 *
 * @brief
 *     Canonical symbol representations.
 *
 * This component provides the `symbol` `class` which is a very light-weight
 * non-owning type that refers to a canonical representatio of a symbol
 * (character string).  A `symbol` points to a `symbol_entry` that is owned by
 * a *symbol pool*.  As such, a `symbol` acts much like an iterator -- except
 * that it cannot be incremented.
 *
 * Likewise, `symbol`s from differnt pools must not be compared to each other,
 * just as comparing iterators into different containes is not allowed.
 * However, the empty `symbol` (that holds the empty string) may be compared
 * with any other `symbol` handed out by any other pool.  To make this work,
 * pool implementations are required to hand out a `symbol` with an entry
 * pointer that references the singleton `return`ed by the
 * `get_empty_symbol_entry` function, when asked to create a canonical
 * representatoin of the empty string.  A default-constructed `symbol` also
 * references the singleton empty symbol entry which is a preferable high-level
 * alternative to create an empty symbol, even without a pool object.
 *
 * This component comes in two flavours: *debug* and *release*.  If the macro
 * `MINIJAVA_ASSERT_ACTIVE` is `#define`d to a constant integral expression
 * that evaluates to a value other than zero, debug configuration is selected.
 * Otherwise, if it is `#define`d to zero, release configuration.  If the macro
 * is not `#define`d at all ot its expansion is not a constant integral
 * expression, the behavior is undefined.  The two configurations are not
 * binary compatible.  Furthermore, in debug configuration, some functions
 * marked as `noexcept` might allocate memory so -- under very tragic
 * circumstances -- `std::terminate` might be called without prior warning.
 *
 * In release configuration, `symbol`s are just wrapped pointers.  In debug
 * configuration, they store an additional reference (called *anchor* in the
 * following) to the symbol pool that handed them out.  The anchor allows some
 * defensive checks to ensure that a `symbol` is not used after its pool got
 * destroyed.  These checks are only a best effort and cannot catch all
 * cases of undefined behavior.
 *
 * The following explanation is not required to use `symbol`s effectively but
 * might be helpful for implementing new symbol pools or just worth knowing.
 *
 * Here is how the situation looks in debug configuration.
 *
 *       owning
 *       pointer  +------------+  non-owning pointer              +------+
 *          +---->|symbol_entry|<---------------------------------|symbol|
 *          |     +------------+                                  +------+
 *          |     |symbol_entry|                                      |
 *          |     +------------+  non-owning +------+                 |
 *          |     |symbol_entry|<------------|symbol|                 |
 *          |     +------------+   pointer   +------+                 |
 *          |     | ...        |               |                      |
 *          |     +------------+               |std::weak_ptr         |
 *          |     |symbol_entry|               |                      |
 *          |     +------------+               |                      |
 *          |                                  V                      |
 *      +------+      std::shared_ptr      +------+   std::weak_ptr   |
 *      |symbol|-------------------------->|anchor|<------------------+
 *      | pool |                           +------+
 *      +------+
 *
 * How a symbol pool manages ownership of its `symbol_entry`s is its own
 * implementation detail.  The memory layout shown above is just an example (a
 * good one, though).  If an operation on a `symbol` is performed that involves
 * dereferencing the pointer to its `symbol_entry`, it `assert()`s that the
 * anchor has not expired yet.  Likewise, if two `symbol`s are compared, the
 * comparison operator `assert()`s that they both reference the same anchor (as
 * comparing `symbol`s from different pools is undefined behavior) or that
 * either of them is the empty symbol (as the empty symbol is a special case
 * and may be compared with any other symbol).
 *
 * In release configuration, the situation looks like this and no checks are
 * performed.
 *
 *       owning
 *       pointer  +------------+  non-owning pointer              +------+
 *          +---->|symbol_entry|<---------------------------------|symbol|
 *          |     +------------+                                  +------+
 *          |     |symbol_entry|
 *          |     +------------+  non-owning pointer  +------+
 *          |     |symbol_entry|<---------------------|symbol|
 *          |     +------------+                      +------+
 *          |     | ...        |
 *          |     +------------+
 *          |     |symbol_entry|
 *          |     +------------+
 *          |
 *      +------+
 *      |symbol|
 *      | pool |
 *      +------+
 *
 * Pool implementations must provide the constructor of a `symbol` with a
 * pointer to their anchor.  In release configuration, this can be a `nullptr`.
 *
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <cstring>
#include <functional>
#include <iosfwd>
#include <memory>
#include <string>
#include <type_traits>

#include "global.hpp"
#include "symbol_entry.hpp"
#include "symbol_anchor.hpp"

#define MINIJAVA_INCLUDED_FROM_SYMBOL_HPP
#include "symbol_base.tpp"
#undef MINIJAVA_INCLUDED_FROM_SYMBOL_HPP


namespace minijava
{

	/**
	 * @brief
	 *     A non-owning read-only wrapper around a character sequence with a
	 *     canonical address and a precomputed hash value.
	 *
	 * In release configuration, instances of this type have exactly one
	 * non-`static` data member which is a pointer to an internal string
	 * reperesentation.  The pointed-to internal structure is not owned by the
	 * symbol itself, but by a so called pool.  Symbols may only be created by
	 * pools and remain valid for use unly as long as the pool exists.
	 * Equality on `symbol` objects is defined in terms of address identity of
	 * their internal representations.  Two symbols created by different pools
	 * must not be compared in any way, if not stated anyway.
	 *
	 */
	class symbol final: private detail::symbol_base
	{
	public:

		/**
		 * @brief
		 *     Creates an empty symbol.
		 *
		 * The default constructed empty symbol is a special case, that lives
		 * in a special static pool.  It may be compared with any other symbol
		 * created by any other pool.
		 *
		 * In debug configuration, this function has to allocate memory the
		 * first time it is invoked hich might in turn call `std::terminate` if
		 * an out-of-memory condition occurs just at this moment.
		 *
		 */
		symbol() noexcept
			: detail::symbol_base{symbol_anchor::get_empty_symbol_anchor()}
			, _entry{get_empty_symbol_entry()}
		{
		}

		/**
		 * @brief
		 *     Creates a `symbol` from a `symbol_entry`.
		 *
		 * This constructor should only be used by pool implementations.
		 *
		 * This constructor may not be used to construct an ambiguous
		 * representation of the empty `symbol`.  `entryptr->size == 0` must be
		 * true if and only if `entryptr` is the pointer `return`ed by
		 * `get_empty_symbol_entry()`.  Otherwise, the behavior is undefined.
		 *
		 * The behavior is also undefined if `entryptr == nullptr`.
		 *
		 * @param entryptr
		 *     the `symbol_entry`, this `symbol` will refer to
		 *
		 * @param anchor
		 *     pool anchor used for defensive checks in debug configuration,
		 *     ignored in release configuration
		 *
		 */
		explicit symbol(const symbol_entry *const entryptr,
		                const std::weak_ptr<symbol_anchor>& anchor)
			: detail::symbol_base(anchor)
			, _entry(entryptr)
		{
			assert(entryptr != nullptr);
			assert((entryptr->size == 0) == (entryptr == get_empty_symbol_entry()));
		}

	public:

		/** @brief Alias for `char_traits<char>`. */
		using traits_type = std::char_traits<char>;

		/** @brief Type of the characters in a `symbol`. */
		using value_type = char;

		/** @brief Type alias for a `const` reference to a character in a `symbol`. */
		using const_reference = const char&;

		/** @brief Type alias for a `const` reference to a character in a `symbol`. */
		using reference = const_reference;

		/** @brief Type alias for a `const` pointer to a character in a `symbol`. */
		using const_pointer = const char *;

		/** @brief Type alias for a `const` pointer to a character in a `symbol`. */
		using pointer = const_pointer;

		/** @brief Type of the iterator over the characters in a `symbol`. */
		using const_iterator = const_pointer;

		/** @brief Type of the iterator over the characters in a `symbol`. */
		using iterator = const_iterator;

		/** @brief Type of the reverse iterator over the characters in a `symbol`. */
		using reverse_iterator = std::reverse_iterator<iterator>;

		/** @brief Type of the reverse iterator over the characters in a `symbol`. */
		using const_reverse_iterator = std::reverse_iterator<iterator>;

		/** @brief Integer type that can store the difference between any two characters in a `symbol`. */
		using difference_type = std::ptrdiff_t;

		/** @brief Integer type that can store the size of any `symbol`. */
		using size_type = std::size_t;

	public:

		/**
		 * @brief
		 *     `return`s the number of characters / bytes in the `symbol`.
		 *
		 * The `size` and `length` member functions are synonyms and `return`
		 * the same value.
		 *
		 * @returns
		 *     number of bytes in the `symbol`
		 *
		 */
		size_type size() const noexcept
		{
				return _get_entry()->size;
		}

		/**
		 * @brief
		 *     Synonym for `size`.
		 *
		 * @returns
		 *     number of bytes in the `symbol`
		 *
		 */
		size_type length() const noexcept
		{
			return size();
		}

		/**
		 * @brief
		 *     `return`s a hash value for the `symbol`.
		 *
		 * Since the hash value is pre-computed, this is a constant-time
		 * operation regardless of the `symbol`'s size.
		 *
		 * The hash function is defined by the pool that creates the symbol.
		 * Two symbols created by different pools may have different hash
		 * values even though their character data is equal.
		 *
		 * @returns
		 *     hash value
		 *
		 */
		size_type hash() const noexcept
		{
			return _get_entry()->hash;
		}

		/**
		 * @brief
		 *     `return`s the canonical pointer to the underlying NUL-terminated
		 *     character sequence.
		 *
		 * For two `symbol`s `s1` and `s2`, `s1 == s2` is equivalent to
		 * `s1.data() == s2.data()`.
		 *
		 * @returns
		 *     canonical pointer to NUL-terminated character data
		 *
		 */
		const char * c_str() const noexcept
		{
			return _get_entry()->data;
		}

		/**
		 * @brief
		 *     Synonym for `c_str`.
		 *
		 * @returns
		 *     canonical pointer to NUL-terminated character data
		 *
		 */
		const char * data() const noexcept
		{
			return _get_entry()->data;
		}

		/**
		 * @brief
		 *     `return`s an iterator to the beginning.
		 *
		 * The `return`ed iterator points to the first character of the
		 * `symbol`.
		 *
		 * @returns
		 *     iterator to the beginning
		 *
		 */
		const_iterator begin() const noexcept
		{
			return cbegin();
		}

		/**
		 * @brief
		 *     `return`s an iterator to the end.
		 *
		 * The `return`ed iterator pointins to the after-the-last character of
		 * the `symbol`.
		 *
		 * @returns
		 *     iterator to the beginning
		 *
		 */
		const_iterator end() const noexcept
		{
			return cend();
		}

		/**
		 * @brief
		 *     `return`s an iterator to the beginning.
		 *
		 * The `return`ed iterator points to the first character of the
		 * `symbol`.
		 *
		 * @returns
		 *     iterator to the beginning
		 *
		 */
		const_iterator cbegin() const noexcept
		{
			return _get_entry()->data;
		}

		/**
		 * @brief
		 *     `return`s an iterator to the end.
		 *
		 * The `return`ed iterator pointins to the after-the-last character of
		 * the `symbol`.
		 *
		 * @returns
		 *     iterator to the beginning
		 *
		 */
		const_iterator cend() const noexcept
		{
			return cbegin() + size();
		}

		/**
		 * @brief
		 *     `return`s a reverse iterator to the end.
		 *
		 * The `return`ed iterator points to the last character of the `symbol`
		 * and incrementing it moves towards the beginning.
		 *
		 * @returns
		 *     reverse iterator to the end
		 *
		 */
		const_reverse_iterator rbegin() const noexcept
		{
			return crbegin();
		}

		/**
		 * @brief
		 *     `return`s a reverse iterator to the beginning.
		 *
		 * The `return`ed iterator points to the before-the-first character of
		 * the `symbol` and decrementing it moves towards the end.
		 *
		 * @returns
		 *     reverse iterator to the end
		 *
		 */
		const_reverse_iterator rend() const noexcept
		{
			return crend();
		}

		/**
		 * @brief
		 *     `return`s a reverse iterator to the end.
		 *
		 * The `return`ed iterator points to the last character of the `symbol`
		 * and incrementing it moves towards the beginning.
		 *
		 * @returns
		 *     reverse iterator to the end
		 *
		 */
		const_reverse_iterator crbegin() const noexcept
		{
			return const_reverse_iterator(cend());
		}

		/**
		 * @brief
		 *     `return`s a reverse iterator to the beginning.
		 *
		 * The `return`ed iterator points to the before-the-first character of
		 * the `symbol` and decrementing it moves towards the end.
		 *
		 * @returns
		 *     reverse iterator to the end
		 *
		 */
		const_reverse_iterator crend() const noexcept
		{
			return const_reverse_iterator(cbegin());
		}

		/**
		 * @brief
		 *     Tests whether `symbol` is empty.
		 *
		 * @returns
		 *     `true` if the symbol size is 0, `false` otherwise
		 *
		 */
		bool empty() const noexcept
		{
			return _get_entry()->size == 0;
		}

		/**
		 * @brief
		 *     Get character in `symbol`.
		 *
		 * `return`s the character at position `pos` in the `symbol`.
		 *
		 * If `pos` is greater or equal to the `symbol`'s size, the behavior
		 * is undefined.
		 *
		 * @param pos
		 *     position of a character within the `symbol`
		 *
		 * @returns
		 *     character at the specified position in the `symbol`
		 *
		 */
		char operator[](std::size_t pos) const
		{
			assert(pos < size());
			return _get_entry()->data[pos];
		}

		/**
		 * @brief
		 *     Get character in `symbol`.
		 *
		 * `return`s the character at position `pos` in the `symbol`.
		 *
		 * The function automatically checks whether `pos` is a valid position
		 * of a character in the `symbol` (i.e., whether `pos` is less than the
		 * `symbol` size), throwing a `std::out_of_range` exception if it is
		 * not.
		 *
		 * @param pos
		 *     position of a character within the `symbol`
		 *
		 * @returns
		 *     character at the specified position in the `symbol`
		 *
		 * @throws std::out_of_range
		 *     if `pos` is not less than the `symbol`'s size
		 *
		 */
		char at(std::size_t pos) const
		{
			if (pos >= size()) {
				const auto msg = std::string{}
					+ "invalid position "
					+ std::to_string(pos)
					+ " in symbol of size "
					+ std::to_string(size());
				throw std::out_of_range{msg};
			}
			return _get_entry()->data[pos];
		}

		/**
		 * @brief
		 *     `return`s the first character of the `symbol`.
		 *
		 * This function shall not be called on the empty `symbol`.
		 * Doing so causes undefined behavior.
		 *
		 * @returns
		 *     first character
		 *
		 */
		char front() const
		{
			assert(size() > 0);
			return _get_entry()->data[0];
		}

		/**
		 * @brief
		 *     `return`s the last character of the `symbol`.
		 *
		 * This function shall not be called on the empty `symbol`.
		 * Doing so causes undefined behavior.
		 *
		 * @returns
		 *     last character
		 *
		 */
		char back() const
		{
			assert(size() > 0);
			return _get_entry()->data[size() - 1];
		}

		/**
		 * @brief
		 *     Compares two `symbol`s for equality.
		 *
		 * This is a constant-time operation, regardless of the `symbol`'s
		 * size.
		 *
		 * @param lhs
		 *     first `symbol` to compare
		 *
		 * @param rhs
		 *     second `symbol` to compare
		 *
		 * @returns
		 *     whether `lhs` and `rhs` are equal
		 *
		 */
		friend bool operator==(const symbol& lhs, const symbol& rhs) noexcept
		{
			assert(have_compatible_pool(lhs, rhs));
			return (lhs._entry == rhs._entry);
		}

		/**
		 * @brief
		 *     Compares two `symbol`s for inequality.
		 *
		 * This is a constant-time operation, regardless of the `symbol`'s
		 * size.
		 *
		 * @param lhs
		 *     first `symbol` to compare
		 *
		 * @param rhs
		 *     second `symbol` to compare
		 *
		 * @returns
		 *     whether `lhs` and `rhs` are not equal
		 *
		 */
		friend bool operator!=(const symbol& lhs, const symbol& rhs) noexcept
		{
			return !(lhs == rhs);
		}

	private:

		/**
		 * @brief
		 *     Returns the internal entry.
		 *
		 * In debug configuration this checks if the pool is still constructed.
		 *
		 * @returns
		 *     The internal entry.
		 */
		const symbol_entry * _get_entry() const noexcept
		{
			assert(is_pool_available());
			return _entry;
		}

		/** @brief The internal entry. */
		const symbol_entry * _entry{};

	};  // class symbol


	/**
	 * @brief
	 *     Compares the textual value of a `symbol` and a `std::string`.
	 *
	 * Unlike comparison between two `symbol`s -- which is a O(1) operation --
	 * this is an O(n) operation.
	 *
	 * @param lhs
	 *     `symbol` to compare
	 *
	 * @param rhs
	 *     `std::string` to compare
	 *
	 * @returns
	 *     whether `lhs` and `rhs` refer to the same sequence of characters
	 *
	 */
	inline bool operator==(const symbol& lhs, const std::string& rhs) noexcept
	{
		return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
	}


	/**
	 * @brief
	 *     Compares the textual value of a `std::string` and a `symbol`.
	 *
	 * Unlike comparison between two `symbol`s -- which is a O(1) operation --
	 * this is an O(n) operation.
	 *
	 * @param lhs
	 *     `std::string` to compare
	 *
	 * @param rhs
	 *     `symbol` to compare
	 *
	 * @returns
	 *     whether `lhs` and `rhs` refer to the same sequence of characters
	 *
	 */
	inline bool operator==(const std::string& lhs, const symbol& rhs) noexcept
	{
		return (rhs == lhs);
	}


	/**
	 * @brief
	 *     Compares the textual value of a `symbol` and a `std::string`.
	 *
	 * Unlike comparison between two `symbol`s -- which is a O(1) operation --
	 * this is an O(n) operation.
	 *
	 * @param lhs
	 *     `symbol` to compare
	 *
	 * @param rhs
	 *     `std::string` to compare
	 *
	 * @returns
	 *     whether `lhs` and `rhs` refer to different sequences of characters
	 *
	 */
	inline bool operator!=(const symbol& lhs, const std::string& rhs) noexcept
	{
		return !(lhs == rhs);
	}


	/**
	 * @brief
	 *     Compares the textual value of a `std::string` and a `symbol`.
	 *
	 * Unlike comparison between two `symbol`s -- which is a O(1) operation --
	 * this is an O(n) operation.
	 *
	 * @param lhs
	 *     `std::string` to compare
	 *
	 * @param rhs
	 *     `symbol` to compare
	 *
	 * @returns
	 *     whether `lhs` and `rhs` refer to different sequences of characters
	 *
	 */
	inline bool operator!=(const std::string& lhs, const symbol& rhs) noexcept
	{
		return !(lhs == rhs);
	}


	/**
	 * @brief
	 *     Compares the textual value of a `symbol` and a C-style string.
	 *
	 * Unlike comparison between two `symbol`s -- which is a O(1) operation --
	 * this is an O(n) operation.  If `rhs` is not a NUL terminated character
	 * sequence, the behavior is undefined.
	 *
	 * @param lhs
	 *     `symbol` to compare
	 *
	 * @param rhs
	 *     `char` array to compare
	 *
	 * @returns
	 *     whether `lhs` and `rhs` refer to the same sequence of characters
	 *
	 */
	inline bool operator==(const symbol& lhs, const char *const rhs)
	{
		assert(rhs != nullptr);
		// TODO: Can this be done with a standard library algorithm is just one pass?
		const auto len = std::strlen(rhs);
		return std::equal(lhs.begin(), lhs.end(), rhs, rhs + len);
	}


	/**
	 * @brief
	 *     Compares the textual value of a C-style string and a `symbol`.
	 *
	 * Unlike comparison between two `symbol`s -- which is a O(1) operation --
	 * this is an O(n) operation.  If `rhs` is not a NUL terminated character
	 * sequence, the behavior is undefined.
	 *
	 * @param lhs
	 *     `char` array to compare
	 *
	 * @param rhs
	 *     `symbol` to compare
	 *
	 * @returns
	 *     whether `lhs` and `rhs` refer to the same sequence of characters
	 *
	 */
	inline bool operator==(const char *const lhs, const symbol& rhs)
	{
		return (rhs == lhs);
	}


	/**
	 * @brief
	 *     Compares the textual value of a `symbol` and a C-style string.
	 *
	 * Unlike comparison between two `symbol`s -- which is a O(1) operation --
	 * this is an O(n) operation.  If `rhs` is not a NUL terminated character
	 * sequence, the behavior is undefined.
	 *
	 * @param lhs
	 *     `symbol` to compare
	 *
	 * @param rhs
	 *     `char` array to compare
	 *
	 * @returns
	 *     whether `lhs` and `rhs` refer to different sequences of characters
	 *
	 */
	inline bool operator!=(const symbol& lhs, const char *const rhs)
	{
		return !(lhs == rhs);
	}


	/**
	 * @brief
	 *     Compares the textual value of a C-style string and a `symbol`.
	 *
	 * Unlike comparison between two `symbol`s -- which is a O(1) operation --
	 * this is an O(n) operation.  If `rhs` is not a NUL terminated character
	 * sequence, the behavior is undefined.
	 *
	 * @param lhs
	 *     `char` array to compare
	 *
	 * @param rhs
	 *     `symbol` to compare
	 *
	 * @returns
	 *     whether `lhs` and `rhs` refer to different sequences of characters
	 *
	 */
	inline bool operator!=(const char *const lhs, const symbol& rhs)
	{
		return !(lhs == rhs);
	}


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


namespace std
{

	/**
	 * @brief
	 *     Specialization of `std::hash` for `minijava::symbol`s.
	 *
	 */
	template <>
	struct hash<minijava::symbol>
	{

		/**
		 * @brief
		 *     `return`s the precomputed hash value of a `symbol`.
		 *
		 * @param symbol
		 *     `symbol` to hash
		 *
		 * @returns
		 *     `symbol.hash()`
		 *
		 */
		std::size_t operator()(const minijava::symbol& symbol) const noexcept
		{
			return symbol.hash();
		}

	};

}  // namespace minijava
