/**
 * @file generator.hpp
 *
 * @brief
 *     Backend for text segment assembly generation.
 *
 */

#pragma once

#include "asm/instruction.hpp"
#include "firm.hpp"
#include "io/file_output.hpp"


namespace minijava
{

	namespace backend
	{

		/**
		 * @brief
		 *     Generates virtual assembly for a function given by an IRG.
		 *
		 * @param irg
		 *     lowered and optimized Firm IRG for the function
		 *
		 * @param virtasm
		 *     virtual assembly listing to append to
		 *
		 */
		void assemble_function(firm::ir_graph* irg, virtual_assembly& virtasm);

	}  // namespace backend

}  // namespace minijava
