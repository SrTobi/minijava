#include "symbol/symbol.hpp"

#include "testaux/random_tokens.hpp"
#include "testaux/static_symbol_pool.hpp"

#include <algorithm>
#include <cstring>
#include <random>
#include <sstream>
#include <string>

#define BOOST_TEST_MODULE  symbol_symbol
#include <boost/test/unit_test.hpp>

#include "symbol/symbol_pool.hpp"


BOOST_AUTO_TEST_CASE(same_pointers_compare_equal)
{
	testaux::static_symbol_pool p("You're innocent when you dream");
	const auto s1 = p.get();
	const auto s2 = p.get();
	BOOST_REQUIRE(s1 == s2);
	BOOST_REQUIRE(not (s1 != s2));
}


BOOST_AUTO_TEST_CASE(stream_insertion)
{
	using namespace std::string_literals;
    const auto text = "We laughed, my friends and I"s;
	testaux::static_symbol_pool pool(text);
	const auto s = pool.get();
	auto oss = std::ostringstream{};
	oss << s;
	BOOST_REQUIRE_EQUAL(text, oss.str());
}


BOOST_AUTO_TEST_CASE(test_empty_symbol)
{
    std::string empty = "";
    std::string origin = "non empty";
    testaux::static_symbol_pool empty_pool(empty);
    testaux::static_symbol_pool full_pool(origin);

    minijava::symbol stdctor_sym;
    minijava::symbol empty_normalized = empty_pool.get();
    minijava::symbol full_normalized = full_pool.get();

    // check compare
    BOOST_REQUIRE_EQUAL(stdctor_sym, empty_normalized);
    BOOST_REQUIRE_NE(stdctor_sym, full_normalized);
    BOOST_REQUIRE_NE(empty_normalized, full_normalized);

    // check size
    BOOST_REQUIRE_EQUAL(stdctor_sym.size(), empty.size());
    BOOST_REQUIRE_EQUAL(empty_normalized.size(), empty.size());

    // check content
    BOOST_REQUIRE_EQUAL(stdctor_sym.c_str(), empty.c_str());
    BOOST_REQUIRE_EQUAL(empty_normalized.c_str(), empty.c_str());

    // check empty
    BOOST_REQUIRE(stdctor_sym.empty());
    BOOST_REQUIRE(empty_normalized.empty());

    // check hash
    std::hash<minijava::symbol> hash_fn;
    BOOST_REQUIRE_EQUAL(hash_fn(stdctor_sym), hash_fn(empty_normalized));
    BOOST_REQUIRE_EQUAL(0, hash_fn(stdctor_sym));
    BOOST_REQUIRE_EQUAL(0, stdctor_sym.hash());
    BOOST_REQUIRE_EQUAL(0, empty_normalized.hash());
    BOOST_REQUIRE_EQUAL(full_normalized.hash(), hash_fn(full_normalized));
}


BOOST_AUTO_TEST_CASE(two_normalized_symbols_with_same_origin_only_equal_each_other)
{
    std::string test_string = "some test symbol looking good";
    testaux::static_symbol_pool pool(test_string);
    testaux::static_symbol_pool not_pool("not");

    minijava::symbol not_test_symbol = not_pool.get();
    minijava::symbol first = pool.get();
    minijava::symbol second = pool.get();

    BOOST_REQUIRE_EQUAL(first, second);
    BOOST_REQUIRE_NE(first, not_test_symbol);
    BOOST_REQUIRE_NE(second, not_test_symbol);
}


BOOST_AUTO_TEST_CASE(copied_symbol_equals_original_symbol)
{
    testaux::static_symbol_pool pool("another great example of a symbol");

    minijava::symbol origin = pool.get();
    minijava::symbol copy = origin;

    BOOST_REQUIRE_EQUAL(origin, copy);
    BOOST_REQUIRE_EQUAL(origin.hash(), copy.hash());
}

BOOST_AUTO_TEST_CASE(normalized_symbol_cstr_equals_origin)
{
    std::string origin = "pirate-strings do not like bananas";
    testaux::static_symbol_pool pool(origin);

    minijava::symbol normalized = pool.get();

    BOOST_REQUIRE_EQUAL(origin, normalized.c_str());
}

BOOST_AUTO_TEST_CASE(normalized_symbol_size_and_lenght_equals_origin_size)
{
    std::string origin = "very original string is original";
    testaux::static_symbol_pool pool(origin);

    minijava::symbol normalized = pool.get();

    BOOST_REQUIRE_EQUAL(normalized.size(), origin.size());
    BOOST_REQUIRE_EQUAL(normalized.length(), origin.length());
    auto cstr_size = std::strlen(normalized.c_str());
    BOOST_REQUIRE_EQUAL(cstr_size, normalized.size());
}

BOOST_AUTO_TEST_CASE(normalized_symbol_data_equals_origin)
{
    std::string origin = "the question is sometimes the answer";
    testaux::static_symbol_pool pool(origin);

    minijava::symbol normalized = pool.get();

    BOOST_REQUIRE_EQUAL(normalized.data(), origin.data());
}

BOOST_AUTO_TEST_CASE(symbol_forward_directed_iterators_behave_all_the_same)
{
    std::string origin = "it is worth an iterator";
    testaux::static_symbol_pool pool(origin);

    minijava::symbol normalized = pool.get();

    std::string targetBeginEnd;
    std::string targetCBeginCEnd;
    std::copy(normalized.begin(), normalized.end(), std::back_inserter(targetBeginEnd));
    std::copy(normalized.cbegin(), normalized.cend(), std::back_inserter(targetCBeginCEnd));

    BOOST_REQUIRE_EQUAL(targetBeginEnd, origin);
    BOOST_REQUIRE_EQUAL(targetCBeginCEnd, origin);
}


BOOST_AUTO_TEST_CASE(symbol_reverse_directed_iterators_behave_all_the_same)
{
    std::string origin = "going the other direction might be the answer";
    testaux::static_symbol_pool pool(origin);

    minijava::symbol normalized = pool.get();

    std::string targetRBeginREnd(normalized.rbegin(), normalized.rend());
    std::string targetCRBeginCREnd(normalized.crbegin(), normalized.crend());

    std::string reverse_origin(origin.crbegin(), origin.crend());

    BOOST_REQUIRE_EQUAL(targetRBeginREnd, reverse_origin);
    BOOST_REQUIRE_EQUAL(targetCRBeginCREnd, reverse_origin);
}

BOOST_AUTO_TEST_CASE(empty_symbol_is_empty)
{
    std::string empty = "";
    std::string origin = "non empty";
    testaux::static_symbol_pool empty_pool(empty);
    testaux::static_symbol_pool full_pool(origin);

    minijava::symbol stdctor_sym;
    minijava::symbol empty_normalized = empty_pool.get();
    minijava::symbol full_normalized = full_pool.get();

    BOOST_REQUIRE(stdctor_sym.empty());
    BOOST_REQUIRE(empty_normalized.empty());
    BOOST_REQUIRE(not full_normalized.empty());
}

BOOST_AUTO_TEST_CASE(test_symbol_index_access)
{
    std::string origin = "its good if you know what you want";
    testaux::static_symbol_pool pool(origin);

    minijava::symbol normalized = pool.get();

    for (std::size_t i = 0; i < origin.length(); ++i) {
        BOOST_REQUIRE_EQUAL(origin[i], normalized[i]);
        BOOST_REQUIRE_EQUAL(origin.at(i), normalized.at(i));
    }
}

BOOST_AUTO_TEST_CASE(symbol_at_throws_out_of_bound_exception)
{
    std::string origin = "small";
    testaux::static_symbol_pool pool(origin);

    minijava::symbol normalized = pool.get();

    const size_t to = 100;

    for (std::size_t i = 0; i < to; ++i) {
        if (i < origin.size()) {
            BOOST_REQUIRE_NO_THROW(normalized.at(i));
        } else {
            BOOST_REQUIRE_THROW(normalized.at(i), std::out_of_range);
        }
    }
}

BOOST_AUTO_TEST_CASE(test_symbol_back_front)
{
    std::string origin = "a and o";
    testaux::static_symbol_pool pool(origin);

    minijava::symbol normalized = pool.get();

    BOOST_CHECK_EQUAL(origin.front(), normalized.front());
    BOOST_CHECK_EQUAL(origin.back(), normalized.back());
}


BOOST_AUTO_TEST_CASE(comparison_between_symbols_and_strings)
{
	using namespace std::string_literals;
	const auto text = "Grumpy Wizards make toxic brew for the Evil Queen and Jack"s;
	testaux::static_symbol_pool spool{text};
	const auto canon = spool.get();
	// We use explicit operators here to have full control over the comparison.
	BOOST_REQUIRE(text == canon);
	BOOST_REQUIRE(canon == text);
	BOOST_REQUIRE(text.c_str() == canon);
	BOOST_REQUIRE(canon == text.c_str());
	BOOST_REQUIRE("Grumpy"s != canon);
	BOOST_REQUIRE(canon != "Grumpy"s);
	BOOST_REQUIRE("Wizards" != canon);
	BOOST_REQUIRE(canon != "Wizards");
	BOOST_REQUIRE(""s != canon);
	BOOST_REQUIRE(canon != ""s);
	BOOST_REQUIRE("" != canon);
	BOOST_REQUIRE(canon != "");
}


BOOST_AUTO_TEST_CASE(comparison_between_symbols_and_strings_with_embedded_nulls)
{
	using namespace std::string_literals;
	const auto text = "not done\0yet"s;
	testaux::static_symbol_pool spool{text};
	const auto canon = spool.get();
	BOOST_REQUIRE_EQUAL(canon, text);
	BOOST_REQUIRE_NE(canon, "not done");
	BOOST_REQUIRE_NE(canon, "not done\0boy"s);
	BOOST_REQUIRE_NE(canon, text + "\0"s);
}


BOOST_AUTO_TEST_CASE(comparison_between_empty_symbols_and_strings)
{
	using namespace std::string_literals;
	BOOST_REQUIRE_EQUAL(minijava::symbol{}, std::string{});
	BOOST_REQUIRE_EQUAL(minijava::symbol{}, ""s);
	BOOST_REQUIRE_EQUAL(minijava::symbol{}, "");
	BOOST_REQUIRE_NE(minijava::symbol{}, "\0"s);
	BOOST_REQUIRE_NE(minijava::symbol{}, " ");
}


BOOST_AUTO_TEST_CASE(symbol_pointer_comparison_defines_total_ordering)
{
	auto engine = std::default_random_engine{};
	auto pool = minijava::symbol_pool<>{};
	auto symbols = std::vector<minijava::symbol>{200};
	std::generate(symbols.begin(), symbols.end(), [&](){
		return pool.normalize(
				testaux::get_random_identifier(engine, std::size_t{50})
		);
	});
	std::sort(symbols.begin(), symbols.end(), minijava::symbol_comparator{});
	BOOST_REQUIRE(
			std::adjacent_find(symbols.begin(), symbols.end(), [](const auto& s1, const auto& s2) {
				const auto pointer_greater_than = std::greater<const void*>{};
				return pointer_greater_than(s1.data(), s2.data());
			}) == symbols.end()
	);
}
