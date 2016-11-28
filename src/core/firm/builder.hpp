/**
 * @file builder.hpp
 *
 * @brief
 *     Creates the irg.
 *
 */

#pragma once

#include <parser/ast.hpp>
#include <semantic/semantic.hpp>
#include "libfirm/firm.h"

namespace minijava
{

	namespace /* anonymous */
	{


		struct collect_methods_visitor final : ast::visitor
		{

			collect_methods_visitor(ir_types ir_types)
			{
				_ir_types = irtypes;
			}

			void visit(const ast::instance_method& mthd) override
			{
				auto num_of_local_vars = 9999; // TODO
				auto method_type = new_type_method(mthd.parameters().size(), 1);
				// set params for method
				size_t pos = 0;
				for (auto param : mthd.parameters()) {
					auto param = mthd.parameters().at(i);
					auto param_type = _ir_types.get_var_type(param->var_type());
					set_method_param_type(method_type, pos++, param_type);
				}
				// set return type
				set_method_res_type(method_type, 0, _ir_types.get_var_type(mthd.return_type()));
				// create entity
				auto method_entity = new_entity(get_glob_type(), new_id_from_str(mthd.name().data()), method_type);
				// create graph
				auto fun_graph = new_ir_graph(method_type, num_of_local_vars);
				set_current_ir_graph(fun_graph);
				// todo - create graph nodes...
				irg_finalize_cons(fun_graph);
			}

		private:

			ir_types _ir_types;
		}

	}

	class ir_types {
	public:
		ir_type *get_var_type(ast::type type) {
			// todo - handle arrays..
			if (const auto primitive = boost::get<ast::primitive_type>(type)) {
				switch (primitive) {
					case ast::primitive_type::type_boolean:
						return _boolean_type;
					case ast::primitive_type ::type_int:
						return _int_type;
				}
			}
		}

		constexpr ir_type* type_boolean() const noexcept
		{
			return _boolean_type;
		}

		constexpr ir_type* type_int() const noexcept
		{
			return _int_type;
		}

		void create_entities()
		{

		}

		void create_basic_types()
		{
			_int_mode = mode_Is;
			_int_type = new_type_primitive(_int_mode);
			_boolean_mode = mode_Bs; // maybe use some other type here?
			_boolean_type = new_type_primitive(_boolean_mode);
		}

	private:

		ir_mode* _int_mode;
		ir_mode* _boolean_mode;

		ir_type* _int_type;
		ir_type* _boolean_type;
	};

	class builder
	{

	public:

		builder() {
			_ir_types = ir_types();
		}

		void ast2firm(const ast::program& ast, const semantic_info& semantic_info)
		{
			(void)ast;
			(void)semantic_info;

			ir_init();

			// create basic types
			_ir_types.create_basic_types();

			// collect methods signatures
			auto collect_methods_visitor = collect_methods_visitor(_ir_types);
			collect_methods_visitor.visit(ast);

			auto irg = new_ir_graph();

			ir_finish();
		}

	private:
		ir_types _ir_types;

	};
}