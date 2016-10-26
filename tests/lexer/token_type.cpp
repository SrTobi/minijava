#include "lexer/token_type.hpp"

#include <cstddef>
#include <iterator>
#include <set>
#include <sstream>
#include <string>

#define BOOST_TEST_MODULE  lexer_token_type
#include <boost/test/unit_test.hpp>

#include "../testaux/testaux.hpp"


BOOST_AUTO_TEST_CASE(there_is_no_default_token_type)
{
	const auto tt = minijava::token_type{};
	const auto first = std::begin(minijava::all_token_types());
	const auto last = std::end(minijava::all_token_types());
	BOOST_REQUIRE(std::find(first, last, tt) == last);
}


BOOST_AUTO_TEST_CASE(fancy_name_returns_correct_string_for_valid_token_type)
{
	constexpr auto tt = minijava::token_type::kw_else;
	constexpr auto fancy = fancy_name(tt);
	static_assert(testaux::cx_strcmp("else", fancy) == 0, "");
}


BOOST_AUTO_TEST_CASE(fancy_name_returns_nullptr_for_invalid_token_type)
{
	constexpr auto tt = minijava::token_type{};
	constexpr auto fancy = fancy_name(tt);
	static_assert(fancy == nullptr, "");
}


BOOST_AUTO_TEST_CASE(streaming_a_valid_token_type_inserts_the_correct_string)
{
	const auto tok = minijava::token_type::kw_else;
	auto oss = std::ostringstream{};
	oss << tok;
	BOOST_REQUIRE(oss);
	BOOST_REQUIRE_EQUAL("else", oss.str());
}


BOOST_AUTO_TEST_CASE(streaming_an_invalid_token_type_inserts_some_string)
{
	using namespace std::string_literals;
	const auto tok = minijava::token_type{};
	auto oss = std::ostringstream{};
	oss << tok;
	BOOST_REQUIRE(oss);
	BOOST_REQUIRE_NE(""s, oss.str());
}


BOOST_AUTO_TEST_CASE(fancy_names_are_unique)
{
	auto names = std::set<std::string>{};
	const auto first = std::begin(minijava::all_token_types());
	const auto last = std::end(minijava::all_token_types());
	const auto fancy = [](auto tt){ return fancy_name(tt); };
	std::transform(first, last, std::inserter(names, names.cend()), fancy);
	const auto expected = static_cast<std::size_t>(std::distance(first, last));
	const auto actual = names.size();
	BOOST_REQUIRE_EQUAL(expected, actual);
}
