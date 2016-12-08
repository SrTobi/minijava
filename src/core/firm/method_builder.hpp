/**
 * @file method_builder.hpp
 *
 * @brief
 *     Functions for building FIRM graphs for MiniJava methods.
 *
 */

#pragma once

#include "firm/singleton.hpp"
#include "parser/ast.hpp"
#include "semantic/semantic.hpp"

struct ir_type;

namespace minijava
{

	namespace firm
	{
		class ir_types;

		/**
		 * @brief
		 *     Performs the second Firm pass which creates IR methods.
		 *
		 * If the `libfirm` was not properly initialized before calling this
		 * function, the behavior is undefined.
		 *
		 * The behavior is also undefined if `ast` is not a semantically
		 * correct program or if `seminfo` is not the result of a proper
		 * semantic analysis of `ast`.
		 *
		 * Furthermore, the result is undefined if `types` is not the result of
		 * a successful first Firm pass of the given program.
		 *
		 * @param ast
		 *     AST of the program
		 *
		 * @param semantic_info
		 *     semantic annotation for the given AST
		 *
		 * @param types
		 *     Firm IR types and entities
		 *
		 * @param arguments
		 *     Argument list map, in which the argument arrays for each
		 *     IR Call node are retained.
		 *
		 */
		void create_methods(const ast::program& ast,
		                    const semantic_info& semantic_info,
		                    const ir_types& types,
		                    firm_global_state::argument_list_map& arguments);
	}  // namespace firm

}  // namespace minijava
