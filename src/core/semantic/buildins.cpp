#include "buildins.hpp"

#include "exceptions.hpp"

namespace minijava
{
	namespace semantic
	{
		namespace buildins
		{
			boost::optional<t_type> resolve_binary_operator(ast::binary_operation_type op, const t_type& lhs, const t_type& rhs, const type_system& typesystem)
			{
				boost::optional<t_type> none;
				switch(op)
				{
				case ast::binary_operation_type::assign:

					return typesystem.is_assignable(rhs, lhs)
						? lhs
						: none;

				// logic operators
				case ast::binary_operation_type::logical_or:
				case ast::binary_operation_type::logical_and:
					return lhs == type_system::t_boolean() && rhs == type_system::t_boolean()
						? type_system::t_boolean()
						: none;

				// Equality
				case ast::binary_operation_type::equal:
				case ast::binary_operation_type::not_equal:
					return lhs == rhs || (lhs.is_reference() && rhs.is_reference())
						? type_system::t_boolean()
						: none;


				// integer comparisons
				case ast::binary_operation_type::less_than:
				case ast::binary_operation_type::less_equal:
				case ast::binary_operation_type::greater_than:
				case ast::binary_operation_type::greater_equal:
					return lhs == type_system::t_int() && rhs == type_system::t_int()
						? type_system::t_boolean()
						: none;

				// Arithmetic operations
				case ast::binary_operation_type::plus:
				case ast::binary_operation_type::minus:
				case ast::binary_operation_type::multiply:
				case ast::binary_operation_type::divide:
				case ast::binary_operation_type::modulo:
					return lhs == type_system::t_int() && rhs == type_system::t_int()
						? type_system::t_int()
						: none;
				default:
					MINIJAVA_NOT_REACHED();
				}
			}

			boost::optional<t_type> resolve_unary_operator(ast::unary_operation_type op, const t_type& target)
			{
				boost::optional<t_type> none;
				switch(op)
				{
				case ast::unary_operation_type::logical_not:
					return target == type_system::t_boolean()
						? type_system::t_boolean()
						: none;

				case ast::unary_operation_type::minus:
					return target == type_system::t_int()
						? type_system::t_int()
						: none;
				default:
					MINIJAVA_NOT_REACHED();
				}
			}
		}
	}
}
