#include "dummy.hpp"

#define BOOST_TEST_MODULE  dummy

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "testaux/testaux.hpp"


BOOST_AUTO_TEST_CASE(four_more_than_zero_is_four)
{
	BOOST_CHECK_EQUAL(minijava::four_more(0), 4);
}


static const int silly_data[] = {3, 1, 4, 1, 5, 9, 2, 6, 5};

BOOST_DATA_TEST_CASE(four_more_than_x_is_x_plus_four, silly_data)
{
	BOOST_CHECK_EQUAL(minijava::four_more(sample), sample + 4);
}


BOOST_AUTO_TEST_CASE(fourty_two_is_equal_to_itself)
{
	BOOST_CHECK_EQUAL(testaux::maybe_not_that_useful(40 + 2), 42);
}
