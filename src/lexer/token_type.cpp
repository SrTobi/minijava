#include "token_type.hpp"

#include <ostream>


namespace minijava
{

	std::ostream& operator<<(std::ostream& os, const token_type tt)
	{
		if (const auto fancy = fancy_name(tt)) {
			os << fancy;
		} else {
			os << "invalid token type " << static_cast<int>(tt);
		}
		return os;
	}

}  // namespace minijava
