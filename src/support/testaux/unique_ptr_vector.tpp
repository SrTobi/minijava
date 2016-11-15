#ifndef TESTAUX_INCLUDED_FROM_UNIQUE_PTR_VECTOR_HPP
#error "Never `#include <unique_ptr_vector.tpp>` directly; `#include <unique_ptr_vector.hpp>` instead."
#endif

#include <algorithm>

namespace testaux
{
	template <typename T, typename... ArgTs>
	std::enable_if_t<
			detail::conjunction(std::is_convertible<ArgTs, std::unique_ptr<T>>{}...),
			std::vector<std::unique_ptr<T>>
	>
	make_unique_ptr_vector(ArgTs... args)
	{
		std::vector<std::unique_ptr<T>> result{};
		result.reserve(sizeof...(args));
		int dummy[] = {0, (result.emplace_back(std::move(args)), 0)... };
		(void) dummy;
		return result;
	}
}
