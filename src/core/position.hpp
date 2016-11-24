#pragma once

#include <cstddef>
#include <iosfwd>

namespace minijava
{
	struct position {

	public:
		position() = default;

		position(const std::size_t line, const std::size_t column) : _line{line}, _column{column}
		{ }

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
	};

	/**
	 * @brief
	 *     Compares two `position`s for equality.
	 *
	 * @param lhs
	 *     first `position` to compare
	 *
	 * @param rhs
	 *     second `position` to compare
	 *
	 * @returns
	 *     whether `lhs` and `rhs` are equal
	 *
	 */
	constexpr bool operator==(const minijava::position& lhs, const minijava::position& rhs) noexcept
	{
		return lhs.line() == rhs.line() && lhs.column() == rhs.column();
	}

	/**
	 * @brief
	 *     Compares two `position`s for unequality.
	 *
	 * @param lhs
	 *     first `position` to compare
	 *
	 * @param rhs
	 *     second `position` to compare
	 *
	 * @returns
	 *     whether `lhs` and `rhs` are not equal
	 *
	 */
	constexpr bool operator!=(const minijava::position& lhs, const minijava::position& rhs) noexcept
	{
		return !(lhs == rhs);
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
	 * @param tok
	 *     token to insert
	 *
	 * @returns
	 *     reference to `os`
	 *
	 */
	std::ostream& operator<<(std::ostream& os, const minijava::position& pos);

	/**
	 * @brief
	 *     Compares if `lhs` is lower `rhs`.
	 *
	 * @param lhs
	 *     first `position` to compare
	 *
	 * @param rhs
	 *     second `position` to compare
	 *
	 * @returns
	 *     whether `lhs` is lower than `rhs`
	 *
	 */
	constexpr bool operator<(const minijava::position& lhs, const minijava::position& rhs) noexcept
	{
		return lhs.line() < rhs.line() || (lhs.line() == rhs.line() && lhs.column() < rhs.column());
	}

	/**
	 * @brief
	 *     Compares if `lhs` is lower or equal `rhs`.
	 *
	 * @param lhs
	 *     first `position` to compare
	 *
	 * @param rhs
	 *     second `position` to compare
	 *
	 * @returns
	 *     whether `lhs` is lower or equal than `rhs`
	 *
	 */
	constexpr bool operator<=(const minijava::position& lhs, const minijava::position& rhs) noexcept
	{
		return lhs == rhs || lhs < rhs;
	}

	/**
	 * @brief
	 *     Compares if `lhs` is greater `rhs`.
	 *
	 * @param lhs
	 *     first `position` to compare
	 *
	 * @param rhs
	 *     second `position` to compare
	 *
	 * @returns
	 *     whether `lhs` is greater than `rhs`
	 *
	 */
	constexpr bool operator>(const minijava::position& lhs, const minijava::position& rhs) noexcept
	{
		return lhs != rhs && !(lhs < rhs);
	}

	/**
	 * @brief
	 *     Compares if `lhs` is greater or equal `rhs`.
	 *
	 * @param lhs
	 *     first `position` to compare
	 *
	 * @param rhs
	 *     second `position` to compare
	 *
	 * @returns
	 *     whether `lhs` is greater or equal than `rhs`
	 *
	 */
	constexpr bool operator>=(const minijava::position& lhs, const minijava::position& rhs) noexcept
	{
		return !(lhs < rhs);
	}
}
