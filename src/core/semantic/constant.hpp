/**
 * @file constant.hpp
 *
 * @brief
 *     Value conversion and basic constant folding on literals.
 *
 */

#pragma once

#include <cstdint>
#include <functional>
#include <unordered_map>

#include "parser/ast.hpp"


namespace minijava
{

	/**
	 * @brief
	 *     Signed integer type used for storing constants in the AST.
	 *
	 * The type has the following properties:
	 *
	 *  - It can represent any value of an expression in a valid program.
	 *  - It can represent the value 2<sup>31</sup> which can only occur as the
	 *    literal operand of a unary minus in a valid program.
	 *  - It has (in C++) a rank no less than `int`.
	 *  - It is at least 64 bits wide.
	 *
	 */
	using ast_int_type = decltype(std::int_least64_t{} + 0);

	/**
	 * @brief
	 *     Callback type that can be used to be notified about expressions with
	 *     undefined result during constant folding on the AST.
	 *
	 * The callback should accept a reference to the expression node that
	 * triggered the problem.  It can `return` normally or `throw` an
	 * exception, whatever it likes best.
	 *
	 */
	using ast_problem_handler_type = std::function<void(const ast::node&)>;

	/**
	 * @brief
	 *     Extracts integer and boolean constants from an AST and does basic
	 *     constant folding on expressions.
	 *
	 * The boolean constants `false` and `true` are represented as the integers
	 * 0 and 1.  The extracted map will contain all constants folded up to the
	 * highest possible node within expressions.  The `return`ed map can
	 * contain the value 2<sup>31</sup> which is a valid literal if it is the
	 * operand of a unary minus.  But since constants are folded up, if you
	 * always take the most-folded constant in the tree, you'll never see this
	 * value and can safely cast the constants to a 32 bit signed integer in
	 * two's compement representation.
	 *
	 * If during constant folding, an operation is encountered that has an
	 * undefined result (such as overflow or division by zero), the callback
	 * function `handler` is called with the operation node that triggered the
	 * problem as argument and the result of the operation is treated as
	 * unknown.  If the handler `return`s normally, constant extraction will
	 * proceed.
	 *
	 * If the AST is not a correctly typed program, the behavior is undefined.
	 * Therefore, this function shall only be called after type checking
	 * completed successfully.  Furthermore, the behevior is undefined if the
	 * node IDs in the AST are ambigous.
	 *
	 * @param ast
	 *     AST to traverse
	 *
	 * @param handler
	 *     callback function to invoke in case of an undefined result
	 *
	 * @returns
	 *     mapping from AST node IDs to constants
	 *
	 * @throws semantic_error
	 *     if the AST contains unrepresentable literals
	 *
	 */
	std::unordered_map<std::size_t, ast_int_type>
	extract_constants(const ast::node& ast, const ast_problem_handler_type& handler);

	/**
	 * @brief
	 *     Convenience function that calls `extract_constants` with a problem
	 *     handler that does nothing.
	 *
	 * @param ast
	 *     AST to traverse
	 *
	 * @returns
	 *     mapping from AST node IDs to constants
	 *
	 * @throws semantic_error
	 *     if the AST contains unrepresentable literals
	 *
	 */
	std::unordered_map<std::size_t, ast_int_type>
	extract_constants(const ast::node& ast);

}  // namespace minijava
