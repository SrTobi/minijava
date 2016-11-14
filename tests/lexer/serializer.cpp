#include "lexer/serializer.hpp"

#include <sstream>
#include <string>
#include <utility>
#include <vector>

#define BOOST_TEST_MODULE  lexer_serializer
#include <boost/test/unit_test.hpp>

#include "lexer/token.hpp"
#include "symbol_pool.hpp"

#include "testaux/token_string.hpp"

using namespace std::string_literals;

using testaux::id;
using testaux::lit;
using tt = minijava::token_type;


namespace /* anonymous */
{

    static minijava::symbol_pool<> g_pool{};

	class token_sequence
	{
	public:

		template <typename... ArgTs>
		token_sequence(ArgTs&&... args)
		{
			_tokens = {testaux::make_token(g_pool, std::forward<ArgTs>(args))...};
		}

		auto begin() const noexcept
		{
			return _tokens.begin();
		}

		auto end() const noexcept
		{
			return _tokens.end();
		}

	private:

		std::vector<minijava::token> _tokens{};

	};

}  // namespace /* anonymous */


BOOST_AUTO_TEST_CASE(empty_sequence)
{
	const auto tokens = token_sequence{};
	auto oss = std::ostringstream{};
	minijava::pretty_print(oss, std::begin(tokens), std::end(tokens));
	BOOST_REQUIRE_EQUAL(""s, oss.str());
}


BOOST_AUTO_TEST_CASE(sequence_with_eof)
{
	const auto tokens = token_sequence{tt::eof, tt::eof, tt::eof};
	auto oss = std::ostringstream{};
	minijava::pretty_print(oss, std::begin(tokens), std::end(tokens));
	BOOST_REQUIRE_EQUAL(""s, oss.str());
}


BOOST_AUTO_TEST_CASE(basic_expressions)
{
	const auto tokens = token_sequence{
		id("foo"), tt::assign, lit("1"), tt::multiply, tt::left_paren,
		id("bar"), tt::left_bracket, lit("0"), tt::right_bracket, tt::plus,
		lit("42"), tt::right_paren,
	};
	auto oss = std::ostringstream{};
	minijava::pretty_print(oss, std::begin(tokens), std::end(tokens));
	BOOST_REQUIRE_EQUAL("foo = 1 * (bar[0] + 42)"s, oss.str());
}


BOOST_AUTO_TEST_CASE(basic_indentation)
{
	const auto tokens = token_sequence{
		tt::kw_class, id("MyClass"), tt::left_brace, tt::kw_public, tt::kw_int,
		id("bar"), tt::semicolon, tt::kw_public, tt::kw_int, id("foo"),
		tt::left_paren, tt::right_paren, tt::left_brace, tt::kw_return,
		lit("17"), tt::semicolon, tt::right_brace, tt::right_brace,
	};
	const auto expected = ""s
		+ "class MyClass {\n"
		+ "\tpublic int bar;\n"
		+ "\tpublic int foo() {\n"
		+ "\t\treturn 17;\n"
		+ "\t}\n"
		+ "}";
	auto oss = std::ostringstream{};
	minijava::pretty_print(oss, std::begin(tokens), std::end(tokens));
	BOOST_REQUIRE_EQUAL(expected, oss.str());
}
