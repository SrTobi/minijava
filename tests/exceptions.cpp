#include "exceptions.hpp"

#include <string>

#define BOOST_TEST_MODULE  exceptions
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_CASE(ice_with_default_error_message)
{
	const auto e = minijava::internal_compiler_error{};
	BOOST_REQUIRE_EQUAL("Internal compiler error", e.what());
}


BOOST_AUTO_TEST_CASE(ice_with_custom_error_message)
{
	using namespace std::string_literals;
	const auto msg = "Too little sunlight"s;
	const auto e = minijava::internal_compiler_error{msg};
	BOOST_REQUIRE_EQUAL(msg, e.what());
}


BOOST_AUTO_TEST_CASE(ice_with_source_code_loacation)
{
	using namespace std::string_literals;
	const auto e = minijava::internal_compiler_error{"file.cpp", 42, "func"};
	BOOST_REQUIRE_EQUAL("file.cpp:42: func: Internal compiler error"s, e.what());
}


BOOST_AUTO_TEST_CASE(ice_with_source_code_loacation_and_custom_message)
{
	using namespace std::string_literals;
	const auto e = minijava::internal_compiler_error{"file.cpp", 42, "func", "oops"};
	BOOST_REQUIRE_EQUAL("file.cpp:42: func: oops"s, e.what());
}


BOOST_AUTO_TEST_CASE(make_ice_macro)
{
	using namespace std::string_literals;
	const auto msg = "Internal compiler error"s;
	const auto file = std::string{__FILE__};
	const auto func = std::string{__func__};
	const auto line = std::to_string(__LINE__ + 1);
	const minijava::internal_compiler_error e = MINIJAVA_MAKE_ICE();
	const auto what = std::string{e.what()};
	BOOST_REQUIRE_EQUAL(file + ":" + line + ": " + func + ": " + msg, what);
}


BOOST_AUTO_TEST_CASE(make_ice_msg_macro)
{
	using namespace std::string_literals;
	const auto msg = "Bad things going on"s;
	const auto file = std::string{__FILE__};
	const auto func = std::string{__func__};
	const auto line = std::to_string(__LINE__ + 1);
	const minijava::internal_compiler_error e = MINIJAVA_MAKE_ICE_MSG(msg);
	const auto what = std::string{e.what()};
	BOOST_REQUIRE_EQUAL(file + ":" + line + ": " + func + ": " + msg, what);
}


BOOST_AUTO_TEST_CASE(not_implemented_ice)
{
	const auto e = minijava::not_implemented_error{};
	BOOST_REQUIRE_EQUAL("Not implemented yet", e.what());
}
