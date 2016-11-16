#include "std_definitions.hpp"

#include "exceptions.hpp"

namespace minijava {

	boost::optional<semantic_type> resolve_binary_operator(ast::binary_operation_type op, const semantic_type& lhs, const semantic_type& rhs)
	{
		switch(op)
		{
		case ast::binary_operation_type::assign:

			return lhs == rhs || (lhs.is_reference() && rhs.kind() == semantic_type_kind::kind_null_reference)
				? lhs
				: boost::optional<semantic_type>{};

		// logic operators
		case ast::binary_operation_type::logical_or:
		case ast::binary_operation_type::logical_and:
			return lhs == semantic_type_kind::kind_boolean && rhs == semantic_type_kind::kind_boolean
				? semantic_type{semantic_type_kind::kind_boolean}
				: boost::optional<semantic_type>{};

		// Equality
		case ast::binary_operation_type::equal:
		case ast::binary_operation_type::not_equal:
			return lhs == rhs || (lhs.is_reference() && rhs.is_reference())
				? semantic_type{semantic_type_kind::kind_boolean}
				: boost::optional<semantic_type>{};


		// integer comparisons
		case ast::binary_operation_type::less_than:
		case ast::binary_operation_type::less_equal:
		case ast::binary_operation_type::greater_than:
		case ast::binary_operation_type::greater_equal:
			return lhs == semantic_type_kind::kind_int && rhs == semantic_type_kind::kind_int
				? semantic_type{semantic_type_kind::kind_boolean}
				: boost::optional<semantic_type>{};

		// Arithmetic operations
		case ast::binary_operation_type::plus:
		case ast::binary_operation_type::minus:
		case ast::binary_operation_type::multiply:
		case ast::binary_operation_type::divide:
		case ast::binary_operation_type::modulo:
			return lhs == semantic_type_kind::kind_int && rhs == semantic_type_kind::kind_int
				? semantic_type{semantic_type_kind::kind_int}
				: boost::optional<semantic_type>{};
		default:
			MINIJAVA_NOT_REACHED();
		}
	}

	boost::optional<semantic_type> resolve_unary_operator(ast::unary_operation_type op, const semantic_type& target)
	{
		switch(op)
		{
		case ast::unary_operation_type::logical_not:
			return target == semantic_type_kind::kind_boolean
				? semantic_type{semantic_type_kind::kind_boolean}
				: boost::optional<semantic_type>{};

		case ast::unary_operation_type::minus:
			return target == semantic_type_kind::kind_int
				? semantic_type{semantic_type_kind::kind_int}
				: boost::optional<semantic_type>{};
		default:
			MINIJAVA_NOT_REACHED();
		}
	}
}
