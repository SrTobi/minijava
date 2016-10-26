#include "lexer/keyword.hpp"

#include <sstream>

#define BOOST_TEST_MODULE  lexer_keyword
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_CASE(lookup_keyword)
{
	for (const auto tt : minijava::all_token_types()) {
		if (category(tt) == minijava::token_category::keyword) {
			const auto name = fancy_name(tt);
			BOOST_REQUIRE_EQUAL(tt, minijava::classify_word(name));
		}
	}
}


BOOST_AUTO_TEST_CASE(lookup_non_keyword)
{
	const auto tt = minijava::classify_word("foo");
	BOOST_REQUIRE_EQUAL(minijava::token_type::identifier, tt);
}
