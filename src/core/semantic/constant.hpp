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

#include "parser/ast.hpp"
#include "semantic/attribute.hpp"


namespace minijava
{

	namespace sem
	{

		/** @brief Type mapping expression nodes to their constant value. */
		using const_attributes = ast_attributes<std::int32_t, ast_node_filter<ast::expression>>;

		/**
		 * @brief
		 *     Callback type that can be used to be notified about expressions
		 *     with undefined result during constant folding on the AST.
		 *
		 * The callback should accept a reference to the expression node that
		 * triggered the problem.  It can `return` normally or `throw` an
		 * exception, whatever it likes best.
		 *
		 */
		using ast_problem_handler_type = std::function<void(const ast::node&)>;

		/**
		 * @brief
		 *     Extracts integer and boolean constants from an AST and does
		 *     basic constant folding on expressions.
		 *
		 * The boolean constants `false` and `true` are represented as the
		 * integers 0 and 1.  The extracted map will contain all constants
		 * folded up to the highest possible node within expressions.  Constant
		 * propagation beyond expression boundaries is not performed.
		 *
		 * If during constant folding, an operation is encountered that has an
		 * undefined result (such as overflow or division by zero), the
		 * callback function `handler` is called with the operation node that
		 * triggered the problem as argument and the result of the operation is
		 * treated as unknown.  If the handler `return`s normally, constant
		 * extraction will proceed.
		 *
		 * If the AST is not a correctly typed program, the behavior is
		 * undefined.  Therefore, this function shall only be called after type
		 * checking completed successfully.  Furthermore, the behevior is
		 * undefined if the node IDs in the AST are ambigous.
		 *
		 * @param ast
		 *     AST to traverse
		 *
		 * @param handler
		 *     callback function to invoke in case of a constant expression
		 *     with undefined result
		 *
		 * @returns
		 *     mapping from expression nodes to constants
		 *
		 * @throws semantic_error
		 *     if the AST contains unrepresentable literals
		 *
		 */
		const_attributes extract_constants(const ast::node& ast, const ast_problem_handler_type& handler);

		/**
		 * @brief
		 *     Convenience function that calls `extract_constants` with a
		 *     problem handler that does nothing.
		 *
		 * @param ast
		 *     AST to traverse
		 *
		 * @returns
		 *     mapping from expression nodes to constants
		 *
		 * @throws semantic_error
		 *     if the AST contains unrepresentable literals
		 *
		 */
		const_attributes extract_constants(const ast::node& ast);

	}  // namespace sem

}  // namespace minijava
