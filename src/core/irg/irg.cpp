#include "irg/irg.hpp"

#include "libfirm/firm.h"

#include "irg/builtins.hpp"
#include "irg/lowering.hpp"
#include "irg/method_builder.hpp"
#include "irg/type_builder.hpp"

namespace minijava
{

	firm_global_state create_firm_ir(const ast::program& ast,
	                                 const semantic_info& semantic_info)
	{
		auto ir = firm_global_state{};
		auto types = irg::create_types(ast, semantic_info);
		irg::create_globals(semantic_info, types);
		irg::create_methods(semantic_info, types);
		return ir;
	}

	// since libfirm keeps the implicit state, we don't actually need the first
	// parameter; it's just to make sure that the user keeps the firm_global_state object
	void dump_firm_ir(firm_global_state& firm, const std::string& directory)
	{
		assert(firm);
		(void) firm; // suppress warning in release mode
		if (!directory.empty()) {
			ir_set_dump_path(directory.c_str());
		}
		dump_all_ir_graphs("");
	}


	// since libfirm keeps the implicit state, we don't actually need the first
	// parameter; it's just to make sure that the user keeps the firm_ir object
	void emit_x64_assembly_firm(firm_global_state& firm, file_output& output_file)
	{
		assert(firm);
		(void) firm; // suppress warning in release mode
		irg::lower();
		be_parse_arg("isa=amd64");
		be_main(output_file.handle(), output_file.filename().c_str());
	}

}  // namespace minijava