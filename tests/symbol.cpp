#include "symbol.hpp"
#include "symbol_pool.hpp"

#include <cstring>
#include <sstream>
#include <string>

#define BOOST_TEST_MODULE  symbol
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_CASE(symbols_have_the_same_size_as_pointers)
{
	static_assert(sizeof(minijava::symbol) == sizeof(void*), "");
}

BOOST_AUTO_TEST_CASE(same_pointers_compare_equal)
{
	minijava::symbol_pool<> pool;
	const auto p = "You're innocent when you dream";
	const auto s1 = pool.normalize(p);
	const auto s2 = pool.normalize(p);
	BOOST_REQUIRE(s1 == s2);
	BOOST_REQUIRE(not (s1 != s2));
}


BOOST_AUTO_TEST_CASE(different_pointers_from_different_pools_to_different_data_compare_unequal)
{
	minijava::symbol_pool<> pool1;
	minijava::symbol_pool<> pool2;
	const auto p1 = "The bats are in the belfry";
	const auto p2 = "The dew is on the moor";
	const auto s1 = pool1.normalize(p1);
	const auto s2 = pool2.normalize(p2);
	BOOST_REQUIRE(not (s1 == s2));
	BOOST_REQUIRE(s1 != s2);
}


BOOST_AUTO_TEST_CASE(different_pointers_from_different_pools_to_same_data_compare_unequal)
{
	minijava::symbol_pool<> pool1;
	minijava::symbol_pool<> pool2;
	using namespace std::string_literals;
	const auto text = "Running through a graveyard"s;
	const auto copy = text;
	BOOST_REQUIRE(text.c_str() != copy.c_str());  // self-test
	const auto s1 = pool1.normalize(text.c_str());
	const auto s2 = pool2.normalize(copy.c_str());
	BOOST_REQUIRE(not (s1 == s2));
	BOOST_REQUIRE(s1 != s2);
	BOOST_REQUIRE(std::strcmp(s1.c_str(), s2.c_str()) == 0);
}


BOOST_AUTO_TEST_CASE(stream_insertion)
{
	minijava::symbol_pool<> pool;
	using namespace std::string_literals;
	const auto text = "We laughed, my friends and I"s;
	const auto s = pool.normalize(text.c_str());
	auto oss = std::ostringstream{};
	oss << s;
	BOOST_REQUIRE_EQUAL(text, oss.str());
}
