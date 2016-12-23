#include "asm/text.hpp"

#include <cassert>

#include "exceptions.hpp"


namespace minijava
{

	namespace backend
	{

		void assemble_function(firm::ir_graph* irg, virtual_assembly& /*virtasm*/)
		{
			assert(irg != nullptr);
			MINIJAVA_NOT_IMPLEMENTED();
		}

		void allocate_registers(const virtual_assembly& /*virtasm*/, real_assembly& /*realasm*/)
		{
			MINIJAVA_NOT_IMPLEMENTED();
		}

		void write_text(const real_assembly& /*realasm*/, file_output& /*out*/)
		{
			MINIJAVA_NOT_IMPLEMENTED();
		}

	}  // namespace backend

}  // namespace minijava
