#include "semantic/semantic_error.hpp"

#include <cstring>
#include <exception>
#include <string>
#include <type_traits>

#define BOOST_TEST_MODULE  semantic_semantic_error
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_CASE(derived_from_exception)
{
	static_assert(std::is_base_of<std::exception, minijava::semantic_error>{}, "");
}


BOOST_AUTO_TEST_CASE(can_throw_and_catch)
{
	try {
		throw minijava::semantic_error{};
	} catch (const minijava::semantic_error& e) {
		BOOST_REQUIRE(e.what() != nullptr);
	}
}


BOOST_AUTO_TEST_CASE(default_constructed_has_some_message)
{
	const auto e = minijava::semantic_error{};
	BOOST_REQUIRE_GT(std::strlen(e.what()), 0);
}


BOOST_AUTO_TEST_CASE(constructed_with_message_has_that_message)
{
	using namespace std::string_literals;
	const auto msg = "Your program makes no sense at all"s;
	const auto e = minijava::semantic_error{msg};
	BOOST_REQUIRE_EQUAL(msg, e.what());
}
