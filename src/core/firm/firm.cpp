#include "firm/firm.hpp"

#include <atomic>
#include <stdexcept>

#include "libfirm/firm.h"

#include "firm/type_builder.hpp"
#include "firm/method_builder.hpp"

namespace minijava
{

	firm_ir::firm_ir()
	{
		static std::atomic_flag initialized = ATOMIC_FLAG_INIT;
		if (initialized.test_and_set()) {
			throw std::logic_error{
					"libfirm was already initialized and is not re-entrant."
			};
		}
		ir_init();
		set_optimize(0);
		auto mode_p = new_reference_mode("P64", irma_twos_complement, 64, 64);
		set_modeP(mode_p);
	}

	firm_ir::firm_ir(firm_ir&& other)
	{
		if (!other._firm_owner) {
			throw std::logic_error{
					"Tried to move from an instance which was already moved."
			};
		}
		other._firm_owner = false;
	}

	firm_ir::~firm_ir()
	{
		if (_firm_owner) {
			ir_finish();
		}
	}

	firm_ir create_firm_ir(const ast::program& ast,
	                       const semantic_info& semantic_info)
	{
		auto ir = firm_ir{};
		auto types = firm::create_types(ast, semantic_info);
		//firm::create_methods(ast, semantic_info, types);
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

	// since libfirm keeps the implicit state, we don't actually need the first
	// parameter; it's just to make sure that the user keeps the firm_ir object
	void emit_x64_assembly_firm(const firm_ir&, file_output& output_file)
	{
		// FIXME: implement lowering properly (?), see https://github.com/MatzeB/jFirm/blob/master/src/example/Lower.java
		// FIXME: do we need to call ir_lower_intrinsics here?
		// FIXME: the below is just a placeholder, I have no idea what I'm doing
		// lower_highlevel();
		be_parse_arg("isa=amd64");
		be_main(output_file.handle(), output_file.filename().c_str());
	}

}  // namespace minijava
