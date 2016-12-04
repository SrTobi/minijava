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
	 *     Returns the default executable for a GCC-compatible C compiler on the
	 *     current system.
	 *
	 * @return
	 *     C compiler executable
	 *
	 */
	std::string get_default_c_compiler();

	/**
	 * @brief
	 *     Links the given assembly against the minijava runtime using the
	 *     given C compiler.
	 *
	 * @param compiler_executable
	 *     executable of the (GCC-compatible) C compiler
	 *
	 * @param output_filename
	 *     path to the output file
	 *
	 * @param assembly_filename
	 *     path to the assembly file containing the minijava program
	 *
	 */
	void link_runtime(const std::string& compiler_executable,
	                  const std::string& output_filename,
	                  const std::string& assembly_filename);

}
