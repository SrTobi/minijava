/**
 * @file macros.hpp
 *
 * @brief
 *     Assembly macro expansion.
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
		 *     Expands assembly "macros" in-place in the given assembly such
		 *     that it can be fed to the host assembler.
		 *
		 * @param assembly
		 *     assembly to macro-expand
		 *
		 */
		void expand_macros(real_assembly& assembly);

	}  // namespace backend

}  // namespace minijava
