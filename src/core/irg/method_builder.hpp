/**
 * @file method_builder.hpp
 *
 * @brief
 *     Functions for building FIRM graphs for MiniJava methods.
 *
 */

#pragma once

#include "irg/global_firm_state.hpp"
#include "parser/ast.hpp"
#include "semantic/semantic.hpp"

struct ir_type;

namespace minijava
{

	namespace irg
	{
		struct ir_types;

		/**
		 * @brief
		 *     Performs the second Firm pass which creates IR methods.
		 *
		 * If the `libfirm` was not properly initialized before calling this
		 * function, the behavior is undefined.
		 *
		 * The behavior is also undefined if `seminfo` is not the result of a
		 * proper semantic analysis of the current program.
		 *
		 * Furthermore, the result is undefined if `types` is not the result of
		 * a successful first Firm pass of the current program.
		 *
		 * @param semantic_info
		 *     semantic annotation for the current program
		 *
		 * @param types
		 *     Firm IR types and entities
		 *
		 */
		void create_methods(const semantic_info& semantic_info,
		                    const ir_types& types);
	}  // namespace irg

}  // namespace minijava
