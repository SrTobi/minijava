#pragma once

#include <iosfwd>
#include <vector>

namespace minijava
{

	/**
	 * @brief
	 *     Runs the compiler with the provided command-line arguments.
	 *
	 * @param args
	 *     command-line arguments
	 *
	 * @param thestdout
	 *     destination for regular output
	 *
	 * @param thestderr
	 *     destination for error output
	 *
	 * @throws std::exception
	 *     on failure to successfully complete the requested task
	 *
	 */
	void real_main(const std::vector<const char *>& args,
	               std::ostream& thestdout,
	               std::ostream& thestderr);

	// `stdout` and `stderr` are standard-library macros so we cannot use them
	// as parameter names.

}  // namespace minijava
