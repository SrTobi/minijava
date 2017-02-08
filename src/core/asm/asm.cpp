#include "asm/asm.hpp"

#include <cassert>
#include <cstddef>

#include "asm/allocator.hpp"
#include "asm/assembly.hpp"
#include "asm/data.hpp"
#include "asm/generator.hpp"
#include "asm/macros.hpp"
#include "asm/output.hpp"
#include "exceptions.hpp"
#include "irg/irg.hpp"


namespace minijava
{

	void assemble(firm_ir& ir, file_output& out)
	{
		assert(ir);
		const auto guard = make_irp_guard(*ir->second, ir->first);
		backend::write_data_segment(firm::get_glob_type(), out);
		out.print("\t.globl %s\n", "minijava_main");
		const auto n = firm::get_irp_n_irgs();
		for (std::size_t i = 0; i < n; ++i) {
			const auto irg = firm::get_irp_irg(i);
			const auto virtasm = backend::assemble_function(irg);
			//backend::write_text(virtasm, out);  // TODO: remove
			auto realasm = backend::allocate_registers(virtasm);
			backend::expand_macros(realasm);
			backend::write_text(realasm, out);
			out.write("\n");
		}
	}

}  // namespace minijava
