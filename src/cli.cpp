#include "cli.hpp"

#include <iostream>

namespace minijava
{

	void real_main(const std::vector<std::string>& args)
	{
		for (auto&& arg : args) {
			std::cout << arg << '\n';
		}
	}

}  // namespace minijava
