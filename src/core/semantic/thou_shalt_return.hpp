/**
 * @file thou_shalt_return.hpp
 *
 * @brief
 *     Static analysis routines to ensure that non-`void` functions `return` on
 *     each possible path.
 *
 * The rule enforced by this component is defined as follows.  The terminology
 * is loosely following §&nbsp;14.21 of the JLS.
 *
 * If the control flow can reach the end of a function with a `return` type
 * other than `void`, the program is ill-formed.
 *
 * Control flow can reach the end of a function if and only if the function
 * body (which is a block) can complete normally.
 *
 * A block can complete normally if and only if
 *
 * - it is empty or
 * - the last block statement of the block is reachable can complete normally
 *
 * A block statement is reachable if and only if:
 *
 * - it is the first block statement of a block or
 * - its predecessor is reachable and can complete normally
 *
 * A `return` statement cannot complete normally.
 *
 * An `if` statement can complete normally if and only if:
 *
 * - it has no `else` statement or
 * - either the then or the `else` statement can complete normally
 *
 * All other block statements can complete normally.
 *
 */

#pragma once

#include "parser/ast.hpp"


namespace minijava
{

	/**
	 * @brief
	 *     Checks that all non-`void` methods in the program `return` a value
	 *     on all possible paths.
	 *
	 * If the program is not well-typed, the behavior is undefined.  Therefore,
	 * this function shall only be called after type-checking was successful.
	 *
	 * @param ast
	 *     program to check
	 *
	 * @throws semantic_error
	 *     if there are paths that might `return` without a value
	 *
	 */
	void check_return_paths(const ast::program& ast);

}  // namespace minijava
