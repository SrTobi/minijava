#pragma once


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

	private:

		/** @brief Line number. */
		std::size_t _line{};

		/** @brief Column number. */
		std::size_t _column{};
	};
}