#include "semantic/std_definitions.hpp"

#include <algorithm>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

#define BOOST_TEST_MODULE  semantic_std_definitions
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>


BOOST_AUTO_TEST_CASE(check_unary_logical_not_operation)
{
	using minijava::ast::unary_operation_type;
	auto t_int_arr = minijava::semantic_type(minijava::semantic_type_kind::kind_int, 1);
	auto t_int = minijava::semantic_type(minijava::semantic_type_kind::kind_int);
	auto t_void = minijava::semantic_type(minijava::semantic_type_kind::kind_void);
	auto t_boolean = minijava::semantic_type(minijava::semantic_type_kind::kind_boolean);

	BOOST_CHECK(not minijava::resolve_unary_operator(unary_operation_type::logical_not, t_int));
	BOOST_CHECK(not minijava::resolve_unary_operator(unary_operation_type::logical_not, t_void));
	BOOST_CHECK(not minijava::resolve_unary_operator(unary_operation_type::logical_not, t_int_arr));
	BOOST_CHECK(minijava::resolve_unary_operator(unary_operation_type::logical_not, t_boolean));
}
