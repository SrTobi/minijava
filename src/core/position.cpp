#include "position.hpp"
#include <ostream>

namespace minijava
{

	bool operator==(const minijava::position& lhs, const minijava::position& rhs) noexcept
	{
		return lhs.line() == rhs.line() && lhs.column() == rhs.column();
	}

	bool operator!=(const minijava::position& lhs, const minijava::position& rhs) noexcept
	{
		return !(lhs == rhs);
	}

	std::ostream& operator<<(std::ostream& os, const minijava::position& pos)
	{
		os << "line: " << pos.line()
		   << " column: " << pos.column();
		return os;
	}
}