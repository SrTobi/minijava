#include "lexer/token.hpp"

#include <string>

#define BOOST_TEST_MODULE  lexer_token
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "symbol.hpp"
#include "symbol_pool.hpp"


BOOST_AUTO_TEST_CASE(token_ctor_id)
{
	minijava::symbol_pool<> pool;
	using namespace std::string_literals;
	const auto name = "matchstick"s;
	const auto canonical = pool.normalize(name.c_str());
	const auto tok = minijava::token::create_identifier(canonical);
	BOOST_REQUIRE_EQUAL(minijava::token_type::identifier, tok.type());
	BOOST_REQUIRE_EQUAL(canonical, tok.name());
	BOOST_REQUIRE_EQUAL(std::size_t{0}, tok.line());
	BOOST_REQUIRE_EQUAL(std::size_t{0}, tok.column());
}


BOOST_AUTO_TEST_CASE(token_ctor_integer_literal)
{
	const auto value = std::uint32_t{42};
	const auto tok = minijava::token::create_integer_literal(value);
	BOOST_REQUIRE_EQUAL(minijava::token_type::integer_literal, tok.type());
	BOOST_REQUIRE_EQUAL(value, tok.value());
	BOOST_REQUIRE_EQUAL(std::size_t{0}, tok.line());
	BOOST_REQUIRE_EQUAL(std::size_t{0}, tok.column());
}


static minijava::token_type token_ctor_data[] = {
	minijava::token_type::kw_if,
	minijava::token_type::kw_else,
	minijava::token_type::left_paren,
	minijava::token_type::right_paren,
	minijava::token_type::eof,
};

BOOST_DATA_TEST_CASE(token_ctor_punct, token_ctor_data)
{
	const auto tok = minijava::token::create(sample);
	BOOST_REQUIRE_EQUAL(sample, tok.type());
	BOOST_REQUIRE_EQUAL(std::size_t{0}, tok.line());
	BOOST_REQUIRE_EQUAL(std::size_t{0}, tok.column());
}


BOOST_AUTO_TEST_CASE(identifiers_with_same_name_compare_equal)
{
	minijava::symbol_pool<> pool;
	using namespace std::string_literals;
	const auto name = "matchstick"s;
	const auto canonical = pool.normalize(name.c_str());
	const auto tok1 = minijava::token::create_identifier(canonical);
	const auto tok2 = minijava::token::create_identifier(canonical);
	BOOST_REQUIRE_EQUAL(tok1, tok2);
}


BOOST_AUTO_TEST_CASE(identifiers_with_same_name_compare_equal_even_if_source_location_differs)
{
	minijava::symbol_pool<> pool;
	using namespace std::string_literals;
	const auto name = "matchstick"s;
	const auto canonical = pool.normalize(name.c_str());
	auto tok1 = minijava::token::create_identifier(canonical);
	auto tok2 = minijava::token::create_identifier(canonical);
	tok1.set_line(23);
	tok2.set_column(10);
	BOOST_REQUIRE_EQUAL(tok1, tok2);
}


BOOST_AUTO_TEST_CASE(identifiers_with_different_name_compare_not_equal)
{
	minijava::symbol_pool<> pool;
	using namespace std::string_literals;
	const auto name1 = "apple"s;
	const auto name2 = "banana"s;
	const auto canon1 = pool.normalize(name1.c_str());
	const auto canon2 = pool.normalize(name2.c_str());
	const auto tok1 = minijava::token::create_identifier(canon1);
	const auto tok2 = minijava::token::create_identifier(canon2);
	BOOST_REQUIRE_NE(tok1, tok2);
}


// TODO @Moritz Klammler: Add comprehensive unit tests for equality comparison.
