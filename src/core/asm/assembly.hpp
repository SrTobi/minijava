/**
 * @file assembly.hpp
 *
 * @brief
 *     Assembly code for single functions.
 *
 */

#pragma once

#include <string>
#include <vector>

#include "asm/basic_block.hpp"


namespace minijava
{

	namespace backend
	{

		/**
		 * @brief
		 *     Assembly for a function block.
		 *
		 * @tparam RegT
		 *     register type (`virtual_register` or `real_register`)
		 *
		 */
		template <typename RegT>
		struct assembly final
		{
			/**
			 * @brief
			 *     Cretaes a new function assembly with the given linker name.
			 *
			 * @param thelabel
			 *     linker name for the function
			 *
			 */
			assembly(std::string thelabel) : ldname{std::move(thelabel)}
			{
			}

			/** @brief Linker name for the function. */
			std::string ldname{};

			/** @brief List of basic blocks. */
			std::vector<basic_block<RegT>> blocks{};
		};

		/** @brief Type used for x64 assemblies using virtual registers. */
		using virtual_assembly = assembly<virtual_register>;

		/** @brief Type used for x64 assemblies using real registers. */
		using real_assembly = assembly<real_register>;

	}  // namespace backend

} // namespace minijava
