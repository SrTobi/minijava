#include "lexer.hpp"

// This file is empty.

namespace minijava
{

	namespace /* anonymous */
	{
		const char default_lexical_error_msg[] = "Invalid input";
	}

	lexical_error::lexical_error()
			: source_error{default_lexical_error_msg}
	{
	}

	lexical_error::lexical_error(const std::string& msg)
			: source_error{msg}
	{
	}

}  // namespace minijava
