#include "asm/data.hpp"

#include <cassert>
#include "firm.hpp"

#include "exceptions.hpp"
#include "global.hpp"


namespace minijava
{

	namespace backend
	{
		namespace {
			void write_data_segment_header(file_output& out)
			{
				out.write("\t.data\n");
			}

			constexpr std::size_t log2_floor(std::size_t v)
			{
				std::size_t result = 0;
				while (v >>= 1)
					++result;
				return result;
			}

			std::size_t determine_alignment(const firm::ir_entity* entity)
			{
				std::size_t alignment = firm::get_entity_alignment(entity);
				if (alignment == 0) {
					const firm::ir_type *type = firm::get_entity_type(entity);
					alignment = firm::get_type_alignment(type);
				}
				return alignment;
			}

			std::size_t determine_size(const firm::ir_entity* entity)
			{
				const auto type = firm::get_entity_type(entity);
				std::size_t size = firm::get_type_size(type);
				return size;
			}

			bool initializer_is_null(const firm::ir_initializer_t *initializer)
			{
				assert(initializer);
				switch (firm::get_initializer_kind(initializer)) {
				case firm::IR_INITIALIZER_NULL:
					return true;
				case firm::IR_INITIALIZER_TARVAL: {
					auto tv = firm::get_initializer_tarval_value(initializer);
					return firm::tarval_is_null(tv);
				}
				case firm::IR_INITIALIZER_CONST: {
					auto value = firm::get_initializer_const_value(initializer);
					if (!is_Const(value))
						return false;
					return firm::is_Const_null(value);
				}
				case firm::IR_INITIALIZER_COMPOUND: {
					for (size_t i = 0, n = firm::get_initializer_compound_n_entries(initializer); i < n; ++i) {
						const auto subinitializer = firm::get_initializer_compound_value(initializer, i);
						if (!initializer_is_null(subinitializer))
							return false;
					}
					return true;
				}
				};
				MINIJAVA_NOT_REACHED();
			}

			void write_initializer(const firm::ir_entity* entity, const firm::ir_initializer_t* initializer, file_output& out)
			{
				const auto type = firm::get_entity_type(entity);

				switch (firm::get_initializer_kind(initializer)) {
				case firm::IR_INITIALIZER_CONST: {
					const auto val = get_initializer_const_value(initializer);

					// we can only output address values
					assert(firm::get_irn_opcode(val) == firm::iro_Address);

					const auto member_name = firm::get_entity_ld_name(entity);
					const auto target_name = firm::get_entity_ld_name(firm::get_Address_entity(val));
					out.print("\t.quad %s\t\t# %s\n", target_name, member_name);
					return;
				}

				case firm::IR_INITIALIZER_COMPOUND: {
					assert(!firm::is_Array_type(type));
					assert(firm::is_compound_type(type));
					std::size_t offset = 0;
					std::size_t n_members = firm::get_compound_n_members(type);
					for (std::size_t i = 0; i < n_members; ++i) {
						const auto member = firm::get_compound_member(type, i);

						// we can not handle spaces in between member -> check that they are next to each other
						assert(offset == static_cast<std::size_t>(firm::get_entity_offset(member)));
						assert(0 == firm::get_entity_bitfield_size(member));
						assert(i < firm::get_initializer_compound_n_entries(initializer));
						const auto sub_initializer = firm::get_initializer_compound_value(initializer, i);

						write_initializer(member, sub_initializer, out);
						offset += firm::get_type_size(firm::get_entity_type(member));
					}

					return;
				}
				default:
					MINIJAVA_NOT_REACHED();
				}
			}

			void write_global_entity(const firm::ir_entity* entity, file_output& out)
			{
				// do not write stuff, that shall not be linked
				const auto linkage = firm::get_entity_linkage(entity);
				if (linkage & firm::IR_LINKAGE_NO_CODEGEN)
					return;

				// writen data shall have a definition
				if (!entity_has_definition(entity))
					return;

				// do not write methods in the data segment
				if(firm::is_method_entity(entity))
					return;

				const auto name = firm::get_entity_ld_name(entity);
				const auto size = determine_size(entity);
				const auto alignment = determine_alignment(entity);

				// do not write entities that do not have a initializer
				const auto initializer = firm::get_entity_initializer(entity);
				if(initializer_is_null(initializer))
				{
					out.print("\t# Global %s (no definition)\n", name);
					out.print("\t.local %s\n", name);
					out.print("\t.comm %s, %zu, %zu\n", name, size, alignment);
				} else {
					assert(alignment > 0);
					assert((std::size_t(1) << log2_floor(alignment)) == alignment);
					out.print("\t# Global %s\n", name);
					out.print("\t.p2align %zu\n", log2_floor(alignment));
					out.print("\t.type %s, @object\n", name);
					out.print("\t.size %s, %zu\n", name, size);
					out.print("%s:\n", name);
					write_initializer(entity, initializer, out);
				}
				out.print("\n");
			}
		}

		void write_data_segment(firm::ir_type*const glob, file_output& out)
		{
			assert(glob != nullptr);  (void) glob;
			if (MINIJAVA_WINDOWS_ASSEMBLY) {
				out.write("\t.section .rdata,\"dr\"\n");
				out.write("\t.p2align 3\n");
				out.write("\t.align 8\n");
				out.write("_mj_g1:\n");
				out.write("\t.quad _mj_g2\n");
				out.write("\t.quad _mj_g3\n");
				out.write("\t.comm _mj_g2,4,4\n");
				out.write("\t.comm _mj_g3,4,4\n");
				out.write("\t.p2align 3\n");
				out.write("\t.align 8\n");
				out.write("mj_System_v6:\n");
				out.write("\t.quad _mj_g1\n");
			} else {
				write_data_segment_header(out);
				std::size_t type_count = get_compound_n_members(glob);
				for (std::size_t i = 0; i < type_count; ++i) {
					const auto* entity = firm::get_compound_member(glob, i);
					write_global_entity(entity, out);
				}
			}
		}

	}  // namespace backend

}  // namespace minijava
