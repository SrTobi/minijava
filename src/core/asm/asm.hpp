/**
 * @file asm.hpp
 *
 * @brief
 *     Interface for x64 assembly backend.
 *
 */

#pragma once

#include "io/file_output.hpp"
#include "irg/irg.hpp"


namespace minijava
{

	/**
	 * @brief
	 *     Emits x64 assembly for the lowered IRG.
	 *
	 * This function performs no optimization.  This has to be done beforehand,
	 * if desired.
	 *
	 * @param ir
	 *     lowered Firm IRG
	 *
	 * @param out
	 *     file to which the assembly shall be written
	 *
	 */
	void assemble(firm_ir& ir, file_output& out);

	/**
	 * @brief
	 *     Assembly code generation backend.
	 *
	 */
	namespace backend {}

}  // namespace minijava
