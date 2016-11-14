#include "symbol/symbol_anchor.hpp"

#include "global.hpp"

#include <cstddef>
#include <set>
#include <type_traits>
#include <utility>

#define BOOST_TEST_MODULE  symbol_symbol_anchor
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_CASE(symbol_anchor_is_empty_pod)
{
	BOOST_REQUIRE(std::is_empty<minijava::symbol_anchor>{});
	BOOST_REQUIRE(std::is_pod<minijava::symbol_anchor>{});
}


BOOST_AUTO_TEST_CASE(empty_symbol_anchor_is_singleton)
{
	const auto w1 = minijava::symbol_anchor::get_empty_symbol_anchor();
	const auto w2 = minijava::symbol_anchor::get_empty_symbol_anchor();
	const auto s1 = w1.lock();
	const auto s2 = w2.lock();
	BOOST_REQUIRE_EQUAL(s1.get(), s2.get());
}


BOOST_AUTO_TEST_CASE(empty_symbol_anchor_is_null_iff_release)
{
	const auto sp = minijava::symbol_anchor::get_empty_symbol_anchor().lock();
	BOOST_REQUIRE_EQUAL(!!(MINIJAVA_ASSERT_ACTIVE), !!(sp));
}


BOOST_AUTO_TEST_CASE(make_symbol_anchor_is_null_iff_release)
{
	const auto sp = minijava::symbol_anchor::make_symbol_anchor();
	BOOST_REQUIRE_EQUAL(!!(MINIJAVA_ASSERT_ACTIVE), !!(sp));
}


BOOST_AUTO_TEST_CASE(make_symbol_anchor_is_different_every_time)
{
	const auto n = std::size_t{100};
	auto anchors = std::set<std::shared_ptr<minijava::symbol_anchor>>{};
	for (auto i = std::size_t{0}; i < n; ++i) {
		auto sp = minijava::symbol_anchor::make_symbol_anchor();
		anchors.insert(std::move(sp));
	}
	const auto expected = MINIJAVA_ASSERT_ACTIVE ? n : std::size_t{1};
	BOOST_REQUIRE_EQUAL(expected, anchors.size());
}
