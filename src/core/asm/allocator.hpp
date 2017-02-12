/**
 * @file allocator.hpp
 *
 * @brief
 *     Register allocation.
 *
 */

#pragma once

#include "asm/assembly.hpp"


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
		 * @returns
		 *     real assembly listing
		 *
		 */
		real_assembly allocate_registers(const virtual_assembly& virtasm);

	}  // namespace backend

}  // namespace minijava
