/**
 * @file allocator.hpp
 *
 * @brief
 *     Register allocation.
 *
 */

#pragma once

#include "asm/instruction.hpp"


namespace minijava
{

	namespace backend
	{

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

	}  // namespace backend

}  // namespace minijava
