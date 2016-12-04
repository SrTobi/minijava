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

	class ir_types; // FIXME: move to namespace

	namespace firm
	{

		void create_firm_method(const semantic_info& sem_info,
		                        ir_types& firm_types,
		                        const ast::method& method);

	}  // namespace firm

}  // namespace minijava
