/**
 * @file subprocess.hpp
 *
 * @brief
 *     Helper functions for executing a subprocess on the current platform.
 *
 */

#pragma once

#include <string>
#include <vector>

namespace minijava
{

	/**
	 * @brief
	 *     Runs a subprocess with the given command line.
	 *
	 * @param command
	 *     command line of the subprocess
	 *
	 * @throws std::runtime_error
	 *     if the subprocess fails
	 *
	 * @throws std::system_error
	 *     if an error occurs while trying to start/join the subprocess
	 *
	 */
	void run_subprocess(const std::vector<std::string>& command);

}
