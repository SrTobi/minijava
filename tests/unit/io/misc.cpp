#include "io/misc.hpp"

#include <cerrno>
#include <string>
#include <system_error>

#define BOOST_TEST_MODULE  io_misc.hpp
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

using namespace std::string_literals;


static const std::string positive_data[] = {
	"file.txt",
	"bar/baz/foo",
	"just-about/ any //./.silly...ness.",
	"/",
	".",
	"./.",
};

BOOST_DATA_TEST_CASE(positive, positive_data)
{
	minijava::check_file_name(sample);
}


static const std::string negative_data[] = {
	""s,
	"\0"s,
	"file.txt\0"s,
	"file.txt\0oho"s,
	"\0file.txt"s,
	"\0\0\0"s,
};

BOOST_DATA_TEST_CASE(negative, negative_data)
{
	BOOST_REQUIRE_EXCEPTION(
		minijava::check_file_name(sample),
		std::system_error,
		[](auto&& e){ return e.code().value() == EINVAL; }
	);
}
