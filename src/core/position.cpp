#include "position.hpp"

#include <ostream>


namespace minijava
{

	std::ostream& operator<<(std::ostream& os, const minijava::position pos)
	{
		return os << "line: " << pos.line() << " column: " << pos.column();
	}

}
