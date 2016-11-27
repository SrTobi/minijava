#include "position.hpp"

#define BOOST_TEST_MODULE  position
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_CASE(position_compare_le)
{
	using pos = minijava::position;

	BOOST_CHECK_LE(pos(1,1), pos(1,1));
	BOOST_CHECK_LE(pos(1,1), pos(2,2));
	BOOST_CHECK_LE(pos(1,1), pos(1,2));
}

BOOST_AUTO_TEST_CASE(position_compare_lt)
{
	using pos = minijava::position;

	BOOST_CHECK_LT(pos(1,1), pos(2,2));
	BOOST_CHECK_LT(pos(1,1), pos(1,2));
	BOOST_CHECK(!(pos(1,1) < pos(1,1)));
}

BOOST_AUTO_TEST_CASE(position_compare_gt)
{
	using pos = minijava::position;

	BOOST_CHECK_GT(pos(2,2), pos(1,1));
	BOOST_CHECK_GT(pos(1,2), pos(1,1));
	BOOST_CHECK(!(pos(1,1) > pos(1,1)));
}

BOOST_AUTO_TEST_CASE(position_compare_ge)
{
	using pos = minijava::position;

	BOOST_CHECK_GE(pos(2,2), pos(1,1));
	BOOST_CHECK_GE(pos(1,2), pos(1,1));
	BOOST_CHECK_GE(pos(1,1), pos(1,1));
}
