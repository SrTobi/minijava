#pragma once

#include <memory>

#include "parser/ast.hpp"

namespace minijava
{
	/**
	 * @brief
	 *     Exception used to report syntacitcal errors from within the parser.
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


    void analyse_program(const ast::program& prog);
}
