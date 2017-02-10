#pragma once


#include <string>
#include <stdexcept>

#include "position.hpp"


namespace minijava
{

	/**
	 * @brief
	 *     Exception used to report errors originated by the source.
	 *
	 */
	struct source_error: std::runtime_error
	{
		/**
		 * @brief
		 *     Creates a new exception object with a custom error message and
		 *     optional source location information.
		 *
		 * @param msg
		 *     custom error message
		 *
		 * @param pos
		 *     parser-defined error location
		 *
		 */
		source_error(const std::string& msg, minijava::position pos = {});

		/**
		 * @brief
		 *     `return`s the position of the parser-defined error location.
		 *
		 * If the error location is unknown, 0 is `return`ed.
		 *
		 * @returns
		 *     position
		 *
		 */
		minijava::position position() const noexcept;

	private:

		/** @brief Position of the parser-defined error token. */
		minijava::position _position{0, 0};


	};  // struct source_error
}
