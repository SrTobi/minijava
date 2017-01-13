#include "runtime/runtime.hpp"

#include <string>

#define BOOST_TEST_MODULE  runtime_runtime
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_CASE(runtime_source_can_be_called)
{
	using namespace std::string_literals;
	BOOST_REQUIRE_NE(""s, minijava::runtime_source());
}
