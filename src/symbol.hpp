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

namespace minijava
{
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
	class symbol final
	{
	public:
		/**
		 * @brief
		 *     Underlying entry for a symbol.
		 *
		 * Must be provided and owned by an external factory.
		 * If the symbols lifetime exceeds the factory the behaviour is undefined.
		 */
		struct symbol_entry
		{
			explicit symbol_entry(const char * cstr, std::size_t size, std::size_t hash)
				: cstr(cstr)
				, size(size)
				, hash(hash)
			{
				assert(hash == std::hash<std::string>()(std::string(cstr, size)));
			}


			/** Pointer to the actual string. Must be NUL-terminated */
			const char * cstr;

			/** Size of the symbol's string */
			const std::size_t size;

			/** The precomputed hash of the symbol */
			const std::size_t hash;
		};

		/**
		 * @brief
		 *     Creates a `string` from a symbol_entry.
		 *
		 * The symbol_entry must be created and owned by some kind of factory.
		 *
		 * @param s
		 *     pointer to a NUL-terminated character sequence
		 *
		 */
		constexpr explicit symbol(const symbol_entry * s) : _entry{s}
		{
			assert(s != nullptr);
		}

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
		constexpr const char * c_str() const noexcept
		{
			return _entry->cstr;
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
		friend constexpr bool operator==(const symbol& lhs, const symbol& rhs) noexcept
		{
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
		friend constexpr bool operator!=(const symbol& lhs, const symbol& rhs) noexcept
		{
			return !(lhs == rhs);
		}


	private:

		/** @brief The internal entry. */
		const symbol_entry * _entry;

	};  // class symbol

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
