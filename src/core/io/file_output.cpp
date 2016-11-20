#include "io/file_output.hpp"

#include <cassert>
#include <cerrno>
#include <string>
#include <system_error>


namespace minijava
{

	namespace detail
	{

		namespace /* anonymous */
		{

			std::string compose_errmsg(const std::string& message, const std::string& filename)
			{
				const auto m = !message.empty()  ? 'M' : 0;
				const auto f = !filename.empty() ? 'F' : 0;
				switch (m + f) {
				case 'M':        return message;
				case 'F':        return filename;
				case 'M' + 'F':  return message + ": " + filename;
				default:         return "I/O error";
				}
			}

		}  // namespace /* anonymous */


		[[noreturn]]
		void file_output_throw_empty_state(const char* func)
		{
			using namespace std::string_literals;
			assert(func != nullptr);
			const auto ec = std::error_code{EBADF, std::generic_category()};
			throw std::system_error{ec, "minijava::file_output::"s + func};
		}

		[[noreturn]]
		void file_output_throw_errno(const std::string& message, const std::string& filename)
		{
			const auto ec = std::error_code{errno, std::system_category()};
			throw std::system_error{ec, compose_errmsg(message, filename)};
		}

	}  // namespace detail

}  // namespace minijava
