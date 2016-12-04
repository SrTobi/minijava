#include "system/subprocess.hpp"

#include <string>
#include <vector>

#include <boost/algorithm/string/join.hpp>

#define BOOST_TEST_MODULE  system_subprocess
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "testaux/temporary_file.hpp"


namespace /* anonymous */
{

	struct command
	{
		template <typename... ArgTs>
		command(ArgTs&&... args) : tokens{std::forward<ArgTs>(args)...}
		{
		}

		std::vector<std::string> tokens{};
	};

	std::ostream& operator<<(std::ostream& os, const command& cmd)
	{
		using namespace std::string_literals;
		return os << boost::algorithm::join(cmd.tokens, " "s);
	}

}  // namespace /* anonymous */


// TODO: Check whether these work on Windows, too.  If not, and no portable
//       commands can be found, use `#ifdef __unix__` to selectively add those
//       that work.
static const command successful_commands[] = {
	{"true"},
	{"test", "42", "-eq", "42"},
};

BOOST_DATA_TEST_CASE(subprocess_success, successful_commands)
{
	minijava::run_subprocess(sample.tokens);
}


static const command failing_commands[] = {
	{"false"},
	{"test", "0", "-gt", "1"},
};

BOOST_DATA_TEST_CASE(subprocess_faulure, failing_commands)
{
	BOOST_CHECK_THROW(
		minijava::run_subprocess(sample.tokens),
		std::runtime_error
	);
}


BOOST_AUTO_TEST_CASE(subprocess_error)
{
	const auto cmd = [](){
		const testaux::temporary_file temp{};
		return temp.filename();
	}();
	BOOST_CHECK_THROW(
		minijava::run_subprocess({cmd}),
		std::system_error
	);
}


BOOST_AUTO_TEST_CASE(subprocess_invalid)
{
	BOOST_CHECK_THROW(
		minijava::run_subprocess({}),
		std::invalid_argument
	);
}
