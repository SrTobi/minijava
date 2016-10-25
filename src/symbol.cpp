#include "symbol.hpp"

#include <ostream>


namespace minijava
{

	std::ostream&
	operator<<(std::ostream& os, const symbol& str)
	{
		return (os << str.c_str());
	}

}  // namespace minijava
