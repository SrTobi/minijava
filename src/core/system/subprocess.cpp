#include "system/subprocess.hpp"

#include <algorithm>
#include <cerrno>
#include <stdexcept>
#include <system_error>


namespace /* anonymous */
{

	[[noreturn]]
	void throw_invoke_subprocess_failed(int error, const std::string& cmd)
	{
		const auto ec = std::error_code{error, std::system_category()};
		throw std::system_error{ec, "Could not start '" + cmd + "'"};
	}

	[[noreturn]]
	void throw_subprocess_failed(const std::string& cmd)
	{
		using namespace std::string_literals;
		throw std::runtime_error{"Subprocess '" + cmd + "' exited with non-zero status"};
	}

	void do_run_subprocess(std::vector<char*> argv);

}  //  namespace /* anonymous */

namespace minijava
{

	void run_subprocess(const std::vector<std::string>& command)
	{
		if (command.size() < 1) {
			throw std::invalid_argument{"Cannot execute empty command"};
		}
		auto copy = command;  // Avoid const correctness issues.
		auto argv = std::vector<char*>{copy.size() + 1};
		std::transform(
			copy.begin(), copy.end(), argv.begin(),
			[](auto&& s) { return &s[0]; }
		);
		argv[copy.size()] = nullptr;
		return do_run_subprocess(std::move(argv));
	}

}  // namespace minijava


#define MINIJAVA_INCLUDED_FROM_SYSTEM_SUBPROCESS_CPP
#  if defined (__unix__)
#    include "system/subprocess_posix.tpp"
#  elif defined (_WIN32)
#    include "system/subprocess_windows.tpp"
#  else
#    include "system/subprocess_generic.tpp"
#  endif
#undef MINIJAVA_INCLUDED_FROM_SYSTEM_SUBPROCESS_CPP
