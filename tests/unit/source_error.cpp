#include "source_error.hpp"

#define BOOST_TEST_MODULE  source_error
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_CASE(source_error_can_be_created)
{
	{
		minijava::source_error s{"message"};
	}
	{

		minijava::source_error s{"message", minijava::position{1,1}};
	}
}
