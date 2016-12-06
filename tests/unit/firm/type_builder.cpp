#include "firm/type_builder.hpp"

#define BOOST_TEST_MODULE  firm_type_builder
#include <boost/test/unit_test.hpp>

#include "firm/firm.hpp"


static const auto g_firm = minijava::firm_ir{};


BOOST_AUTO_TEST_CASE(default_constructed_primitive_types_are_null)
{
	const auto pt = minijava::firm::primitive_types{};
	BOOST_CHECK(nullptr == pt.int_mode);
	BOOST_CHECK(nullptr == pt.boolean_mode);
	BOOST_CHECK(nullptr == pt.int_type);
	BOOST_CHECK(nullptr == pt.boolean_type);
}


BOOST_AUTO_TEST_CASE(created_primitive_types_are_not_null)
{
	const auto pt = minijava::firm::primitive_types::get_instance();
	BOOST_CHECK(nullptr != pt.int_mode);
	BOOST_CHECK(nullptr != pt.boolean_mode);
	BOOST_CHECK(nullptr != pt.int_type);
	BOOST_CHECK(nullptr != pt.boolean_type);
}


BOOST_AUTO_TEST_CASE(primitive_types_are_unique)
{
	const auto p1 = minijava::firm::primitive_types::get_instance();
	const auto p2 = minijava::firm::primitive_types::get_instance();
	BOOST_CHECK_EQUAL(p1.int_mode, p2.int_mode);
	BOOST_CHECK_EQUAL(p1.boolean_mode, p2.boolean_mode);
	BOOST_CHECK_EQUAL(p1.int_type, p2.int_type);
	BOOST_CHECK_EQUAL(p1.boolean_type, p2.boolean_type);
}
