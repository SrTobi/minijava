#include "symbol.hpp"

#include <ostream>


namespace minijava
{

	const std::shared_ptr<symbol_debug_pool_anchor>& symbol_debug_pool_anchor::get_empty_symbol_anchor() noexcept
	{
		static const auto empty_symbol_anchor = std::make_shared<symbol_debug_pool_anchor>();
		return empty_symbol_anchor;
	}

	std::ostream&
	operator<<(std::ostream& os, const symbol& str)
	{
		return (os << str.c_str());
	}

}  // namespace minijava
