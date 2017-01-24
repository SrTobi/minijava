/**
 * @file text.hpp
 *
 * @brief
 *     Backend for text segment assembly generation.
 *
 */

#pragma once

#include "asm/instruction.hpp"
#include "io/file_output.hpp"


namespace minijava
{

	namespace backend
	{

		/**
		 * @brief
		 *     Emits virtual x64 assembly code in AT&T syntax.
		 *
		 * @param virtasm
		 *     virtual assembly listing to output
		 *
		 * @param out
		 *     file to append to
		 *
		 */
		[[deprecated]]
		void write_text(const virtual_assembly& virtasm, file_output& out);

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
