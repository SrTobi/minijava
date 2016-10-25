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
		template<typename Inner, typename Outer>
		friend class symbol_pool;
	private:

		/**
		 * @brief
		 *     Creates a `string` from a given canonical pointer.
		 *
		 * @param s
		 *     pointer to a NUL-terminated character sequence
		 *
		 */
		constexpr explicit symbol(const char * s) : _ptr{s}
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
			return _ptr;
		}

	private:

		/** @brief The wrapped raw pointer. */
		const char * _ptr{};

	};  // class symbol


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
	 *     whether `lhs` and `rhs` wrap the same canonical pointer
	 *
	 */
	constexpr bool operator==(const symbol& lhs, const symbol& rhs) noexcept
	{
		return (lhs.c_str() == rhs.c_str());
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
	constexpr bool operator!=(const symbol& lhs, const symbol& rhs) noexcept
	{
		return !(lhs == rhs);
	}

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
