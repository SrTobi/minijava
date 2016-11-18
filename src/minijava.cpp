#include <cstdio>
#include <cstdlib>
#include <exception>
#include <vector>

#include "cli.hpp"
#include "global.hpp"


int main(int argc, char * * argv)
{
	try {
		const auto args = std::vector<const char*>{argv, argv + argc};
		minijava::real_main(args, stdin, stdout, stderr);
		return EXIT_SUCCESS;
	} catch (const std::exception& e) {
		// NB: Don't alter the string "error: " -- it is required output.
		std::fprintf(stderr, "%s: error: %s\n", MINIJAVA_PROJECT_NAME, e.what());
		return EXIT_FAILURE;
	}
}
