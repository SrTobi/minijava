/**
 * @file builtins.hpp
 *
 * @brief
 *     Methods for adding the builtins to the IR graph.
 *
 */

#pragma once

#include "firm/type_builder.hpp"


namespace minijava
{

	namespace firm
	{

		/**
		 * @brief
		 *     Creates global variables and adds them to the IR graph.
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
		 *     semantic annotations for the current program
		 *
		 * @param types
		 *     Firm IR types and entities
		 *
		 */
		void create_globals(const semantic_info& semantic_info,
							ir_types& types);

	}  // namespace firm

}  // namespace minijava
