#include "asm/asm.hpp"

#include <cassert>
#include <cstddef>

#include "asm/data.hpp"
#include "asm/text.hpp"
#include "irg/irg.hpp"


namespace minijava
{

	void assemble(firm_ir& ir, file_output& out)
	{
		assert(ir);
		const auto guard = make_irp_guard(*ir->second, ir->first);
		backend::write_data_segment(firm::get_glob_type(), out);
		const auto n = firm::get_irp_n_irgs();
		auto virtasm = backend::virtual_assembly{};
		auto realasm = backend::real_assembly{};
		for (std::size_t i = 0; i < n; ++i) {
			const auto irg = firm::get_irp_irg(i);
			virtasm.clear();
			backend::assemble_function(irg, virtasm);
			realasm.clear();
			backend::allocate_registers(virtasm, realasm);
			backend::write_text(realasm, out);
		}
	}

}  // namespace minijava