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

BOOST_AUTO_TEST_CASE(hash_of_symbol_equals_hash_of_std_string)
{
    std::hash<std::string> str_hasher;
    std::string test_string = "symbols love to make love";
    std::size_t expected_hash = str_hasher(test_string);

    std::hash<minijava::symbol> poolstr_hasher;
    minijava::symbol_pool<> pool;
    minijava::symbol pool_symbol = pool.normalize(test_string);
    std::size_t actual_hash = poolstr_hasher(pool_symbol);

    BOOST_REQUIRE_EQUAL(actual_hash, expected_hash);
}


BOOST_AUTO_TEST_CASE(two_normalized_symbols_with_same_origin_only_equal_each_other)
{
    minijava::symbol_pool<> pool;
    std::string test_string = "some test symbol looking good";

    minijava::symbol not_test_symbol = pool.normalize("not");
    minijava::symbol first = pool.normalize(test_string);
    minijava::symbol second = pool.normalize(test_string);

    BOOST_REQUIRE_EQUAL(first, second);
    BOOST_REQUIRE_NE(first, not_test_symbol);
    BOOST_REQUIRE_NE(second, not_test_symbol);
}


BOOST_AUTO_TEST_CASE(copied_symbol_equals_original_symbol)
{
    minijava::symbol_pool<> pool;

    minijava::symbol origin = pool.normalize("another great example of a symbol");
    minijava::symbol copy = origin;

    BOOST_REQUIRE_EQUAL(origin, copy);
}

BOOST_AUTO_TEST_CASE(normalized_symbol_data_equals_origin)
{
    minijava::symbol_pool<> pool;
    std::string origin = "pirate-strings do not like bananas";

    minijava::symbol normalized = pool.normalize(origin);

    BOOST_REQUIRE_EQUAL(normalized.c_str(), origin.c_str());
}

BOOST_AUTO_TEST_CASE(normalized_symbol_size_equals_origin_size)
{
    minijava::symbol_pool<> pool;
    std::string origin = "very original string is original";

    minijava::symbol normalized = pool.normalize(origin);

    BOOST_REQUIRE_EQUAL(normalized.size(), origin.size());
    auto cstr_size = std::strlen(normalized.c_str());
    BOOST_REQUIRE_EQUAL(cstr_size, normalized.size());
}
