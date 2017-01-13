#pragma once

#include "firm.hpp"
#include "io/file_output.hpp"


namespace minijava
{

	namespace backend
	{

		/**
		 * @brief
		 *     Emits assembly code in AT&T syntax for the data segment.
		 *
		 * The generated assembly code initializes the global data structures
		 * in `glob` and does nothing else.
		 *
		 * @param glob
		 *     Firm global type
		 *
		 * @param out
		 *     assembly output file
		 *
		 */
		void write_data_segment(firm::ir_type* glob, file_output& out);

	}  // namespace backend

}  // namespace minijava
