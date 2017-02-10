/**
 * @file basic_block.hpp
 *
 * @brief
 *     Assembly code for basic blocks.
 *
 */

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "asm/instruction.hpp"


namespace minijava
{

	namespace backend
	{

		/**
		 * @brief
		 *     Assembly for a basic block.
		 *
		 * A *basic block* is list of consecutive assembly instructions with an
		 * associated label.  Control flow always passes through a basic block
		 * as a whole, never only through parts of it.
		 *
		 * @tparam RegT
		 *     register type (`virtual_register` or `real_register`)
		 *
		 */
		template <typename RegT>
		struct basic_block final
		{
			/** @brief Type of assembly instruction. */
			using instruction_type = instruction<RegT>;

			/**
			 * @brief
			 *     Cretaes a new basic block with the given label.
			 *
			 * @param thelabel
			 *     label for the block
			 *
			 */
			basic_block(std::string thelabel) : label{std::move(thelabel)}
			{
			}

			/** Label of the block. */
			std::string label{};

			/** Consecutive instructions in the block. */
			std::vector<instruction_type> code{};

			/** Scratch space for construction. */
			std::vector<instruction_type> scratch{};
		};

		/** @brief Basic block in virtual assembly. */
		using virtual_basic_block = basic_block<virtual_register>;

		/** @brief Basic block in real assembly. */
		using real_basic_block = basic_block<real_register>;

	}  // namespace backend

} // namespace minijava
