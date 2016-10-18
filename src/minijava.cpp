#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "cli.hpp"

#ifndef MINIJAVA_TEST

int main(int argc, char * * argv) {
	try {
		auto args = std::vector<std::string>{};
		for (auto i = 0; i < argc; ++i) {
			args.push_back(argv[i]);
		}
		minijava::real_main(args);
		return EXIT_SUCCESS;
	} catch (const std::exception& e) {
		std::cerr << "minijava: error: " << e.what() << "\n";
		return EXIT_FAILURE;
	}
}

#endif
