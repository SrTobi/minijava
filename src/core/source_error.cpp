#include "parser/parser.hpp"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <sstream>
#include <string>
#include <utility>

#include <boost/algorithm/string/join.hpp>


namespace minijava
{
	source_error::source_error(const std::string& msg, const minijava::position pos)
		: std::runtime_error{msg}
		, _position{pos}
	{
	}


	position source_error::position() const noexcept
	{
		return _position;
	}
}
