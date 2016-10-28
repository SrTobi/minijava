#include "symbol.hpp"

#include <cstring>
#include <sstream>
#include <string>

#define BOOST_TEST_MODULE  symbol
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(same_pointers_compare_equal)
{
	minijava::static_symbol_pool p("You're innocent when you dream");
	const auto s1 = p.get();
	const auto s2 = p.get();
	BOOST_REQUIRE(s1 == s2);
	BOOST_REQUIRE(not (s1 != s2));
}

BOOST_AUTO_TEST_CASE(stream_insertion)
{
	using namespace std::string_literals;
    const auto text = "We laughed, my friends and I"s;
	minijava::static_symbol_pool pool(text);
	const auto s = pool.get();
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
    minijava::static_symbol_pool pool(test_string);
    minijava::symbol pool_symbol = pool.get();
    std::size_t actual_hash = poolstr_hasher(pool_symbol);

    BOOST_REQUIRE_EQUAL(actual_hash, expected_hash);
}


BOOST_AUTO_TEST_CASE(two_normalized_symbols_with_same_origin_only_equal_each_other)
{
    std::string test_string = "some test symbol looking good";
    minijava::static_symbol_pool pool(test_string);
    minijava::static_symbol_pool not_pool("not");

    minijava::symbol not_test_symbol = not_pool.get();
    minijava::symbol first = pool.get();
    minijava::symbol second = pool.get();

    BOOST_REQUIRE_EQUAL(first, second);
    BOOST_REQUIRE_NE(first, not_test_symbol);
    BOOST_REQUIRE_NE(second, not_test_symbol);
}


BOOST_AUTO_TEST_CASE(copied_symbol_equals_original_symbol)
{
    minijava::static_symbol_pool pool("another great example of a symbol");

    minijava::symbol origin = pool.get();
    minijava::symbol copy = origin;

    BOOST_REQUIRE_EQUAL(origin, copy);
}

BOOST_AUTO_TEST_CASE(normalized_symbol_data_equals_origin)
{
    std::string origin = "pirate-strings do not like bananas";
    minijava::static_symbol_pool pool(origin);

    minijava::symbol normalized = pool.get();

    BOOST_REQUIRE_EQUAL(normalized.c_str(), origin.c_str());
}

BOOST_AUTO_TEST_CASE(normalized_symbol_size_equals_origin_size)
{
    std::string origin = "very original string is original";
    minijava::static_symbol_pool pool(origin);

    minijava::symbol normalized = pool.get();

    BOOST_REQUIRE_EQUAL(normalized.size(), origin.size());
    auto cstr_size = std::strlen(normalized.c_str());
    BOOST_REQUIRE_EQUAL(cstr_size, normalized.size());
}
