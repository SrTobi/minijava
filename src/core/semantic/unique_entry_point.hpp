/**
 * @file unique_entry_point.hpp
 *
 * @brief
 *     Static analysis routines to ensure there's only one entry point (main
 *     method) in the program
 *
 */

#pragma once

#include "parser/ast.hpp"


namespace minijava
{
	/**
	 * @brief
	 *     Checks that there's only one entry point (main method) in the
	 *     program.
	 *
	 * @param ast
	 *     program to check
	 *
	 * @throws semantic_error
	 *     if multiple entry points are found
	 *
	 */
	void check_unique_entry_point(const ast::program& ast);
}
