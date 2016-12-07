#include "firm/firm.hpp"

#include "libfirm/firm.h"

#include "firm/type_builder.hpp"
#include "firm/method_builder.hpp"
#include "firm/lowering.hpp"

namespace minijava
{

	firm_global_state create_firm_ir(const ast::program& ast, const semantic_info& semantic_info)
	{
		auto ir = firm_global_state{};
		auto types = firm::create_types(ast, semantic_info);
		firm::create_methods(ast, semantic_info, types);
		return ir;
	}

	// since libfirm keeps the implicit state, we don't actually need the first
	// parameter; it's just to make sure that the user keeps the firm_global_state object
	void dump_firm_ir(firm_global_state& firm, const std::string& directory)
	{
		assert(firm);
		if (!directory.empty()) {
			ir_set_dump_path(directory.c_str());
		}
		dump_all_ir_graphs(".vcg");
	}


	// since libfirm keeps the implicit state, we don't actually need the first
	// parameter; it's just to make sure that the user keeps the firm_ir object
	void emit_x64_assembly_firm(firm_global_state& firm, file_output& output_file)
	{
		assert(firm);
		// FIXME: implement lowering properly (?), see https://github.com/MatzeB/jFirm/blob/master/src/example/Lower.java
		// FIXME: do we need to call ir_lower_intrinsics here?
		// FIXME: the below is just a placeholder, I have no idea what I'm doing
		// lower_highlevel();
		firm::lower();
		dump_all_ir_graphs("after-lowering");
		be_parse_arg("isa=amd64");
		be_main(output_file.handle(), output_file.filename().c_str());
	}

}  // namespace minijava
