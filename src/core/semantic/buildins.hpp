#pragma once

#include "parser/ast.hpp"
#include "symbol/symbol_pool.hpp"
#include "type_system.hpp"

#include <boost/optional.hpp>

namespace minijava
{
	namespace semantic
	{
		namespace buildins
		{
			boost::optional<t_type> resolve_binary_operator(ast::binary_operation_type op, const t_type& lhs, const t_type& rhs, const type_system& typesystem);
			boost::optional<t_type> resolve_unary_operator(ast::unary_operation_type op, const t_type& target);

			template<typename AllocT>
			t_type register_system(type_system& typesystem, symbol_pool<AllocT>& pool)
			{
				// build java.io.PrintStream with one method println
				auto* print_stream = typesystem.new_class(pool.normalize("java.io.PrintStream"), nullptr);
				auto* println = print_stream->new_method(type_system::t_void(), pool.normalize("println"), nullptr);
				println->add_parameter(type_system::t_int(), pool.normalize("i"), nullptr);

				// build system with out field
				auto* system = typesystem.new_class(pool.normalize("java.lang.System"), nullptr);
				system->new_field(print_stream->type(), pool.normalize("out"), nullptr);

				return system->type();
			}

			template<typename AllocT>
			t_type register_string(type_system& typesystem, symbol_pool<AllocT>& pool)
			{
				auto* system = typesystem.new_class(pool.normalize("java.lang.String"), nullptr);
				return system->type();
			}
		}
	}
}
