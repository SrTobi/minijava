#pragma once

#include <string>
#include <vector>

namespace minijava
{

	/**
	 * @brief
	 *         Runs the compiler with the provided command-line
	 *         arguments.
	 *
	 * @param args
	 *         command-line arguments (starting at `argv[1]`)
	 *
	 * @throws std::exception
	 *         on failure to successfully complete the requested task
	 *
	 */
	void real_main(const std::vector<std::string>& args);

}  // namespace minijava
