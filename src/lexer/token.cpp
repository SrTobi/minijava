#include "token.hpp"

#include <ostream>


namespace minijava
{

	// TODO @Moritz Klammler: Finish the implementation of this class.

	std::ostream& operator<<(std::ostream& os, const token& tok)
	{
		os << tok.type();
		if (tok.type() == token_type::identifier)
			os << ' ' << tok.name();
		if (tok.type() == token_type::integer_literal)
			os << ' ' << tok.value();
		os << " [line: " << tok.line() << ", column: " << tok.column() << "]";
		return os;
	}

}  // namespace minijava
