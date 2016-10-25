#include "string.hpp"

#include <cstring>
#include <sstream>
#include <string>

#define BOOST_TEST_MODULE  string
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_CASE(c_str_returns_correct_pointer)
{
	constexpr auto p = "The hills are soft and green";
	constexpr auto s = minijava::string::create_from_canonical_pointer(p);
	static_assert(p == s.c_str(), "");
}


BOOST_AUTO_TEST_CASE(same_pointers_compare_equal)
{
	const auto p = "You're innocent when you dream";
	const auto s1 = minijava::string::create_from_canonical_pointer(p);
	const auto s2 = minijava::string::create_from_canonical_pointer(p);
	BOOST_REQUIRE(s1 == s2);
	BOOST_REQUIRE(not (s1 != s2));
}


BOOST_AUTO_TEST_CASE(different_pointers_to_different_data_compare_unequal)
{
	const auto p1 = "The bats are in the belfry";
	const auto p2 = "The dew is on the moor";
	const auto s1 = minijava::string::create_from_canonical_pointer(p1);
	const auto s2 = minijava::string::create_from_canonical_pointer(p2);
	BOOST_REQUIRE(not (s1 == s2));
	BOOST_REQUIRE(s1 != s2);
}


BOOST_AUTO_TEST_CASE(different_pointers_to_same_data_compare_unequal)
{
	using namespace std::string_literals;
	const auto text = "Running through a graveyard"s;
	const auto copy = text;
	BOOST_REQUIRE(text.c_str() != copy.c_str());  // self-test
	const auto s1 = minijava::string::create_from_canonical_pointer(text.c_str());
	const auto s2 = minijava::string::create_from_canonical_pointer(copy.c_str());
	BOOST_REQUIRE(not (s1 == s2));
	BOOST_REQUIRE(s1 != s2);
	BOOST_REQUIRE(std::strcmp(s1.c_str(), s2.c_str()) == 0);
}


BOOST_AUTO_TEST_CASE(stream_insertion)
{
	using namespace std::string_literals;
	const auto text = "We laughed, my friends and I"s;
	const auto s = minijava::string::create_from_canonical_pointer(text.c_str());
	auto oss = std::ostringstream{};
	oss << s;
	BOOST_REQUIRE_EQUAL(text, oss.str());
}
