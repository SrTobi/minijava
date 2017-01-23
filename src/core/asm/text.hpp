/**
 * @file text.hpp
 *
 * @brief
 *     Backend for text segment assembly generation.
 *
 */

#pragma once

#include <vector>

#include "asm/instruction.hpp"
#include "firm.hpp"
#include "io/file_output.hpp"


namespace minijava
{

	namespace backend
	{

		/** @brief Type used for x64 assemblies using virtual registers. */
		using virtual_assembly = std::vector<virtual_instruction>;

		/** @brief Type used for x64 assemblies using real registers. */
		using real_assembly = std::vector<real_instruction>;

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

		/**
		 * @brief
		 *     Converts virtual to real assembly by allocating registers.
		 *
		 * @param virtasm
		 *     virtual assembly listing to transform
		 *
		 * @param realasm
		 *     real assembly listing to append to
		 *
		 */
		void allocate_registers(const virtual_assembly& virtasm, real_assembly& realasm);

		/**
		 * @brief
		 *     Emits x64 assembly code in AT&T syntax.
		 *
		 * @param realasm
		 *     real assembly listing to output
		 *
		 * @param out
		 *     file to append to
		 *
		 */
		void write_text(const real_assembly& realasm, file_output& out);

	}  // namespace backend

}  // namespace minijava
