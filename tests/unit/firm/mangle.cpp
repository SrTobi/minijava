#include "firm/mangle.hpp"

#include <string>

#define BOOST_TEST_MODULE  firm_mangle
#include <boost/test/unit_test.hpp>

#include "libfirm/ident.h"

#include "symbol/symbol_pool.hpp"
#include "firm/firm.hpp"

#include "testaux/ast_test_factory.hpp"


static auto global_state_is_evil = minijava::firm_global_state{};


// TOTHINK: What good is class name mangling?
BOOST_AUTO_TEST_CASE(mangled_class)
{
	using namespace std::string_literals;
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_empty_class("Test");
	const auto mangled = minijava::firm::mangle(*ast);
	const auto expected = "Mj_Test_c4"s;
	const auto actual = std::string{get_id_str(mangled)};
	BOOST_REQUIRE_EQUAL(expected, actual);
}


BOOST_AUTO_TEST_CASE(mangled_variable)
{
	using namespace std::string_literals;
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_declaration("foo", minijava::ast::primitive_type::type_int);
	const auto mangled = minijava::firm::mangle(*ast);
	const auto expected = "mj_foo_v3"s;
	const auto actual = std::string{get_id_str(mangled)};
	BOOST_REQUIRE_EQUAL(expected, actual);
}


BOOST_AUTO_TEST_CASE(mangled_field)
{
	using namespace std::string_literals;
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.as_class("Test", tf.make_declaration("field", "Abc"));
	const auto mangled = minijava::firm::mangle(*ast, *ast->fields().front());
	const auto expected = "Mj_Test_c4_field_f5"s;
	const auto actual = std::string{get_id_str(mangled)};
	BOOST_REQUIRE_EQUAL(expected, actual);
}


BOOST_AUTO_TEST_CASE(mangled_method)
{
	using namespace std::string_literals;
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.as_class("Test", tf.make_empty_method("method"));
	const auto mangled = minijava::firm::mangle(*ast, *ast->instance_methods().front());
	const auto expected = "Mj_Test_c4_method_m6"s;
	const auto actual = std::string{get_id_str(mangled)};
	BOOST_REQUIRE_EQUAL(expected, actual);
}


BOOST_AUTO_TEST_CASE(same_names_equal)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_empty_class("Test");
	const auto mangled1st = minijava::firm::mangle(*ast);
	const auto mangled2nd = minijava::firm::mangle(*ast);
	BOOST_REQUIRE(mangled1st == mangled2nd);
}
