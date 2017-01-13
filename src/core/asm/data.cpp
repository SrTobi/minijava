#include "asm/data.hpp"

#include <cassert>

#include "exceptions.hpp"


namespace minijava
{

	namespace backend
	{

		void write_data_segment(firm::ir_type*const glob, file_output& /*out*/)
		{
			assert(glob != nullptr);
			(void) glob;
			MINIJAVA_NOT_IMPLEMENTED();
		}

	}  // namespace backend

}  // namespace minijava
