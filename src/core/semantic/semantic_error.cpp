#include "semantic/semantic_error.hpp"


namespace minijava
{

	semantic_error::semantic_error()
		: source_error{"ill-formed program"}
	{
	}

	semantic_error::semantic_error(const std::string& msg, minijava::position pos)
		: source_error{msg, pos}
	{
	}

}  // namespace minijava
