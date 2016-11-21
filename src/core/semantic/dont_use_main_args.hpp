/**
 * @file dont_use_main_args.hpp
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
	 *     Checks that nobody uses the String[] args parameter in the main method
	 *
	 * @param ast
	 *     program to check
	 *
	 * @throws semantic_error
	 *     if a usage is found
	 *
	 */
	void check_args_usage(const ast::program& ast);
}
