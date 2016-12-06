#include "firm/firm.hpp"

#include "libfirm/firm.h"

#include "firm/type_builder.hpp"
#include "firm/method_builder.hpp"

namespace minijava
{

	firm_ir::firm_ir()
	{
		ir_init();
		set_optimize(0);
		auto mode_p = new_reference_mode("P64", irma_twos_complement, 64, 64);
		set_modeP(mode_p);
	}

	firm_ir::~firm_ir()
	{
		ir_finish();
	}

	firm_ir create_firm_ir(const ast::program& ast,
	                       const semantic_info& semantic_info)
	{
		auto ir = firm_ir{};
		auto types = firm::create_types(ast, semantic_info);
		firm::create_methods(ast, semantic_info, types);
		return ir;
	}

	// since libfirm keeps the implicit state, we don't actually need the first
	// parameter; it's just to make sure that the user keeps the firm_ir object
	void dump_firm_ir(const firm_ir&, const std::string& directory)
	{
		if (!directory.empty()) {
			ir_set_dump_path(directory.c_str());
		}
		dump_all_ir_graphs(".vcg");
	}

	void lower();

	// since libfirm keeps the implicit state, we don't actually need the first
	// parameter; it's just to make sure that the user keeps the firm_ir object
	void emit_x64_assembly_firm(const firm_ir&, file_output& output_file)
	{
		// FIXME: implement lowering properly (?), see https://github.com/MatzeB/jFirm/blob/master/src/example/Lower.java
		// FIXME: do we need to call ir_lower_intrinsics here?
		// FIXME: the below is just a placeholder, I have no idea what I'm doing
		// lower_highlevel();
		lower();
		be_parse_arg("isa=amd64");
		be_main(output_file.handle(), output_file.filename().c_str());
	}

	void layoutFields();

	// move all class methods to global type
	void layoutClass(ir_type* clazz) {
		auto glob = get_glob_type();
		auto member_size = get_class_n_members(clazz);
		for (size_t j = member_size; j > 0; j--) {
			auto member = get_class_member(clazz, j - 1);
			if (is_method_entity(member)) {
				set_entity_owner(member, glob);
			}
		}

	}

	void layoutTypes() {
		auto num_types = get_irp_n_types();
		auto glob = get_glob_type();
		for (size_t i = 0; i < num_types; i++) {
			auto type = get_irp_type(i);
			if (!is_Class_type(type) || type == glob) {
				continue;
			}

			layoutClass(type);
			if (is_compound_type(type) ||
					is_Class_type(type)) {
				default_layout_compound_type(type);
			} else if (is_Primitive_type(type)) {
				// no layouting
			} else {
				set_type_state(type, layout_fixed);
			}
		}
	}

	void lower() {
		layoutTypes();
	}

}  // namespace minijava
