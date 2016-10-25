#include "symbol_pool.hpp"

#include <cstddef>
#include <string>

#define BOOST_TEST_MODULE  symbol_pool
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_CASE(empty_when_default_constructed)
{
	const auto pool = minijava::symbol_pool<>{};
	BOOST_REQUIRE_EQUAL(std::size_t{0}, pool.size());
	BOOST_REQUIRE(pool.empty());
}


BOOST_AUTO_TEST_CASE(empty_pool_contains_nothing)
{
	const auto pool = minijava::symbol_pool<>{};
	BOOST_REQUIRE(not pool.contains("elephant"));
}


BOOST_AUTO_TEST_CASE(contains_string_after_normalization)
{
	using namespace std::string_literals;
	auto pool = minijava::symbol_pool<>{};
	const auto text = "matchstick"s;
	pool.normalize(text);
	BOOST_REQUIRE(pool.contains(text));
}


BOOST_AUTO_TEST_CASE(returns_canonical_pointer_after_normalization)
{
	using namespace std::string_literals;
	auto pool = minijava::symbol_pool<>{};
	const auto text = "matchstick"s;
	const auto canonical = pool.normalize(text);
	BOOST_REQUIRE_EQUAL(canonical, pool.normalize(text));
}


BOOST_AUTO_TEST_CASE(canonical_pointer_is_inside_pool)
{
	using namespace std::string_literals;
	auto pool = minijava::symbol_pool<>{};
	const auto text = "matchstick"s;
	const auto canonical = pool.normalize(text);
	BOOST_REQUIRE(canonical.c_str() != text.c_str());
}


BOOST_AUTO_TEST_CASE(correct_size_after_normalization)
{
	auto pool = minijava::symbol_pool<>{};
	BOOST_REQUIRE_EQUAL(std::size_t{0}, pool.size());
	pool.normalize("alpha");
	BOOST_REQUIRE_EQUAL(std::size_t{1}, pool.size());
	BOOST_REQUIRE(not pool.empty());
	pool.normalize("beta");
	BOOST_REQUIRE_EQUAL(std::size_t{2}, pool.size());
	BOOST_REQUIRE(not pool.empty());
	pool.normalize("gamma");
	BOOST_REQUIRE_EQUAL(std::size_t{3}, pool.size());
	BOOST_REQUIRE(not pool.empty());
	pool.normalize("delta");
	BOOST_REQUIRE_EQUAL(std::size_t{4}, pool.size());
	BOOST_REQUIRE(not pool.empty());
	pool.normalize("beta");  // should already be present
	BOOST_REQUIRE_EQUAL(std::size_t{4}, pool.size());
	BOOST_REQUIRE(not pool.empty());
}
