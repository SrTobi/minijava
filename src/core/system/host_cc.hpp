/**
 * @file host_cc.hpp
 *
 * @brief
 *     Helpers for calling the (GCC-compatible) compiler on the host system.
 *
 */

#pragma once

#include <string>

namespace minijava
{

	/**
	 * @brief
	 *     Returns the default command for a GCC-compatible C compiler on the
	 *     current system.
	 *
	 * @return
	 *     C compiler command
	 *
	 */
	std::string get_default_c_compiler_command();

	/**
	 * @brief
	 *     Links the given assembly against the minijava runtime using the
	 *     given C compiler.
	 *
	 * @param compiler_command
	 *     command used for invoking the (GCC-compatible) C compiler
	 *
	 * @param output_file
	 *     path to the output file
	 *
	 * @param minijava_assembly
	 *     path to the assembly file containing the minijava program
	 *
	 */
	void link_runtime(const std::string& compiler_command,
	                  const std::string& output_file,
	                  const std::string& minijava_assembly);

}
