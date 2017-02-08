#include "asm/data.hpp"

#include <cassert>

#include "exceptions.hpp"


namespace minijava
{

	namespace backend
	{

		void write_data_segment(firm::ir_type*const glob, file_output& out)
		{
			assert(glob != nullptr);  (void) glob;
			out.write("\t.data\n");
			out.write("\t.p2align 3\n");
			out.write("\t.type .mj_g1, @object\n");
			out.write("\t.size .mj_g1, 16\n");
			out.write(".mj_g1:\n");
			out.write("\t.quad .mj_g2\n");
			out.write("\t.quad .mj_g3\n");
			out.write("\t.local .mj_g2\n");
			out.write("\t.comm .mj_g2,4,4\n");
			out.write("\t.local .mj_g3\n");
			out.write("\t.comm .mj_g3,4,4\n");
			out.write("\t.p2align 3\n");
			out.write("\t.type mj_System_v6, @object\n");
			out.write("\t.size mj_System_v6, 8\n");
			out.write("mj_System_v6:\n");
			out.write("\t.quad .mj_g1\n");
		}

	}  // namespace backend

}  // namespace minijava
