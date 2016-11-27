/**
 * @file position.hpp
 *
 * @brief
 *     References to source code locations.
 *
 * Line and column numbers start at 1.  The value 0 is reserved to represent an
 * unknown index.
 *
 * The relational operators are defined in their natural sense.  Their
 * documentation uses the phrase &ldquo;A comes before B&rdquo; to mean that if
 * the source code were linearized (as if by storing the text file in a
 * character array), then location A would come before location B.  This
 * obviously assumes that code is read in the order in which the bytes appear
 * in memory, which is okay, because MiniJava only allows ASCII anyway.
 *
 */

#pragma once

#include <cstddef>
#include <iosfwd>


namespace minijava
{

	/**
	 * @brief
	 *     Source code location.
	 *
	 */
	class position
	{
	public:

		/**
		 * @brief
		 *     Creates an unknow `position`.
		 *
		 */
		constexpr position() noexcept = default;

		/**
		 * @brief
		 *     Creates a `position` from the given line and column number.
		 *
		 * @param line
		 *     line number
		 *
		 * @param column
		 *     column number
		 *
		 */
		constexpr position(const std::size_t line, const std::size_t column) noexcept
			: _line{line}, _column{column}
		{
		}

		/**
		 * @brief
		 *     `return`s the line number.
		 *
		 * If the value is 0, the line number is unknown.
		 *
		 * @returns
		 *     line number
		 *
		 */
		constexpr std::size_t line() const noexcept
		{
			return _line;
		}

		/**
		 * @brief
		 *     `return`s the column number.
		 *
		 * If the value is 0, the column number is unknown.
		 *
		 * @returns
		 *     column number
		 *
		 */
		constexpr std::size_t column() const noexcept
		{
			return _column;
		}

	private:

		/** @brief Line number. */
		std::size_t _line{};

		/** @brief Column number. */
		std::size_t _column{};

	};  // class position


	/**
	 * @brief
	 *     Tests whether the position `lhs` is the same as `rhs`.
	 *
	 * @param lhs
	 *     first `position` to compare
	 *
	 * @param rhs
	 *     second `position` to compare
	 *
	 * @returns
	 *     whether `lhs` and `rhs` are the same
	 *
	 */
	constexpr bool operator==(const position lhs, const position rhs) noexcept
	{
		return lhs.line() == rhs.line() && lhs.column() == rhs.column();
	}

	/**
	 * @brief
	 *     Tests whether the position `lhs` is different from `rhs`.
	 *
	 * @param lhs
	 *     first `position` to compare
	 *
	 * @param rhs
	 *     second `position` to compare
	 *
	 * @returns
	 *     whether `lhs` and `rhs` are different
	 *
	 */
	constexpr bool operator!=(const position lhs, const position rhs) noexcept
	{
		return !(lhs == rhs);
	}

	/**
	 * @brief
	 *     Tests whether the position `lhs` comes before `rhs`.
	 *
	 * @param lhs
	 *     first `position` to compare
	 *
	 * @param rhs
	 *     second `position` to compare
	 *
	 * @returns
	 *     whether `lhs` comes before `rhs`
	 *
	 */
	constexpr bool operator<(const position lhs, const position rhs) noexcept
	{
		return (lhs.line() < rhs.line())
			|| (lhs.line() == rhs.line() && lhs.column() < rhs.column());
	}

	/**
	 * @brief
	 *     Tests whether the position `lhs` comes after `rhs`.
	 *
	 * @param lhs
	 *     first `position` to compare
	 *
	 * @param rhs
	 *     second `position` to compare
	 *
	 * @returns
	 *     whether `lhs` comes after `rhs`
	 *
	 */
	constexpr bool operator>(const position lhs, const position rhs) noexcept
	{
		return !(lhs < rhs) && (lhs != rhs);
	}

	/**
	 * @brief
	 *     Tests whether the position `lhs` comes not after `rhs`.
	 *
	 * @param lhs
	 *     first `position` to compare
	 *
	 * @param rhs
	 *     second `position` to compare
	 *
	 * @returns
	 *     whether `lhs` doesn't come after `rhs`
	 *
	 */
	constexpr bool operator<=(const position lhs, const position rhs) noexcept
	{
		return !(lhs > rhs);
	}

	/**
	 * @brief
	 *     Tests whether the position `lhs` comes not before `rhs`.
	 *
	 * @param lhs
	 *     first `position` to compare
	 *
	 * @param rhs
	 *     second `position` to compare
	 *
	 * @returns
	 *     whether `lhs` doesn't come before `rhs`
	 *
	 */
	constexpr bool operator>=(const position lhs, const position rhs) noexcept
	{
		return !(lhs < rhs);
	}

	/**
	 * @brief
	 *     Inserts a textual representation of a position into an output stream.
	 *
	 * The textual representation consists of the textual representation of the
	 * `position`'s line and column, written for humans.
	 * i.e. "line: 10 column: 20"
	 *
	 * @param os
	 *     stream to write to
	 *
	 * @param pos
	 *     position to insert
	 *
	 * @returns
	 *     reference to `os`
	 *
	 */
	std::ostream& operator<<(std::ostream& os, const position pos);

}  // namespace minijava
