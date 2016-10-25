#include "lexer/keyword.hpp"

#include <sstream>

#define BOOST_TEST_MODULE  lexer_keyword
#include <boost/test/unit_test.hpp>


// TODO @Moritz Klammler: Come up with a smarter test using fancy_name.

BOOST_AUTO_TEST_CASE(lookup_keyword)
{
	BOOST_REQUIRE_EQUAL(minijava::token_type::kw_if, minijava::classify_word("if"));
	BOOST_REQUIRE_EQUAL(minijava::token_type::kw_else, minijava::classify_word("else"));
}
