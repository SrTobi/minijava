#include "string.hpp"

#include <ostream>


namespace minijava
{

	std::ostream&
	operator<<(std::ostream& os, const string& str)
	{
		return (os << str.c_str());
	}

}  // namespace minijava
