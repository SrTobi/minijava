/**
 * @file generator.hpp
 *
 * @brief
 *     Backend for text segment assembly generation.
 *
 */

#pragma once

#include "asm/assembly.hpp"
#include "firm.hpp"


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
		 * @returns
		 *     virtual assembly listing
		 *
		 */
		virtual_assembly assemble_function(firm::ir_graph* irg);

	}  // namespace backend

}  // namespace minijava
