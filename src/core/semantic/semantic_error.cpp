#include "semantic/semantic_error.hpp"


namespace minijava
{

	semantic_error::semantic_error()
		: std::runtime_error{"ill-formed program"}
	{
	}

	semantic_error::semantic_error(const std::string& msg)
		: std::runtime_error{msg}
	{
	}

}  // namespace minijava
