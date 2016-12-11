#include "util/raii.hpp"

#include <utility>

#define BOOST_TEST_MODULE  util_raii
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_CASE(const_setter_sets_ints)
{
	const auto setter = minijava::const_setter<int>{42};
	auto value = 0;
	setter(&value);
	BOOST_REQUIRE_EQUAL(42, value);
}


BOOST_AUTO_TEST_CASE(const_setter_sets_pointer)
{
	const auto setter = minijava::const_setter<const void*>{nullptr};
	const void* value = &setter;
	setter(&value);
	BOOST_REQUIRE(nullptr == value);
}


BOOST_AUTO_TEST_CASE(set_temporarily_sets_and_resets_value)
{
	auto value = 7;
	{
		const auto g = minijava::set_temporarily(value, 42);
		BOOST_REQUIRE_EQUAL(42, value);
	}
	BOOST_REQUIRE_EQUAL(7, value);
}


BOOST_AUTO_TEST_CASE(increment_temporarily_sets_and_resets_value)
{
	auto value = 7;
	{
		const auto g1 = minijava::increment_temporarily(value);
		BOOST_REQUIRE_EQUAL(8, value);
		{
			const auto g2 = minijava::increment_temporarily(value, 2);
			BOOST_REQUIRE_EQUAL(10, value);
		}
		BOOST_REQUIRE_EQUAL(8, value);
	}
	BOOST_REQUIRE_EQUAL(7, value);
}


BOOST_AUTO_TEST_CASE(reassignment_of_guard)
{
	auto value = 0;
	{
		auto guard = minijava::set_temporarily(value, 1);
		guard = minijava::set_temporarily(value, 2);
		BOOST_REQUIRE_EQUAL(0, value);
	}
	BOOST_REQUIRE_EQUAL(1, value);
}


BOOST_AUTO_TEST_CASE(movement_of_guard)
{
	using std::swap;
	auto value = 0;
	{
		auto outer = minijava::set_temporarily(value, 1);
		{
			auto inner = minijava::set_temporarily(value, 2);
			swap(outer, inner);
		}
		BOOST_REQUIRE_EQUAL(0, value);
	}
	BOOST_REQUIRE_EQUAL(1, value);
}
