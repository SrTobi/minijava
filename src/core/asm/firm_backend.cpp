#include "asm/firm_backend.hpp"

#include <cassert>

#include "firm.hpp"

namespace minijava
{

	void emit_x64_assembly_firm(firm_ir& ir, file_output& output_file)
	{
		assert(ir);
		const auto guard = make_irp_guard(*ir->second, ir->first);
		firm::be_parse_arg("isa=amd64");
		firm::be_main(output_file.handle(), output_file.filename().c_str());
	}

}
