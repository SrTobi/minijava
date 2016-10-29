#include <exception>
#include <ios>
#include <iostream>
#include <vector>

#include "cli.hpp"
#include "global.hpp"


int main(int argc, char * * argv)
{
	try {
		const auto args = std::vector<const char *>{argv, argv + argc};
		minijava::real_main(args, std::cin, std::cout, std::cerr);
		return EXIT_SUCCESS;
	} catch (const std::exception& e) {
		// NB: Don't alter the string "error: " -- it is required output.
		std::cerr << MINIJAVA_PROJECT_NAME << ": error: " << e.what() << "\n";
		return EXIT_FAILURE;
	}
}
