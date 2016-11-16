#pragma once

#include "parser/ast.hpp"
#include "semantic_type.hpp"

#include <boost/optional.hpp>

namespace minijava {
	boost::optional<semantic_type> resolve_binary_operator(ast::binary_operation_type op, const semantic_type& lhs, const semantic_type& rhs);
	boost::optional<semantic_type> resolve_unary_operator(ast::unary_operation_type op, const semantic_type& target);
}
