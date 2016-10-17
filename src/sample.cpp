#include <iostream>


#ifndef MINIJAVA_TEST

int main(int argc, char * * argv) {
	std::cout << "Hello Compiler!" << std::endl;
	for (auto i = 0; i < argc; ++i) {
		std::cout << "argv[" << i << "] = " << argv[i] << "\n";
	}
	return 0;
}

#endif
