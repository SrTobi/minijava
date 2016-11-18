#pragma once

#include "parser/ast.hpp"
#include "symbol/symbol_pool.hpp"
#include "type_system.hpp"

#include <boost/optional.hpp>

namespace minijava
{
	namespace semantic
	{
		boost::optional<t_type> resolve_binary_operator(ast::binary_operation_type op, const t_type& lhs, const t_type& rhs, const type_system& typesystem);
		boost::optional<t_type> resolve_unary_operator(ast::unary_operation_type op, const t_type& target);
	}
}
