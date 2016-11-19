#pragma once

#include <cstddef>
#include <iosfwd>

namespace minijava
{
	struct position {

	public:
		position(const std::size_t line, const std::size_t column) : _line(line), _column(column)
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
		inline std::size_t line() const noexcept
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
		inline std::size_t column() const noexcept
		{
			return _column;
		}

		bool operator==(const position& lhs, const position& rhs) noexcept
		{
			return lhs.line() == rhs.line() && lhs.column() == rhs.column();
		}

		bool operator!=(const position& lhs, const position& rhs) noexcept {
			return !(lhs == rhs);
		}

		bool operator==(const position& lhs, const int rhs) noexcept {
			return rhs == 0 && lhs.line() == 0 && lhs.column() == 0;
		}

		bool operator!=(const position& lhs, const int rhs) noexcept {
			return !(lhs == rhs);
		}

		std::ostream& operator<<(std::ostream& os, const position& pos) {
			os << "line: " << pos.line()
			   << " column: " << pos.column();
		}

	private:

		/** @brief Line number. */
		std::size_t _line{};

		/** @brief Column number. */
		std::size_t _column{};
	};
}