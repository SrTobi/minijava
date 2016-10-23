#include "cli.hpp"

#include <exception>
#include <fstream>
#include <ios>
#include <sstream>
#include <string>
#include <vector>

#include "testaux/temporary_file.hpp"
#include "testaux/testaux.hpp"

#define BOOST_TEST_MODULE  cli
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>


BOOST_AUTO_TEST_CASE(calling_real_main_with_help_option_does_something)
{
	using namespace std::string_literals;
	std::ostringstream mystdout{};
	std::ostringstream mystderr{};
	minijava::real_main({"", "--help"}, mystdout, mystderr);
	const auto actualout = mystdout.str();
	const auto actualerr = mystderr.str();
	BOOST_REQUIRE_NE(""s, actualout);
	BOOST_REQUIRE_EQUAL(""s, actualerr);
	BOOST_REQUIRE_EQUAL('\n', actualout.back());
}


BOOST_AUTO_TEST_CASE(calling_real_main_with_no_option_displays_help)
{
	using namespace std::string_literals;
	std::ostringstream defaultstdout{};
	std::ostringstream defaultstderr{};
	minijava::real_main({""}, defaultstdout, defaultstderr);
	std::ostringstream helpstdout{};
	std::ostringstream helpstderr{};
	minijava::real_main({"", "--help"}, helpstdout, helpstderr);
	BOOST_REQUIRE_EQUAL(defaultstdout.str(), helpstdout.str());
	BOOST_REQUIRE_EQUAL(defaultstderr.str(), helpstderr.str());
}


BOOST_AUTO_TEST_CASE(calling_real_main_with_version_option_does_something)
{
	using namespace std::string_literals;
	std::ostringstream mystdout{};
	std::ostringstream mystderr{};
	minijava::real_main({"", "--version"}, mystdout, mystderr);
	const auto actualout = mystdout.str();
	const auto actualerr = mystderr.str();
	BOOST_REQUIRE_NE(""s, actualout);
	BOOST_REQUIRE_EQUAL(""s, actualerr);
	BOOST_REQUIRE_EQUAL('\n', actualout.back());
}


static const testaux::you_can_print_me<std::vector<const char *>> garbage_data[] = {
	{{"", "--ergo"}},
	{{"", "--echo"}},
	{{"", "--echo", "--echo"}},
	{{"", "--echo", "foo", "bar", "baz"}},
	{{"", "the", "-bats", "are in the", "--belfry"}},
};

BOOST_DATA_TEST_CASE(calling_real_main_with_garbage_throws, garbage_data)
{
	using namespace std::string_literals;
	std::ostringstream mystdout{};
	std::ostringstream mystderr{};
	try {
		minijava::real_main(sample.value, mystdout, mystderr);
		TESTAUX_FAIL_NO_EXCEPTION();
	} catch (const std::exception& e) {
		BOOST_REQUIRE(e.what()[0] != '\0');
	}
	BOOST_REQUIRE_EQUAL(""s, mystdout.str());
	BOOST_REQUIRE_EQUAL(""s, mystderr.str());
}


static const std::string echo_data[] = {
	"",
	"\n",
	"    \n  \t  \n  ",
	"hello, world",
	"The quick\nbrown fox jumps\nover the sleazy\ndog.\n",
};

BOOST_DATA_TEST_CASE(echo_outputs_file_to_stdout, echo_data)
{
	using namespace std::string_literals;
	testaux::temporary_file tempfile{sample};
	std::ostringstream mystdout{};
	std::ostringstream mystderr{};
	minijava::real_main({"", "--echo", tempfile.filename().c_str()},
	                    mystdout, mystderr);
	const auto actualout = mystdout.str();
	const auto actualerr = mystderr.str();
	BOOST_REQUIRE_EQUAL(sample, actualout);
	BOOST_REQUIRE_EQUAL(""s, actualerr);
}


BOOST_AUTO_TEST_CASE(if_file_is_not_readable_echo_throws_exception_and_outputs_nothing)
{
	using namespace std::string_literals;
	const auto filename = testaux::temporary_file{}.filename();
	std::ostringstream mystdout{};
	std::ostringstream mystderr{};
	try {
		minijava::real_main({"", "--echo", filename.c_str()}, mystdout, mystderr);
		TESTAUX_FAIL_NO_EXCEPTION();
	} catch (const std::exception& e) { /* okay */ }
	BOOST_REQUIRE_EQUAL(""s, mystdout.str());
	BOOST_REQUIRE_EQUAL(""s, mystderr.str());
}


BOOST_AUTO_TEST_CASE(if_stdout_is_not_writeable_echo_throws_exception)
{
	using namespace std::string_literals;
	testaux::temporary_file tempfile{"abc"};
	std::ofstream mystdout{};  // invalid
	std::ostringstream mystderr{};
	try {
		minijava::real_main({"", "--echo", tempfile.filename().c_str()},
		                    mystdout, mystderr);
		TESTAUX_FAIL_NO_EXCEPTION();
	} catch (const std::exception& e) { /* okay */ }
	BOOST_REQUIRE_EQUAL(""s, mystderr.str());
}
