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
	 *     A non-owning read-only wrapper around a canonical pointer to a
	 *     NUL-terminated character sequence.
	 *
	 * Instances of this type have exactly one non-`static` data member which
	 * is a pointer to a NUL-terminated character sequence.  The pointed-to
	 * memory is not owned and must remain valid for the entire life-time of
	 * the `string` object.  Equality on `string` objects is defined in terms
	 * of pointer identity.  This means that in order to be useful, no two
	 * `string` objects should be created with different pointers pointing to
	 * buffers with the same data.  In other words, all users must agree that
	 * for a given text, only a single pointer is used.  We call such pointer a
	 * *canonical* pointer.  How a canonical pointer is obtained is not the
	 * business of this `class`.  In order to avoid creating a `string` from a
	 * non-canonical pointer by accident, there is no public constructor and
	 * the provided factory function has a long and ugly name.  There should
	 * probably be a central authority -- called a *pool* -- that
	 * &ldquo;canonizes&rdquo; strings and manages the life-times of the
	 * buffers referred to by the canonical pointers.  Mixing `string`s from
	 * different pools is likely not going to be very useful.
	 *
	 */
	class symbol final
	{
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

		/**
		 * @brief
		 *     Creates a `string` from a given canonical pointer.
		 *
		 * The behavior is undefined unless `s` points to a NUL-terminated
		 * character sequence.
		 *
		 * The created object will compare equal to other `string` objects if
		 * and only if they were created from the same pointer, regardless of
		 * the contents of the pointed-to memory.
		 *
		 * This function has a long and ugly name to discourage people calling
		 * it by accident.
		 *
		 * @param s
		 *     pointer to a NUL-terminated character sequence
		 *
		 */
		constexpr static symbol create_from_canonical_pointer(const char * s)
		{
			return symbol{s};
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
