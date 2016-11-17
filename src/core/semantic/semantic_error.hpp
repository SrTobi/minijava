#pragma once

#include <stdexcept>
#include <string>

namespace minijava
{
	/**
	* @brief
	*     Exception used to report semantic errors from within the parser.
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
		semantic_error(const std::string msg);

	};  // struct semantic_error
}
