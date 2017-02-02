#include "runtime/runtime.hpp"

static const char source_code[] = {
#include "runtime/runtime.tpp"
};


namespace minijava
{

	std::string runtime_source()
	{
		return source_code;
	}

}  // namespace minijava
