#include "irg/globals.hpp"

#include "exceptions.hpp"
#include "irg/mangle.hpp"


namespace minijava
{

	namespace irg
	{

		namespace /* anonymous */
		{

			void check_reference(const ast::var_decl& vardecl)
			{
				if (vardecl.var_type().rank() != 0) {
					MINIJAVA_THROW_ICE_MSG(
						internal_compiler_error,
						"Cannot handle global object of array type"
					);
				}
				if (boost::get<symbol>(&vardecl.var_type().name()) == nullptr) {
					MINIJAVA_THROW_ICE_MSG(
						internal_compiler_error,
						"Cannot handle global object of non-class type"
					);
				}
			}

			firm::ir_initializer_t* init_global_pointer(
					const ast::var_decl& vardecl,
					const semantic_info& seminfo,
					const ir_types& irtypes,
					int& tally
				)
			{
				check_reference(vardecl);
				const auto semtyp = seminfo.type_annotations().at(vardecl);
				const auto irtype = irtypes.classmap.at(*semtyp.info.declaration());
				const auto entity = firm::new_entity(
					firm::get_glob_type(),
					firm::new_id_from_str(vardecl.name().c_str()),
					irtype
				);
				firm::set_entity_ld_ident(entity, firm::new_id_fmt(".mj_g%d", ++tally));
				firm::set_entity_visibility(entity, firm::ir_visibility_local);
				const auto& fields = semtyp.info.declaration()->fields();
				const auto initializer = firm::create_initializer_compound(fields.size());
				std::size_t idx = 0;
				for (auto&& field : fields) {
					const auto init = init_global_pointer(*field, seminfo, irtypes, tally);
					firm::set_initializer_compound_value(initializer, idx++, init);
				}
				firm::set_entity_initializer(entity, initializer);
				return firm::create_initializer_const(firm::new_Address(entity));
			}

		}  // namespace /* anonymous */


		void create_globals(const semantic_info& seminfo, ir_types& irtypes)
		{
			firm::set_current_ir_graph(firm::get_const_code_irg());
			auto tally = 0;
			for (auto&& glob : seminfo.globals()) {
				const auto initializer = init_global_pointer(*glob, seminfo, irtypes, tally);
				const auto semtype = seminfo.type_annotations().at(*glob);
				const auto pointer = irtypes.typemap.at(semtype);
				const auto entity = firm::new_entity(
					firm::get_glob_type(),
					firm::new_id_from_str(glob->name().c_str()),
					pointer
				);
				firm::set_entity_ld_ident(entity, mangle(*glob));
				firm::set_entity_visibility(entity, firm::ir_visibility_local);
				firm::set_entity_initializer(entity, initializer);
				irtypes.fieldmap.put(*glob, entity);
			}
			firm::set_current_ir_graph(nullptr);
		}

	}  // namespace irg

}  // namespace minijava
