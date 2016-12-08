#include "firm/builtins.hpp"

namespace minijava
{

	namespace firm
	{

		void create_globals(const semantic_info& semantic_info,
							ir_types& types)
		{
			for (const auto& global : semantic_info.globals()) {
				const auto type = semantic_info.type_annotations().at(*global);
				const auto ir_type = types.typemap.at(type);
				const auto entity = new_entity(
						get_glob_type(),
						new_id_from_str(global->name().c_str()),
						ir_type
				);
				const auto initializer = create_initializer_tarval(
						new_tarval_from_long(0, get_type_mode(ir_type))
				);
				set_entity_initializer(entity, initializer);
				// TODO: implement mangle_global in mangle.h/cpp
				set_entity_ld_ident(
						entity, new_id_from_str(global->name().c_str())
				);
				types.fieldmap.put(*global, entity);
			}
		}

	}  // namespace firm

}  // namespace minijava
