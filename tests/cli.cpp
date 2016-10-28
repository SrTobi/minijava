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


// List of all options that select a specific compilier action.
static const std::string all_action_options[] = {
	"--echo",
	"--lextest",
};


// Unspectacular valid MiniJava program.
static const std::string valid_program_data = R"java(
	class Fibonacci {
		public int[] compute(int n) {
			/* Program will crash if n < 2 and this is a comment. */
			int[] values = new int[n];
			values[0] = 0;
			values[1] = 1;
			for (int i = 2; i < n; ++i) {
				values[i] = values[i - 1] + values[i - 2];
			}
			return values;
		}
	}

	class Main {
		public static void main(String[] args) {
			Fibonacci fib = new Fibonacci();
			int[] sequence = fib.compute(10);
			for (int i = 0; i < 10; ++i) {
				System.out.println(sequence[i]);
			}
		}
	}
)java";


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


BOOST_AUTO_TEST_CASE(calling_real_main_with_no_arguments_is_an_error)
{
	using namespace std::string_literals;
	std::ostringstream mystdout{};
	std::ostringstream mystderr{};
	try {
		minijava::real_main({""}, mystdout, mystderr);
		TESTAUX_FAIL_NO_EXCEPTION();
	} catch (const std::exception& e) {
		BOOST_REQUIRE(e.what()[0] != '\0');
	}
	BOOST_REQUIRE_EQUAL(""s, mystdout.str());
	BOOST_REQUIRE_EQUAL(""s, mystderr.str());
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
	{{"", "--echo", "--lextest"}},
	{{"", "--echo", "foo", "bar", "baz"}},
	{{"", "--lextest", "foo", "bar", "baz"}},
	{{"", "--echo", "bar", "--lextest", "baz"}},
	{{"", "foo", "--echo", "bar", "--lextest", "baz"}},
	{{"", "--no-such-option", "--echo", "somefile"}},
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


BOOST_DATA_TEST_CASE(all_actions_succeed_for_valid_arguments_and_valid_input,
                     all_action_options)
{
	using namespace std::string_literals;
	testaux::temporary_file tempfile{valid_program_data};
	std::ostringstream mystdout{};
	std::ostringstream mystderr{};
	minijava::real_main({"", sample.c_str(), tempfile.filename().c_str()},
	                    mystdout, mystderr);
	BOOST_REQUIRE_NE(""s, mystdout.str());
	BOOST_REQUIRE_EQUAL(""s, mystderr.str());
}


BOOST_DATA_TEST_CASE(if_file_is_not_readable_all_actions_throw_and_output_nothing,
                     all_action_options)
{
	using namespace std::string_literals;
	const auto filename = testaux::temporary_file{}.filename();
	std::ostringstream mystdout{};
	std::ostringstream mystderr{};
	try {
		minijava::real_main({"", sample.c_str(), filename.c_str()},
		                    mystdout, mystderr);
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


BOOST_AUTO_TEST_CASE(lextest_for_valid_token_sequence_produces_correct_output)
{
	using namespace std::string_literals;
	testaux::temporary_file tempfile{"42 abstract classes throw 1 + 3 mice."};
	const auto expected_output = ""s
		+ "integer literal 42\n"
		+ "abstract\n"
		+ "identifier classes\n"
		+ "throw\n"
		+ "integer literal 1\n"
		+ "+\n"
		+ "integer literal 3\n"
		+ "identifier mice\n"
		+ ".\n"
		+ "EOF\n";
	std::ostringstream mystdout{};
	std::ostringstream mystderr{};
	minijava::real_main({"", "--lextest", tempfile.filename().c_str()},
	                    mystdout, mystderr);
	BOOST_REQUIRE_EQUAL(expected_output, mystdout.str());
	BOOST_REQUIRE_EQUAL(""s, mystderr.str());
}


BOOST_AUTO_TEST_CASE(lextest_for_invalid_token_sequence_throws_exception)
{
	using namespace std::string_literals;
	testaux::temporary_file tempfile{"int nan = 034g7;"};
	std::ostringstream mystdout{};
	std::ostringstream mystderr{};
	try {
		minijava::real_main({"", "--lextest", tempfile.filename().c_str()},
		                    mystdout, mystderr);
		TESTAUX_FAIL_NO_EXCEPTION();
	} catch (const std::exception& e) { /* okay */ }
	BOOST_REQUIRE_EQUAL(""s, mystderr.str());
	// It is unspecified what will be written to standard output in this case.
}
