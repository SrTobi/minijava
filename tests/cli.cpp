// Important Note: This file contains specially commented-out code that should
// be un-commented again, once the lexer is powerful enough not to produce
// false errors.  The relevant sections of code are readily identified by
// beeing commented-out with four "////" leading slashes.

// TODO: Uncomment some code once the lexer is ready.


#include "cli.hpp"

#include <algorithm>
#include <climits>
#include <exception>
#include <fstream>
#include <ios>
#include <iterator>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "exceptions.hpp"

#include "testaux/temporary_file.hpp"
#include "testaux/testaux.hpp"

#define BOOST_TEST_MODULE  cli
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>


namespace /* anonymous */
{

	// Tests whether the file `filename` has the `expected` content.
	bool file_has_content(const std::string& filename,
	                      const std::string& expected)
	{
		std::ifstream istr{filename};
		if (!istr) {
			throw std::ios_base::failure{"Cannot open file: " + filename};
		}
		istr.exceptions(std::ios_base::badbit);
		const auto first_1 = std::istreambuf_iterator<char>{istr};
		const auto last_1 = std::istreambuf_iterator<char>{};
		const auto first_2 = std::begin(expected);
		const auto last_2 = std::end(expected);
		return std::equal(first_1, last_1, first_2, last_2);
	}

	// Creates a string with `n` uniformyly distributed random bytes.  The seed
	// is deterministic so the test behaves the same every time.
	std::string make_random_string(const std::size_t n)
	{
		auto rndeng = std::default_random_engine{};
		auto rnddst = std::uniform_int_distribution<char>{CHAR_MIN, CHAR_MAX};
		const auto lambda = [&rndeng, &rnddst](){ return rnddst(rndeng); };
		auto s = std::string(n, ' ');
		std::generate(std::begin(s), std::end(s), lambda);
		return s;
	}

}  // namespace /* anonymous */


// List of all options that select a specific compilier action.
static const std::string all_action_options[] = {
	"--echo",
	"--lextest",
};


static const std::string all_diagnostic_options[] = {
	"--help",
	"--version",
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


BOOST_DATA_TEST_CASE(diagnostic_options_produce_output_on_stdout,
                     all_diagnostic_options)
{
	using namespace std::string_literals;
	std::istringstream mystdin{};
	std::ostringstream mystdout{};
	std::ostringstream mystderr{};
	minijava::real_main({"", sample.c_str()}, mystdin, mystdout, mystderr);
	const auto actualout = mystdout.str();
	const auto actualerr = mystderr.str();
	BOOST_REQUIRE_NE(""s, actualout);
	BOOST_REQUIRE_EQUAL(""s, actualerr);
	BOOST_REQUIRE_EQUAL('\n', actualout.back());
}


BOOST_DATA_TEST_CASE(if_stdout_not_writable_diagnostic_options_throw,
                     all_diagnostic_options)
{
	using namespace std::string_literals;
	std::istringstream mystdin{};
	std::ostringstream mystdout{};
	std::ostringstream mystderr{};
	mystdout.setstate(std::ios_base::failbit);
	BOOST_REQUIRE_THROW(
		minijava::real_main({"", sample.c_str()}, mystdin, mystdout, mystderr)
	, std::exception);
	BOOST_REQUIRE_EQUAL(""s, mystderr.str());
}


BOOST_AUTO_TEST_CASE(calling_real_main_with_no_arguments_is_not_implemented)
{
	using namespace std::string_literals;
	std::istringstream mystdin{};
	std::ostringstream mystdout{};
	std::ostringstream mystderr{};
	BOOST_REQUIRE_THROW(
		minijava::real_main({""}, mystdin, mystdout, mystderr)
	, std::exception);
	BOOST_REQUIRE_EQUAL(""s, mystdout.str());
	BOOST_REQUIRE_EQUAL(""s, mystderr.str());
}


static const testaux::you_can_print_me<std::vector<const char *>> garbage_data[] = {
	{{"", "--ergo"}},
	{{"", "--echo", "--echo"}},
	{{"", "--echo=arg"}},
	{{"", "--lextest=arg"}},
	{{"", "--echo=arg", "-"}},
	{{"", "--output"}},
	{{"", "--output", "--lextest"}},
	{{"", "--echo", "--lextest"}},
	{{"", "--echo", "foo", "bar", "baz"}},
	{{"", "--lextest", "foo", "bar", "baz"}},
	{{"", "--echo", "bar", "--lextest", "baz"}},
	{{"", "foo", "--echo", "bar", "--lextest", "baz"}},
	{{"", "--no-such-option", "--echo", "somefile"}},
};

BOOST_DATA_TEST_CASE(garbage_throws, garbage_data)
{
	using namespace std::string_literals;

	auto exception_is_not_empty = [](const std::exception& e)
	{
		return e.what()[0] != '\0';
	};

	std::istringstream mystdin{};
	std::ostringstream mystdout{};
	std::ostringstream mystderr{};
	BOOST_REQUIRE_EXCEPTION(
			minijava::real_main(sample.value, mystdin, mystdout, mystderr)
	, std::exception, exception_is_empty);
	BOOST_REQUIRE_EQUAL(""s, mystdout.str());
	BOOST_REQUIRE_EQUAL(""s, mystderr.str());
}


// And now we're about to test all (input, output) combinations on the `--echo`
// stage.  Hang on...

static const std::string echo_data[] = {
	"",
	"\n",
	"    \n  \t  \n  ",
	"hello, world",
	"The quick\nbrown fox jumps\nover the sleazy\ndog.\n",
	std::string(1000, '\n'),
	std::string(1000000, 'a'),
	make_random_string(1000000),
};

BOOST_DATA_TEST_CASE(echo_implicit_stdin_to_implicit_stdout, echo_data)
{
	using namespace std::string_literals;
	std::istringstream mystdin{sample};
	std::ostringstream mystdout{};
	std::ostringstream mystderr{};
	minijava::real_main({"", "--echo"}, mystdin, mystdout, mystderr);
	BOOST_REQUIRE_EQUAL(sample, mystdout.str());
	BOOST_REQUIRE_EQUAL(""s, mystderr.str());
}


BOOST_DATA_TEST_CASE(echo_implicit_stdin_to_explpcit_stdout, echo_data)
{
	using namespace std::string_literals;
	std::istringstream mystdin{sample};
	std::ostringstream mystdout{};
	std::ostringstream mystderr{};
	minijava::real_main(
		{"", "--output", "-", "--echo"}, mystdin, mystdout, mystderr
	);
	BOOST_REQUIRE_EQUAL(sample, mystdout.str());
	BOOST_REQUIRE_EQUAL(""s, mystderr.str());
}


BOOST_DATA_TEST_CASE(echo_implicit_stdin_to_file, echo_data)
{
	using namespace std::string_literals;
	testaux::temporary_file tempfile{};
	std::istringstream mystdin{sample};
	std::ostringstream mystdout{};
	std::ostringstream mystderr{};
	minijava::real_main(
		{"", "--output", tempfile.filename().c_str(), "--echo"},
		mystdin, mystdout, mystderr
	);
	BOOST_REQUIRE_EQUAL(""s, mystdout.str());
	BOOST_REQUIRE_EQUAL(""s, mystderr.str());
	BOOST_REQUIRE(file_has_content(tempfile.filename(), sample));
}


BOOST_DATA_TEST_CASE(echo_explicit_stdin_to_implicit_stdout, echo_data)
{
	using namespace std::string_literals;
	std::istringstream mystdin{sample};
	std::ostringstream mystdout{};
	std::ostringstream mystderr{};
	minijava::real_main({"", "--echo", "-"}, mystdin, mystdout, mystderr);
	BOOST_REQUIRE_EQUAL(sample, mystdout.str());
	BOOST_REQUIRE_EQUAL(""s, mystderr.str());
}


BOOST_DATA_TEST_CASE(echo_explicit_stdin_to_explicit_stdout, echo_data)
{
	using namespace std::string_literals;
	std::istringstream mystdin{sample};
	std::ostringstream mystdout{};
	std::ostringstream mystderr{};
	minijava::real_main(
		{"", "--output", "-", "--echo", "-"}, mystdin, mystdout, mystderr
	);
	BOOST_REQUIRE_EQUAL(sample, mystdout.str());
	BOOST_REQUIRE_EQUAL(""s, mystderr.str());
}


BOOST_DATA_TEST_CASE(echo_explicit_stdin_to_file, echo_data)
{
	using namespace std::string_literals;
	testaux::temporary_file tempfile{};
	std::istringstream mystdin{sample};
	std::ostringstream mystdout{};
	std::ostringstream mystderr{};
	minijava::real_main(
		{"", "--output", tempfile.filename().c_str(), "--echo", "-"},
		mystdin, mystdout, mystderr
	);
	BOOST_REQUIRE_EQUAL(""s, mystdout.str());
	BOOST_REQUIRE_EQUAL(""s, mystderr.str());
	BOOST_REQUIRE(file_has_content(tempfile.filename(), sample));
}


BOOST_DATA_TEST_CASE(echo_file_to_implicit_stdout, echo_data)
{
	using namespace std::string_literals;
	testaux::temporary_file tempfile{sample};
	std::istringstream mystdin{};
	std::ostringstream mystdout{};
	std::ostringstream mystderr{};
	minijava::real_main(
		{"", "--echo", tempfile.filename().c_str()},
		mystdin, mystdout, mystderr
	);
	BOOST_REQUIRE_EQUAL(sample, mystdout.str());
	BOOST_REQUIRE_EQUAL(""s, mystderr.str());
}


BOOST_DATA_TEST_CASE(echo_file_to_explicit_stdout, echo_data)
{
	using namespace std::string_literals;
	testaux::temporary_file tempfile{sample};
	std::istringstream mystdin{};
	std::ostringstream mystdout{};
	std::ostringstream mystderr{};
	minijava::real_main(
		{"", "--echo", tempfile.filename().c_str(), "--output", "-"},
		mystdin, mystdout, mystderr
	);
	BOOST_REQUIRE_EQUAL(sample, mystdout.str());
	BOOST_REQUIRE_EQUAL(""s, mystderr.str());
}


BOOST_DATA_TEST_CASE(echo_file_to_file, echo_data)
{
	using namespace std::string_literals;
	testaux::temporary_file tempin{sample};
	testaux::temporary_file tempout{};
	std::istringstream mystdin{};
	std::ostringstream mystdout{};
	std::ostringstream mystderr{};
	minijava::real_main(
		{"", "--echo", tempin.filename().c_str(), "--output", tempout.filename().c_str()},
		mystdin, mystdout, mystderr
	);
	BOOST_REQUIRE_EQUAL(""s, mystdout.str());
	BOOST_REQUIRE_EQUAL(""s, mystderr.str());
	BOOST_REQUIRE(file_has_content(tempout.filename(), sample));
}


// We are NOT going to repeat this 3 x 3 combinatorial madness of input sources
// and output destinations for each interception stage.  Rather, we rely on our
// knowledge that the code for determining input and output is separate from
// the code that does the interception so if it works for one stage, it most
// likely works for all.  In order to make the tests run faster, we only test
// the remaining interception stages with in-memory streams.


BOOST_DATA_TEST_CASE(all_actions_succeed_for_valid_arguments_and_valid_input,
                     all_action_options)
{
	using namespace std::string_literals;
	//// std::istringstream mystdin{valid_program_data};
	std::istringstream mystdin{"alpha beta gamma delta"};
	std::ostringstream mystdout{};
	std::ostringstream mystderr{};
	minijava::real_main({"", sample.c_str()}, mystdin, mystdout, mystderr);
	BOOST_REQUIRE_NE(""s, mystdout.str());
	BOOST_REQUIRE_EQUAL(""s, mystderr.str());
}


BOOST_DATA_TEST_CASE(if_file_is_not_readable_all_actions_throw_and_output_nothing,
                     all_action_options)
{
	using namespace std::string_literals;
	const auto filename = testaux::temporary_file{}.filename();
	std::istringstream mystdin{};
	std::ostringstream mystdout{};
	std::ostringstream mystderr{};

	BOOST_REQUIRE_THROW(
		minijava::real_main(
			{"", sample.c_str(), filename.c_str()},
			mystdin, mystdout, mystderr
		)
	, std::exception);

	BOOST_REQUIRE_EQUAL(""s, mystdout.str());
	BOOST_REQUIRE_EQUAL(""s, mystderr.str());
}


BOOST_DATA_TEST_CASE(if_stdin_is_not_readable_all_actions_throw_and_output_nothing,
                     all_action_options)
{
	using namespace std::string_literals;
	std::ifstream mystdin{};
	std::ostringstream mystdout{};
	std::ostringstream mystderr{};
	mystdin.setstate(std::ios_base::badbit);

	BOOST_REQUIRE_THROW(
		minijava::real_main({"", sample.c_str()}, mystdin, mystdout, mystderr)
	, std::exception);
	BOOST_REQUIRE_EQUAL(""s, mystderr.str());
}


BOOST_DATA_TEST_CASE(if_stdout_is_not_writeable_all_actions_throw,
                     all_action_options)
{
	using namespace std::string_literals;
	std::istringstream mystdin{valid_program_data};
	std::ostringstream mystdout{};
	std::ostringstream mystderr{};
	mystdout.setstate(std::ios_base::failbit);

	BOOST_REQUIRE_THROW(
		minijava::real_main({"", sample.c_str()}, mystdin, mystdout, mystderr);
	, std::exception);
	BOOST_REQUIRE_EQUAL(""s, mystderr.str());
}


BOOST_AUTO_TEST_CASE(lextest_for_valid_token_sequence_produces_correct_output)
{
	using namespace std::string_literals;
	//// std::istringstream mystdin{"42 abstract classes throw 1 + 3 mice."};
	std::istringstream mystdin{"alpha beta gamma"};
	std::ostringstream mystdout{};
	std::ostringstream mystderr{};
	const auto expected_output = ""s
		+ "identifier alpha\n"
		+ "identifier beta\n"
		+ "identifier gamma\n"
		//// + "integer literal 42\n"
		//// + "abstract\n"
		//// + "identifier classes\n"
		//// + "throw\n"
		//// + "integer literal 1\n"
		//// + "+\n"
		//// + "integer literal 3\n"
		//// + "identifier mice\n"
		//// + ".\n"
		+ "EOF\n";
	minijava::real_main({"", "--lextest"}, mystdin, mystdout, mystderr);
	BOOST_REQUIRE_EQUAL(expected_output, mystdout.str());
	BOOST_REQUIRE_EQUAL(""s, mystderr.str());
}


BOOST_AUTO_TEST_CASE(lextest_for_invalid_token_sequence_throws_exception)
{
	using namespace std::string_literals;
	std::istringstream mystdin{"int nan = 034g7;"};
	std::ostringstream mystdout{};
	std::ostringstream mystderr{};
	BOOST_REQUIRE_THROW(
		minijava::real_main({"", "--lextest"}, mystdin, mystdout, mystderr)
	, std::exception);
	BOOST_REQUIRE_EQUAL(""s, mystderr.str());
	// It is unspecified what will be written to standard output in this case.
}
