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
	BOOST_REQUIRE(not pool.contains(""));
	BOOST_REQUIRE(not pool.contains("elephant"));
}


BOOST_AUTO_TEST_CASE(pool_returns_empty_symbol_singlton)
{
	auto pool = minijava::symbol_pool<>{};
	auto empty_symbol = pool.normalize("");
	auto full_symbol = pool.normalize("testtest");
	BOOST_REQUIRE(empty_symbol.empty());
	BOOST_REQUIRE_EQUAL(empty_symbol, minijava::symbol{});
	BOOST_REQUIRE_NE(empty_symbol, full_symbol);
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

BOOST_AUTO_TEST_CASE(move_constructed_pool_behaves_like_old_pool)
{
	using namespace std::string_literals;
	auto pool = minijava::symbol_pool<>{};
	const auto text = "matchstick"s;
	const auto canonical = pool.normalize(text);
	BOOST_REQUIRE(canonical.c_str() != text.c_str());

	auto moved_pool = std::move(pool);
	const auto canonical_from_moved = moved_pool.normalize(text);
	BOOST_REQUIRE_EQUAL(canonical_from_moved, canonical);
	BOOST_REQUIRE_EQUAL(moved_pool.size(), std::size_t(1));

	// old pool is empty
	BOOST_REQUIRE(pool.empty());
}

BOOST_AUTO_TEST_CASE(move_assigned_pool_behaves_like_old_pool)
{
	using namespace std::string_literals;
	auto pool = minijava::symbol_pool<>{};
	const auto text = "matchstick"s;
	const auto canonical = pool.normalize(text);
	BOOST_REQUIRE(canonical.c_str() != text.c_str());
	BOOST_REQUIRE_EQUAL(canonical.c_str(), text.c_str());

	auto second_pool = minijava::symbol_pool<>{};

	const auto canonical2 = second_pool.normalize(text);
	BOOST_REQUIRE_EQUAL(canonical2.c_str(), canonical.c_str());
	BOOST_REQUIRE(not canonical2.empty());

	second_pool = std::move(pool);
	const auto canonical_from_moved = second_pool.normalize(text);
	BOOST_REQUIRE_EQUAL(canonical_from_moved, canonical);
	BOOST_REQUIRE_EQUAL(second_pool.size(), std::size_t(1));

	// old pool is empty
	BOOST_REQUIRE(pool.empty());
}
