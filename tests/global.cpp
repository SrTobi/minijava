#include "global.hpp"

#include "testaux/testaux.hpp"

#define BOOST_TEST_MODULE  global
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_CASE(project_name_is_string_literal)
{
	static_assert(testaux::cx_strlen(MINIJAVA_PROJECT_NAME) > 0);
}


BOOST_AUTO_TEST_CASE(project_version_is_string_literal)
{
	static_assert(testaux::cx_strlen(MINIJAVA_PROJECT_VERSION) > 0);
}
