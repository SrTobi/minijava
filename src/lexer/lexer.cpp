#include "lexer.hpp"

// This file is empty.

namespace minijava
{

	namespace /* anonymous */
	{
		const char default_lexical_error_msg[] = "Invalid input";
	}

	lexical_error::lexical_error()
			: std::runtime_error{default_lexical_error_msg}
	{
	}

	lexical_error::lexical_error(const std::string& msg)
			: std::runtime_error{msg}
	{
	}

}  // namespace minijava