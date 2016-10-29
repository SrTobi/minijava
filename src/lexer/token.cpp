#include "token.hpp"

#include <ostream>
#include <regex>


namespace minijava
{

	namespace detail
	{

		bool is_valid_identifier(const symbol lexval) noexcept
		{
			static const auto pattern = std::regex{"[A-Za-z_][0-9A-Za-z_]*"};
			return std::regex_match(lexval.c_str(), pattern);
		}

		bool is_valid_integer_literal(const symbol lexval) noexcept
		{
			static const auto pattern = std::regex{"0|[1-9][0-9]*"};
			return std::regex_match(lexval.c_str(), pattern);
		}

	}

	std::ostream& operator<<(std::ostream& os, const token& tok)
	{
		os << tok.type();
		if (tok.has_lexval()) {
			os << ' ' << tok.lexval();
		}
		return os;
	}

}  // namespace minijava
