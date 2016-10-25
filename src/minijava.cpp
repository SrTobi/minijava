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
		std::cin.exceptions(std::iostream::badbit);
		std::cout.exceptions(std::iostream::badbit);
		minijava::real_main(args, std::cout, std::cerr);
		return EXIT_SUCCESS;
	} catch (const std::exception& e) {
		std::cerr << MINIJAVA_PROJECT_NAME << ": error: " << e.what() << "\n";
		return EXIT_FAILURE;
	}
}
