#include <testx/testx.hpp>


TESTX_AUTO_TEST_CASE(no_param_test)
{
	int i = 5;
	BOOST_CHECK_EQUAL(i, 5);
}

TESTX_PARAM_TEST_CASE(smaller_5, int testing)
{
	BOOST_CHECK_LT(testing, 5);
}

TESTX_PARAM_TEST(smaller_5, 3);
TESTX_PARAM_TEST(smaller_5, 2);
TESTX_PARAM_TEST(smaller_5, 1);
