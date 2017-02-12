#include "cli.hpp"

#include <algorithm>
#include <cstdio>
#include <exception>
#include <string>
#include <vector>

#include "exceptions.hpp"

#include "testaux/random.hpp"
#include "testaux/temporary_file.hpp"
#include "testaux/testaux.hpp"

#include "parser/parser.hpp"

#define BOOST_TEST_MODULE  cli
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/filesystem.hpp>

namespace
{
	bool have_dev_full()
	{
		namespace fs = boost::filesystem;
		static const bool result = fs::exists(fs::path("/dev/full"));
		return result;
	}
}

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
		int i = 2;
		while (i < n) {
			values[i] = values[i - 1] + values[i - 2];
			i = i + 1;
		}
		return values;
	}
}

class Main {
	public static void main(String[] args) {
		Fibonacci fib = new Fibonacci();
		int[] sequence = fib.compute(10);
		System.out.println(sequence[9]);
	}
}
)java";


BOOST_DATA_TEST_CASE(diagnostic_options_produce_output_on_stdout,
                     all_diagnostic_options)
{
	using namespace std::string_literals;
	testaux::temporary_file in{};
	testaux::temporary_file out{};
	testaux::temporary_file err{};
	auto fh_in = testaux::open_file(in.filename(), "rb");
	auto fh_out = testaux::open_file(out.filename(), "wb");
	auto fh_err = testaux::open_file(err.filename(), "wb");
	minijava::real_main({"", sample.c_str()}, fh_in.get(), fh_out.get(), fh_err.get());
	BOOST_REQUIRE(not testaux::file_has_content(out.filename(), ""s));
	BOOST_REQUIRE(testaux::file_has_content(err.filename(), ""s));
}


BOOST_DATA_TEST_CASE(if_stdout_not_writable_diagnostic_options_throw,
                     all_diagnostic_options)
{
	if (have_dev_full()) {
		using namespace std::string_literals;
		testaux::temporary_file in{};
		testaux::temporary_file err{};
		auto fh_in = testaux::open_file(in.filename(), "rb");
		auto fh_out = testaux::open_file("/dev/full", "wb");
		auto fh_err = testaux::open_file(err.filename(), "wb");
		BOOST_REQUIRE_THROW(
			minijava::real_main(
				{"", sample.c_str()},
				fh_in.get(), fh_out.get(), fh_err.get()
			),
			std::exception
		);
		BOOST_REQUIRE(testaux::file_has_content(err.filename(), ""s));
	}
}


BOOST_AUTO_TEST_CASE(calling_real_main_with_no_arguments_is_not_implemented)
{
	using namespace std::string_literals;
	testaux::temporary_file in{};
	testaux::temporary_file out{};
	testaux::temporary_file err{};
	auto fh_in = testaux::open_file(in.filename(), "rb");
	auto fh_out = testaux::open_file(out.filename(), "wb");
	auto fh_err = testaux::open_file(err.filename(), "wb");
	BOOST_REQUIRE_THROW(
		minijava::real_main({""}, fh_in.get(), fh_out.get(), fh_err.get()),
		std::exception
	);
	BOOST_REQUIRE(testaux::file_has_content(out.filename(), ""s));
	BOOST_REQUIRE(testaux::file_has_content(err.filename(), ""s));
}


static const testaux::you_can_print_me<std::vector<const char *>> garbage_data[] = {
	{{"", "--ergo"}},
	{{"", "--echo", "--echo"}},
	{{"", "--echo=arg"}},
	{{"", "--lextest=arg"}},
	{{"", "--echo=arg", "-"}},
	{{"", "--output"}},
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
	testaux::temporary_file in{};
	testaux::temporary_file out{};
	testaux::temporary_file err{};
	auto fh_in = testaux::open_file(in.filename(), "rb");
	auto fh_out = testaux::open_file(out.filename(), "wb");
	auto fh_err = testaux::open_file(err.filename(), "wb");
	BOOST_REQUIRE_EXCEPTION(
		minijava::real_main(sample.value, fh_in.get(), fh_out.get(), fh_err.get()),
		std::exception,
		[](auto&& e){ return e.what()[0] != '\0'; }
	);
	BOOST_REQUIRE(testaux::file_has_content(out.filename(), ""s));
	BOOST_REQUIRE(testaux::file_has_content(err.filename(), ""s));
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
	testaux::make_random_string(1000000),
};

BOOST_DATA_TEST_CASE(echo_implicit_stdin_to_implicit_stdout, echo_data)
{
	using namespace std::string_literals;
	testaux::temporary_file in{sample};
	testaux::temporary_file out{};
	testaux::temporary_file err{};
	auto fh_in = testaux::open_file(in.filename(), "rb");
	auto fh_out = testaux::open_file(out.filename(), "wb");
	auto fh_err = testaux::open_file(err.filename(), "wb");
	minijava::real_main({"", "--echo"}, fh_in.get(), fh_out.get(), fh_err.get());
	BOOST_REQUIRE(testaux::file_has_content(out.filename(), sample));
	BOOST_REQUIRE(testaux::file_has_content(err.filename(), ""s));
}


BOOST_DATA_TEST_CASE(echo_implicit_stdin_to_explpcit_stdout, echo_data)
{
	using namespace std::string_literals;
	testaux::temporary_file in{sample};
	testaux::temporary_file out{};
	testaux::temporary_file err{};
	auto fh_in = testaux::open_file(in.filename(), "rb");
	auto fh_out = testaux::open_file(out.filename(), "wb");
	auto fh_err = testaux::open_file(err.filename(), "wb");
	minijava::real_main(
		{"", "--output", "-", "--echo"}, fh_in.get(), fh_out.get(), fh_err.get()
	);
	BOOST_REQUIRE(testaux::file_has_content(out.filename(), sample));
	BOOST_REQUIRE(testaux::file_has_content(err.filename(), ""s));
}


BOOST_DATA_TEST_CASE(echo_implicit_stdin_to_file, echo_data)
{
	using namespace std::string_literals;
	testaux::temporary_file in{sample};
	testaux::temporary_file out{};
	testaux::temporary_file err{};
	testaux::temporary_file dest{};
	auto fh_in = testaux::open_file(in.filename(), "rb");
	auto fh_out = testaux::open_file(out.filename(), "wb");
	auto fh_err = testaux::open_file(err.filename(), "wb");
	minijava::real_main(
		{"", "--output", dest.filename().c_str(), "--echo"},
		fh_in.get(), fh_out.get(), fh_err.get()
	);
	BOOST_REQUIRE(testaux::file_has_content(out.filename(), ""s));
	BOOST_REQUIRE(testaux::file_has_content(err.filename(), ""s));
	BOOST_REQUIRE(testaux::file_has_content(dest.filename(), sample));
}


BOOST_DATA_TEST_CASE(echo_explicit_stdin_to_implicit_stdout, echo_data)
{
	using namespace std::string_literals;
	testaux::temporary_file in{sample};
	testaux::temporary_file out{};
	testaux::temporary_file err{};
	auto fh_in = testaux::open_file(in.filename(), "rb");
	auto fh_out = testaux::open_file(out.filename(), "wb");
	auto fh_err = testaux::open_file(err.filename(), "wb");
	minijava::real_main({"", "--echo", "-"}, fh_in.get(), fh_out.get(), fh_err.get());
	BOOST_REQUIRE(testaux::file_has_content(out.filename(), sample));
	BOOST_REQUIRE(testaux::file_has_content(err.filename(), ""s));
}


BOOST_DATA_TEST_CASE(echo_explicit_stdin_to_explicit_stdout, echo_data)
{
	using namespace std::string_literals;
	testaux::temporary_file in{sample};
	testaux::temporary_file out{};
	testaux::temporary_file err{};
	auto fh_in = testaux::open_file(in.filename(), "rb");
	auto fh_out = testaux::open_file(out.filename(), "wb");
	auto fh_err = testaux::open_file(err.filename(), "wb");
	minijava::real_main(
		{"", "--output", "-", "--echo", "-"},
		fh_in.get(), fh_out.get(), fh_err.get()
	);
	BOOST_REQUIRE(testaux::file_has_content(out.filename(), sample));
	BOOST_REQUIRE(testaux::file_has_content(err.filename(), ""s));
}


BOOST_DATA_TEST_CASE(echo_explicit_stdin_to_file, echo_data)
{
	using namespace std::string_literals;
	testaux::temporary_file dest{};
	testaux::temporary_file in{sample};
	testaux::temporary_file out{};
	testaux::temporary_file err{};
	auto fh_in = testaux::open_file(in.filename(), "rb");
	auto fh_out = testaux::open_file(out.filename(), "wb");
	auto fh_err = testaux::open_file(err.filename(), "wb");
	minijava::real_main(
		{"", "--output", dest.filename().c_str(), "--echo", "-"},
		fh_in.get(), fh_out.get(), fh_err.get()
	);
	BOOST_REQUIRE(testaux::file_has_content(out.filename(), ""s));
	BOOST_REQUIRE(testaux::file_has_content(err.filename(), ""s));
	BOOST_REQUIRE(testaux::file_has_content(dest.filename(), sample));
}


BOOST_DATA_TEST_CASE(echo_file_to_implicit_stdout, echo_data)
{
	using namespace std::string_literals;
	testaux::temporary_file src{sample};
	testaux::temporary_file in{};
	testaux::temporary_file out{};
	testaux::temporary_file err{};
	auto fh_in = testaux::open_file(in.filename(), "rb");
	auto fh_out = testaux::open_file(out.filename(), "wb");
	auto fh_err = testaux::open_file(err.filename(), "wb");
	minijava::real_main(
		{"", "--echo", src.filename().c_str()},
		fh_in.get(), fh_out.get(), fh_err.get()
	);
	BOOST_REQUIRE(testaux::file_has_content(out.filename(), sample));
	BOOST_REQUIRE(testaux::file_has_content(err.filename(), ""s));
}


BOOST_DATA_TEST_CASE(echo_file_to_explicit_stdout, echo_data)
{
	using namespace std::string_literals;
	testaux::temporary_file src{sample};
	testaux::temporary_file in{};
	testaux::temporary_file out{};
	testaux::temporary_file err{};
	auto fh_in = testaux::open_file(in.filename(), "rb");
	auto fh_out = testaux::open_file(out.filename(), "wb");
	auto fh_err = testaux::open_file(err.filename(), "wb");
	minijava::real_main(
		{"", "--echo", src.filename().c_str(), "--output", "-"},
		fh_in.get(), fh_out.get(), fh_err.get()
	);
	BOOST_REQUIRE(testaux::file_has_content(out.filename(), sample));
	BOOST_REQUIRE(testaux::file_has_content(err.filename(), ""s));
}


BOOST_DATA_TEST_CASE(echo_file_to_file, echo_data)
{
	using namespace std::string_literals;
	testaux::temporary_file src{sample};
	testaux::temporary_file dst{};
	testaux::temporary_file in{};
	testaux::temporary_file out{};
	testaux::temporary_file err{};
	auto fh_in = testaux::open_file(in.filename(), "rb");
	auto fh_out = testaux::open_file(out.filename(), "wb");
	auto fh_err = testaux::open_file(err.filename(), "wb");
	minijava::real_main(
		{"", "--echo", src.filename().c_str(), "--output", dst.filename().c_str()},
		fh_in.get(), fh_out.get(), fh_err.get()
	);
	BOOST_REQUIRE(testaux::file_has_content(out.filename(), ""s));
	BOOST_REQUIRE(testaux::file_has_content(err.filename(), ""s));
	BOOST_REQUIRE(testaux::file_has_content(dst.filename(), sample));
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
	testaux::temporary_file in{valid_program_data};
	testaux::temporary_file out{};
	testaux::temporary_file err{};
	auto fh_in = testaux::open_file(in.filename(), "rb");
	auto fh_out = testaux::open_file(out.filename(), "wb");
	auto fh_err = testaux::open_file(err.filename(), "wb");
	minijava::real_main({"", sample.c_str()}, fh_in.get(), fh_out.get(), fh_err.get());
	BOOST_REQUIRE(not testaux::file_has_content(out.filename(), ""s));
	BOOST_REQUIRE(testaux::file_has_content(err.filename(), ""s));
}


BOOST_DATA_TEST_CASE(if_file_is_not_readable_all_actions_throw_and_output_nothing,
                     all_action_options)
{
	using namespace std::string_literals;
	const auto filename = testaux::temporary_file{}.filename();
	testaux::temporary_file in{};
	testaux::temporary_file out{};
	testaux::temporary_file err{};
	auto fh_in = testaux::open_file(in.filename(), "rb");
	auto fh_out = testaux::open_file(out.filename(), "wb");
	auto fh_err = testaux::open_file(err.filename(), "wb");
	BOOST_REQUIRE_THROW(
		minijava::real_main(
			{"", sample.c_str(), filename.c_str()},
			fh_in.get(), fh_out.get(), fh_err.get()
		),
		std::exception
	);
	BOOST_REQUIRE(testaux::file_has_content(out.filename(), ""s));
	BOOST_REQUIRE(testaux::file_has_content(err.filename(), ""s));
}


BOOST_DATA_TEST_CASE(if_stdin_is_not_readable_all_actions_throw_and_output_nothing,
                     all_action_options)
{
	// TODO: I don't know a portable reliable way to make a FILE pointer
	//       unreadable without invoking UB.
	(void) sample;
}


BOOST_DATA_TEST_CASE(if_stdout_is_not_writeable_all_actions_throw,
                     all_action_options)
{
	if (have_dev_full()) {
		using namespace std::string_literals;
		testaux::temporary_file in{valid_program_data};
		testaux::temporary_file err{};
		auto fh_in = testaux::open_file(in.filename(), "rb");
		auto fh_out = testaux::open_file("/dev/full", "wb");
		auto fh_err = testaux::open_file(err.filename(), "wb");
		BOOST_REQUIRE_THROW(
			minijava::real_main({"", sample.c_str()}, fh_in.get(), fh_out.get(), fh_err.get()),
			std::exception
		);
		BOOST_REQUIRE(testaux::file_has_content(err.filename(), ""s));
	}
}


BOOST_AUTO_TEST_CASE(lextest_for_valid_token_sequence_produces_correct_output)
{
	using namespace std::string_literals;
	testaux::temporary_file in{"42 abstract classes throw 1 + 3 mice."};
	testaux::temporary_file out{};
	testaux::temporary_file err{};
	auto fh_in = testaux::open_file(in.filename(), "rb");
	auto fh_out = testaux::open_file(out.filename(), "wb");
	auto fh_err = testaux::open_file(err.filename(), "wb");
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
	minijava::real_main({"", "--lextest"}, fh_in.get(), fh_out.get(), fh_err.get());
	BOOST_REQUIRE(testaux::file_has_content(out.filename(), expected_output));
	BOOST_REQUIRE(testaux::file_has_content(err.filename(), ""s));
}


BOOST_AUTO_TEST_CASE(lextest_for_invalid_token_sequence_throws_exception)
{
	using namespace std::string_literals;
	testaux::temporary_file in{"int nan = #@?/!;*"};
	testaux::temporary_file out{};
	testaux::temporary_file err{};
	auto fh_in = testaux::open_file(in.filename(), "rb");
	auto fh_out = testaux::open_file(out.filename(), "wb");
	auto fh_err = testaux::open_file(err.filename(), "wb");
	const auto expected_output = ""s
		+ "int\n"
		+ "identifier nan\n"
		+ "=\n";
	BOOST_REQUIRE_THROW(
		minijava::real_main({"", "--lextest"}, fh_in.get(), fh_out.get(), fh_err.get()),
		std::exception
	);
	BOOST_REQUIRE(testaux::file_has_content(out.filename(), expected_output));
	BOOST_REQUIRE(testaux::file_has_content(err.filename(), ""s));
}


// official example: http://pp.info.uni-karlsruhe.de/lehre/WS201617/compprakt/intern/Prog1.java
static const auto official_lexer_test = R"java(
/**
 * A classic class
 * @author Beate Best
 */
class classic {
	public int method(int arg) {
		int res = arg+42;
		res >>= 4;
		return res;
	}
}
)java";

// official example: http://pp.info.uni-karlsruhe.de/lehre/WS201617/compprakt/intern/Prog1.java.lex
static const auto official_lexer_test_result = R"lex(class
identifier classic
{
public
int
identifier method
(
int
identifier arg
)
{
int
identifier res
=
identifier arg
+
integer literal 42
;
identifier res
>>=
integer literal 4
;
return
identifier res
;
}
}
EOF
)lex";

BOOST_AUTO_TEST_CASE(lextest_passes_example_test)
{
	using namespace std::string_literals;
	testaux::temporary_file in{official_lexer_test};
	testaux::temporary_file out{};
	testaux::temporary_file err{};
	auto fh_in = testaux::open_file(in.filename(), "rb");
	auto fh_out = testaux::open_file(out.filename(), "wb");
	auto fh_err = testaux::open_file(err.filename(), "wb");
	minijava::real_main({"", "--lextest"}, fh_in.get(), fh_out.get(), fh_err.get());
	BOOST_REQUIRE(testaux::file_has_content(out.filename(), official_lexer_test_result));
	BOOST_REQUIRE(testaux::file_has_content(err.filename(), ""s));
}


BOOST_AUTO_TEST_CASE(lextest_does_not_eat_null_bytes)
{
	using namespace std::string_literals;
	testaux::temporary_file in{"I /* \0 */ am a sentence."s};
	testaux::temporary_file out{};
	testaux::temporary_file err{};
	auto fh_in = testaux::open_file(in.filename(), "rb");
	auto fh_out = testaux::open_file(out.filename(), "wb");
	auto fh_err = testaux::open_file(err.filename(), "wb");
	const auto expected_output = ""s
		+ "identifier I\n"
		+ "identifier am\n"
		+ "identifier a\n"
		+ "identifier sentence\n"
		+ ".\n"
		+ "EOF\n";
	minijava::real_main({"", "--lextest"}, fh_in.get(), fh_out.get(), fh_err.get());
	BOOST_REQUIRE(testaux::file_has_content(out.filename(), expected_output));
	BOOST_REQUIRE(testaux::file_has_content(err.filename(), ""s));
}


BOOST_AUTO_TEST_CASE(lextest_does_not_eat_null_bytes_on_error)
{
	using namespace std::string_literals;
	testaux::temporary_file in{"I \0 am an error."s};
	testaux::temporary_file out{};
	testaux::temporary_file err{};
	auto fh_in = testaux::open_file(in.filename(), "rb");
	auto fh_out = testaux::open_file(out.filename(), "wb");
	auto fh_err = testaux::open_file(err.filename(), "wb");
	const auto expected_output = "identifier I\n"s;
	BOOST_REQUIRE_THROW(
		minijava::real_main({"", "--lextest"}, fh_in.get(), fh_out.get(), fh_err.get()),
		std::exception
	);
	BOOST_REQUIRE(testaux::file_has_content(out.filename(), expected_output));
	BOOST_REQUIRE(testaux::file_has_content(err.filename(), ""s));
}


BOOST_AUTO_TEST_CASE(parsetest_valid_input)
{
	using namespace std::string_literals;
	testaux::temporary_file in{valid_program_data};
	testaux::temporary_file out{};
	testaux::temporary_file err{};
	auto fh_in = testaux::open_file(in.filename(), "rb");
	auto fh_out = testaux::open_file(out.filename(), "wb");
	auto fh_err = testaux::open_file(err.filename(), "wb");
	minijava::real_main({"", "--parsetest"}, fh_in.get(), fh_out.get(), fh_err.get());
	BOOST_REQUIRE(testaux::file_has_content(out.filename(), ""s));
	BOOST_REQUIRE(testaux::file_has_content(err.filename(), ""s));
}


BOOST_AUTO_TEST_CASE(parsetest_invalid_input)
{
	using namespace std::string_literals;
	testaux::temporary_file in{"class Foo { public static main(String[] args) {} }"};
	testaux::temporary_file out{};
	testaux::temporary_file err{};
	auto fh_in = testaux::open_file(in.filename(), "rb");
	auto fh_out = testaux::open_file(out.filename(), "wb");
	auto fh_err = testaux::open_file(err.filename(), "wb");
	BOOST_REQUIRE_THROW(
		minijava::real_main({"", "--parsetest"}, fh_in.get(), fh_out.get(), fh_err.get()),
		minijava::syntax_error
	);
	BOOST_REQUIRE(testaux::file_has_content(out.filename(), ""s));
	BOOST_REQUIRE(testaux::file_has_content(err.filename(), ""s));
}


// official example: https://pp.info.uni-karlsruhe.de/lehre/WS201617/compprakt/intern/example.input
static const auto official_pretty_printer_test = R"java(
class HelloWorld
{
	public int c;
	public boolean[] array;
	public static /* blabla */ void main(String[] args)
	{ System.out.println( (43110 + 0) );
	boolean b = true && (!false);
	if (23+19 == (42+0)*1)
		b = (0 < 1);
		else if (!array[2+2]) {
			int x = 0;;
			x = x+1;
		} else {
			new HelloWorld().bar(42+0*1, -1);
		}
	}
	public int bar(int a, int b) { return c = (a+b); }
}
)java";

// official example: https://pp.info.uni-karlsruhe.de/lehre/WS201617/compprakt/intern/example.output
static const auto official_pretty_printer_test_result = R"java(class HelloWorld {
	public int bar(int a, int b) {
		return c = (a + b);
	}
	public static void main(String[] args) {
		(System.out).println(43110 + 0);
		boolean b = true && (!false);
		if ((23 + 19) == ((42 + 0) * 1))
			b = (0 < 1);
		else if (!(array[2 + 2])) {
			int x = 0;
			x = (x + 1);
		} else {
			(new HelloWorld()).bar(42 + (0 * 1), -1);
		}
	}
	public boolean[] array;
	public int c;
}
)java";

BOOST_AUTO_TEST_CASE(print_ast_passes_example_test)
{
	using namespace std::string_literals;
	testaux::temporary_file in{official_pretty_printer_test};
	testaux::temporary_file out{};
	testaux::temporary_file err{};
	auto fh_in = testaux::open_file(in.filename(), "rb");
	auto fh_out = testaux::open_file(out.filename(), "wb");
	auto fh_err = testaux::open_file(err.filename(), "wb");
	minijava::real_main({"", "--print-ast"}, fh_in.get(), fh_out.get(), fh_err.get());
	BOOST_REQUIRE(testaux::file_has_content(out.filename(), official_pretty_printer_test_result));
	BOOST_REQUIRE(testaux::file_has_content(err.filename(), ""s));
}
