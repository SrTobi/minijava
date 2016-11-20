#include "lexer/token.hpp"

#include <cstddef>
#include <iterator>
#include <string>
#include <vector>

#define BOOST_TEST_MODULE  lexer_token
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "symbol/symbol.hpp"
#include "symbol/symbol_pool.hpp"

#include "testaux/token_string.hpp"


using testaux::id;
using testaux::lit;
using tt = minijava::token_type;

namespace /* anonymous */
{

	minijava::symbol_pool<> g_pool{};

	class token_pair
	{
	public:

		minijava::token first {minijava::token::create(tt::eof)};
		minijava::token second {minijava::token::create(tt::eof)};

		template <typename T1, typename T2>
		token_pair(T1&& arg1, T2&& arg2)
		{
			this->first = testaux::make_token(g_pool, std::forward<T1>(arg1));
			this->second = testaux::make_token(g_pool, std::forward<T2>(arg2));
		}

		friend std::ostream& operator<<(std::ostream& os, const token_pair& tp)
		{
			return os << "'" << tp.first << "' '" << tp.second << "'";
		}
	};


}


BOOST_AUTO_TEST_CASE(token_ctor_id)
{
	using namespace std::string_literals;
	auto pool = minijava::symbol_pool<>{};
	const auto text = "matchstick"s;
	const auto canon = pool.normalize(text);
	const auto tok = minijava::token::create_identifier(canon);
	BOOST_REQUIRE_EQUAL(tt::identifier, tok.type());
	BOOST_REQUIRE_EQUAL(canon, tok.lexval());
	BOOST_REQUIRE_EQUAL(true, tok.has_lexval());
	BOOST_REQUIRE_EQUAL(std::size_t{0}, tok.line());
	BOOST_REQUIRE_EQUAL(std::size_t{0}, tok.column());
}


BOOST_AUTO_TEST_CASE(token_ctor_integer_literal)
{
	using namespace std::string_literals;
	auto pool = minijava::symbol_pool<>{};
	const auto text = "42"s;
	const auto canon = pool.normalize(text);
	const auto tok = minijava::token::create_integer_literal(canon);
	BOOST_REQUIRE_EQUAL(tt::integer_literal, tok.type());
	BOOST_REQUIRE_EQUAL(canon, tok.lexval());
	BOOST_REQUIRE_EQUAL(true, tok.has_lexval());
	BOOST_REQUIRE_EQUAL(std::size_t{0}, tok.line());
	BOOST_REQUIRE_EQUAL(std::size_t{0}, tok.column());
}


static const auto monostate_token_data = [](){
	constexpr auto idcat = minijava::token_category::identifier;
	constexpr auto litcat = minijava::token_category::literal;
	auto vec = std::vector<tt>{};
	std::copy_if(
		std::begin(minijava::all_token_types()),
		std::end(minijava::all_token_types()),
		std::back_inserter(vec),
		[idcat,litcat](auto t){
			const auto cat = category(t);
			return ((cat != idcat) && (cat != litcat));
		});
	return vec;
}();

BOOST_DATA_TEST_CASE(token_ctor_punct, monostate_token_data)
{
	const auto tok = minijava::token::create(sample);
	BOOST_REQUIRE_EQUAL(sample, tok.type());
	BOOST_REQUIRE_EQUAL(false, tok.has_lexval());
	BOOST_REQUIRE_EQUAL(std::size_t{0}, tok.line());
	BOOST_REQUIRE_EQUAL(std::size_t{0}, tok.column());
}


static const token_pair equal_data[] = {
	{id("foo"), id("foo")},
	{lit("100"), lit("100")},
	{tt::kw_assert, tt::kw_assert},
	{tt::bit_not, tt::bit_not},
	{tt::eof, tt::eof},
};

BOOST_DATA_TEST_CASE(tokens_that_compare_equal, equal_data)
{
	auto tok1 = sample.first;
	auto tok2 = sample.second;
	BOOST_REQUIRE_EQUAL(tok1, tok2);
	BOOST_REQUIRE_EQUAL(tok2, tok1);
	tok1.set_line(std::size_t{12});
	BOOST_REQUIRE_EQUAL(tok1, tok2);
	tok1.set_column(std::size_t{345});
	BOOST_REQUIRE_EQUAL(tok2, tok1);
	tok2.set_line(std::size_t{6});
	BOOST_REQUIRE_EQUAL(tok1, tok2);
	tok2.set_column(std::size_t{7});
	BOOST_REQUIRE_EQUAL(tok2, tok1);
}


static const token_pair not_equal_data[] = {
	{id("not"), id("equal")},
	{id("five"), lit("5")},
	{lit("12345"), lit("123456")},
	{id("and"), tt::logical_and},
	{tt::kw_for, id("ever")},
	{tt::kw_if, tt::kw_else},
	{tt::kw_goto, tt::plus},
	{tt::eof, tt::left_paren},
	{tt::eof, id("EOF")},
};

BOOST_DATA_TEST_CASE(tokens_that_compare_not_equal, not_equal_data)
{
	const auto tok1 = sample.first;
	const auto tok2 = sample.second;
	BOOST_REQUIRE_NE(tok1, tok2);
	BOOST_REQUIRE_NE(tok2, tok1);
}


BOOST_AUTO_TEST_CASE(identifiers_are_streamed_correctly)
{
	auto pool = minijava::symbol_pool<>{};
	const auto canon = pool.normalize("foo");
	const auto tok = minijava::token::create_identifier(canon);
	auto oss = std::ostringstream{};
	oss << tok;
	BOOST_REQUIRE_EQUAL("identifier foo", oss.str());
}


BOOST_AUTO_TEST_CASE(integer_literals_are_streamed_correctly)
{
	auto pool = minijava::symbol_pool<>{};
	const auto canon = pool.normalize("42");
	const auto tok = minijava::token::create_integer_literal(canon);
	auto oss = std::ostringstream{};
	oss << tok;
	BOOST_REQUIRE_EQUAL("integer literal 42", oss.str());
}


BOOST_AUTO_TEST_CASE(keywords_are_streamed_correctly)
{
	const auto tok = minijava::token::create(tt::kw_void);
	auto oss = std::ostringstream{};
	oss << tok;
	BOOST_REQUIRE_EQUAL("void", oss.str());
}


BOOST_AUTO_TEST_CASE(operators_are_streamed_correctly)
{
	const auto tok = minijava::token::create(tt::logical_and);
	auto oss = std::ostringstream{};
	oss << tok;
	BOOST_REQUIRE_EQUAL("&&", oss.str());
}


BOOST_AUTO_TEST_CASE(semicolon_is_streamed_correctly)
{
	const auto tok = minijava::token::create(tt::semicolon);
	auto oss = std::ostringstream{};
	oss << tok;
	BOOST_REQUIRE_EQUAL(";", oss.str());
}


BOOST_AUTO_TEST_CASE(eof_is_streamed_correctly)
{
	const auto tok = minijava::token::create(tt::eof);
	auto oss = std::ostringstream{};
	oss << tok;
	BOOST_REQUIRE_EQUAL("EOF", oss.str());
}
