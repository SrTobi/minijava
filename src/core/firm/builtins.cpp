#include "firm/builtins.hpp"

#include <cassert>
#include <string>

#include "firm/mangle.hpp"

namespace minijava
{

	namespace firm
	{

		void create_globals(const semantic_info& semantic_info,
							ir_types& types)
		{
			assert(semantic_info.globals().size() == 1);
			const auto& system = semantic_info.globals().at(0);
			assert("System" == system->name());
			const auto type = semantic_info.type_annotations().at(*system);
			assert(type.info.is_reference());
			const auto type_decl = type.info.declaration();
			assert("java.lang.System" == type_decl->name());
			const auto ir_record_type = types.classmap.at(*type_decl);
			const auto ir_ptr_type = types.typemap.at(type);
			assert(type_decl->fields().size() == 1);
			const auto& out_field = type_decl->fields().at(0);
			const auto out_type = semantic_info.type_annotations().at(*out_field);
			assert(type.info.is_reference());
			const auto out_type_decl = out_type.info.declaration();
			assert("java.io.PrintStream" == out_type_decl->name());
			const auto out_ir_record_type = types.classmap.at(*out_type_decl);
			// create global entities
			const auto out_entity = new_entity(
					get_glob_type(),
					new_id_from_str("mj_System_out"),
					out_ir_record_type
			);
			const auto system_entity = new_entity(
					get_glob_type(),
					new_id_from_str("mj_System"),
					ir_record_type
			);
			const auto system_ptr_entity = new_entity(
					get_glob_type(),
					new_id_from_str(system->name().c_str()),
					ir_ptr_type
			);
			set_entity_ld_ident(system_ptr_entity, mangle(*system));
			types.fieldmap.put(*system, system_ptr_entity);
			// initialize globals
			set_current_ir_graph(get_const_code_irg());
			const auto out_initializer = create_initializer_compound(0);
			set_entity_initializer(out_entity, out_initializer);
			const auto system_initializer = create_initializer_compound(1);
			set_initializer_compound_value(system_initializer, 0,
					create_initializer_const(new_Address(out_entity)));
			set_entity_initializer(system_entity, system_initializer);
			const auto system_ptr_initializer = create_initializer_const(
					new_Address(system_entity)
			);
			set_entity_initializer(system_ptr_entity, system_ptr_initializer);
			set_current_ir_graph(nullptr);
		}

	}  // namespace firm

}  // namespace minijava
