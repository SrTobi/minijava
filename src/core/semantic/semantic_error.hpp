/**
 * @file semantic_error.hpp
 *
 * @brief
 *     Error handling for semantic analysis.
 *
 */

#pragma once

#include <stdexcept>
#include <string>


namespace minijava
{

	/**
	 * @brief
	 *     Exception used to report errors from the semantic analysis.
	 *
	 */
	struct semantic_error: std::runtime_error
	{

		/**
		 * @brief
		 *     Creates a new exception object with a generic error message and
		 *     no source location information.
		 *
		 */
		semantic_error();

		/**
		 * @brief
		 *     Creates a new exception object with a custom error message and
		 *     no source location information.
		 *
		 * @param msg
		 *     custom error message
		 *
		 */
		semantic_error(const std::string& msg);

	};  // struct semantic_error

}  // namespace minijava
