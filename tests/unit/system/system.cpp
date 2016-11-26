#include "system/system.hpp"

#include <cstddef>

#define BOOST_TEST_MODULE  system_system
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_CASE(set_max_stack_size_limit_query)
{
	try {
		const auto prev1 = minijava::set_max_stack_size_limit(0);
		const auto prev2 = minijava::set_max_stack_size_limit(0);
		BOOST_REQUIRE_EQUAL(prev1, prev2);
	} catch (const std::system_error& e) {
#ifndef MINIJAVA_HAVE_RLIMIT
		BOOST_REQUIRE_EQUAL(ENOSYS, e.code().value());
#endif
	}
}


BOOST_AUTO_TEST_CASE(set_max_stack_size_limit_1gib)
{
	try {
		const auto desired = static_cast<std::ptrdiff_t>(std::size_t{1} << 30);
		minijava::set_max_stack_size_limit(desired);
		const auto actual = minijava::set_max_stack_size_limit(0);
		BOOST_REQUIRE_EQUAL(desired, actual);
	} catch (const std::system_error&) {
		// failure is okay
	}
}


BOOST_AUTO_TEST_CASE(set_max_stack_size_limit_unlimited)
{
	try {
		const auto desired = std::ptrdiff_t{-1};
		minijava::set_max_stack_size_limit(desired);
		const auto actual = minijava::set_max_stack_size_limit(0);
		BOOST_REQUIRE_EQUAL(desired, actual);
	} catch (const std::system_error&) {
		// failure is okay
	}
}
