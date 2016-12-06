/**
 * @file method_builder.hpp
 *
 * @brief
 *     Functions for building FIRM graphs for MiniJava methods.
 *
 */

#pragma once

#include "parser/ast.hpp"
#include "semantic/semantic.hpp"

struct ir_type;

namespace minijava
{


	namespace firm
	{
		class ir_types;

		void create_methods(const ast::program& ast,
		                    const semantic_info& semantic_info,
		                    const ir_types& types);
	}  // namespace firm

}  // namespace minijava
