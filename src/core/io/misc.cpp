#include "io/misc.hpp"

#include <cerrno>
#include <system_error>


namespace minijava
{

	void check_file_name(const std::string& filename)
	{
		const auto ec = std::error_code{EINVAL, std::generic_category()};
		if (filename.empty()) {
			throw std::system_error{ec, "File-name cannot be empty"};
		}
		if (filename.find('\0') != std::string::npos) {
			throw std::system_error{ec, "File-name cannot contain NUL bytes"};
		}
	}


}  // namespace minijava
