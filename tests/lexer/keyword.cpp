#include "lexer/keyword.hpp"

#include <sstream>

#define BOOST_TEST_MODULE  lexer_keyword
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_CASE(classify_word_after_name_is_identity_for_keywords)
{
	for (const auto tt : minijava::all_token_types()) {
		if (category(tt) == minijava::token_category::keyword) {
			BOOST_REQUIRE_EQUAL(tt, minijava::classify_word(name(tt)));
		}
	}
}


BOOST_AUTO_TEST_CASE(lookup_non_keyword)
{
	const auto tt = minijava::classify_word("foo");
	BOOST_REQUIRE_EQUAL(minijava::token_type::identifier, tt);
}
