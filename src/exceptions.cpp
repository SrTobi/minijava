#include "exceptions.hpp"


namespace minijava
{

	namespace /* anonymous */
	{

		const char default_ice_msg[] = "Internal compiler error";

		std::string compose_with_src_loc(const std::string& file,
										 const int line,
										 const std::string& func,
										 const std::string& msg = default_ice_msg)
		{
			return file + ":" + std::to_string(line) + ": " + func + ": " + msg;
		}

	}

	internal_compiler_error::internal_compiler_error() :
		std::runtime_error{default_ice_msg}
	{
	}

	explicit internal_compiler_error::internal_compiler_error(const std::string& msg) :
		std::runtime_error{msg}
	{
	}

	internal_compiler_error::internal_compiler_error(const std::string& file,
													 const int line,
													 const std::string& func) :
		std::runtime_error{compose_with_src_loc(file, line, func)}
	{
	}

	internal_compiler_error::internal_compiler_error(const std::string& file,
													 const int line,
													 const std::string& func,
													 const std::string& msg) :
		std::runtime_error{compose_with_src_loc(file, line, func, msg)}
	{
	}

	not_implemented_error::not_implemented_error() :
		internal_compiler_error{"Not implemented yet"}
	{
	}

}  // namespace minijava
