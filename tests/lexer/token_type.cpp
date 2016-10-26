#include "lexer/token_type.hpp"

#include <sstream>

#define BOOST_TEST_MODULE  lexer_token_type
#include <boost/test/unit_test.hpp>

#include "../testaux/cx_string.hpp"


BOOST_AUTO_TEST_CASE(fancy_name_valid)
{
	constexpr auto tt = minijava::token_type::kw_else;
	constexpr auto fancy = fancy_name(tt);
	static_assert(testaux::cx_strcmp("else", fancy) == 0, "");
}


BOOST_AUTO_TEST_CASE(fancy_name_invalid)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
	constexpr auto tt = static_cast<minijava::token_type>(0xdead);
#pragma GCC diagnostic pop
	constexpr auto fancy = fancy_name(tt);
	static_assert(fancy == nullptr, "");
}


BOOST_AUTO_TEST_CASE(stream_fancy_valid)
{
	auto oss = std::ostringstream{};
	oss << minijava::token_type::kw_else;
	BOOST_REQUIRE_EQUAL("else", oss.str());
}
