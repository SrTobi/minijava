#define BOOST_TEST_MODULE  dummy

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "cli.hpp"


BOOST_AUTO_TEST_CASE(when_called_with_no_arguments_real_main_does_nothing)
{
	minijava::real_main({});
}
