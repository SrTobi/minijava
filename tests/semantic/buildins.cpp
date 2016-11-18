#include "semantic/buildins.hpp"

#include <algorithm>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

#define BOOST_TEST_MODULE  semantic_buildins
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "semantic/type_system.hpp"


BOOST_AUTO_TEST_CASE(check_unary_logical_not_operation)
{
	namespace semantic = minijava::semantic;
	using minijava::ast::unary_operation_type;
	using semantic::type_system;

	BOOST_CHECK(not semantic::resolve_unary_operator(unary_operation_type::logical_not, type_system::t_int()));
	BOOST_CHECK(not semantic::resolve_unary_operator(unary_operation_type::logical_not, type_system::t_void()));
	BOOST_CHECK(not semantic::resolve_unary_operator(unary_operation_type::logical_not, type_system::t_boolean_array(1)));
	BOOST_CHECK(semantic::resolve_unary_operator(unary_operation_type::logical_not, type_system::t_boolean()));
}
