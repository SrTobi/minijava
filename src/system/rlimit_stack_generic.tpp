#ifndef MINIJAVA_INCLUDED_FROM_SYSTEM_SYSTEM_HPP
#error "Never `#include` the source file `<system/rlimit_stack_generic.tpp>`"
#endif

#include <cerrno>
#include <system_error>


namespace minijava
{

	std::ptrdiff_t set_max_stack_size_limit(const std::ptrdiff_t /* limit */)
	{
		const auto ec = std::error_code{ENOSYS, std::system_category()};
		throw std::system_error{ec};
	}


}  // namespace minijava
