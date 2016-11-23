#include "semantic/type_info.hpp"

#define BOOST_TEST_MODULE  semantic_type_system
#include <boost/test/unit_test.hpp>

#include "symbol/symbol_pool.hpp"
#include "testaux/unique_ptr_vector.hpp"

namespace ast = minijava::ast;


namespace /* anonymous */
{

	minijava::sem::basic_type_info
	make_me_a_type_please(minijava::symbol_pool<>& pool, const bool builtin,
						  std::unique_ptr<ast::class_declaration>& out)
	{
		out = std::make_unique<ast::class_declaration>(
			pool.normalize("Elephant"),
			testaux::make_unique_ptr_vector<ast::var_decl>(),
			testaux::make_unique_ptr_vector<ast::instance_method>(),
			testaux::make_unique_ptr_vector<ast::main_method>()
		);
		return minijava::sem::basic_type_info{*out, builtin};
	}

}  // namespace /* anonymous */

BOOST_AUTO_TEST_CASE(properties_of_null_t)
{
	constexpr auto t = minijava::sem::basic_type_info::make_null_type();
	static_assert(nullptr == t.declaration(), "there out to be no declaration of void");
	static_assert(    t.is_builtin(),      "null ought to be a builtin");
	static_assert(not t.is_primitive(),    "null ought to be a primitive");
	static_assert(not t.is_instantiable(), "null ought to be not instantiable");
	static_assert(    t.is_reference(),    "null ought to be a reference");
	static_assert(    t.is_null(),         "null ought to be null");
	static_assert(not t.is_void(),         "null ought to be not void");
	static_assert(not t.is_int(),          "null ought to be not int");
	static_assert(not t.is_boolean(),      "null ought to be boolean");
	static_assert(not t.is_user_defined(), "null ought to be not user-defined");
}


BOOST_AUTO_TEST_CASE(properties_of_void_t)
{
	constexpr auto t = minijava::sem::basic_type_info::make_void_type();
	static_assert(nullptr == t.declaration(), "there out to be no declaration of void");
	static_assert(    t.is_builtin(),      "void ought to be a builtin");
	static_assert(    t.is_primitive(),    "void ought to be a primitive");
	static_assert(not t.is_instantiable(), "void ought to be not instantiable");
	static_assert(not t.is_reference(),    "void ought to be no reference");
	static_assert(not t.is_null(),         "void ought to be not null");
	static_assert(    t.is_void(),         "void ought to be void");
	static_assert(not t.is_int(),          "void ought to be not int");
	static_assert(not t.is_boolean(),      "void ought to be boolean");
	static_assert(not t.is_user_defined(), "void ought to be not user-defined");
}


BOOST_AUTO_TEST_CASE(properties_of_int_t)
{
	constexpr auto t = minijava::sem::basic_type_info::make_int_type();
	static_assert(nullptr == t.declaration(), "there out to be no declaration of int");
	static_assert(    t.is_builtin(),      "int ought to be a builtin");
	static_assert(    t.is_primitive(),    "int ought to be a primitive");
	static_assert(    t.is_instantiable(), "int ought to be instantiable");
	static_assert(not t.is_reference(),    "int ought to be no reference");
	static_assert(not t.is_null(),         "int ought to be not null");
	static_assert(not t.is_void(),         "int ought to be not void");
	static_assert(    t.is_int(),          "int ought to be int");
	static_assert(not t.is_boolean(),      "int ought to be boolean");
	static_assert(not t.is_user_defined(), "int ought to be not user-defined");
}


BOOST_AUTO_TEST_CASE(properties_of_boolean_t)
{
	constexpr auto t = minijava::sem::basic_type_info::make_boolean_type();
	static_assert(nullptr == t.declaration(), "there out to be no declaration of boolean");
	static_assert(    t.is_builtin(),      "boolean ought to be a builtin");
	static_assert(    t.is_primitive(),    "boolean ought to be a primitive");
	static_assert(    t.is_instantiable(), "boolean ought to be instantiable");
	static_assert(not t.is_reference(),    "boolean ought to be no reference");
	static_assert(not t.is_null(),         "boolean ought to be not null");
	static_assert(not t.is_void(),         "boolean ought to be not void");
	static_assert(not t.is_int(),          "boolean ought to be not int");
	static_assert(    t.is_boolean(),      "boolean ought to be boolean");
	static_assert(not t.is_user_defined(), "boolean ought to be not user-defined");
}


BOOST_AUTO_TEST_CASE(properties_of_builtin_reference_types)
{
	auto pool = minijava::symbol_pool<>{};
	auto clazz = std::unique_ptr<ast::class_declaration>{};
	const auto tb = make_me_a_type_please(pool, true, clazz);
	BOOST_REQUIRE_EQUAL(clazz.get(), tb.declaration());
	BOOST_REQUIRE( tb.is_builtin());
	BOOST_REQUIRE(!tb.is_primitive());
	BOOST_REQUIRE( tb.is_instantiable());
	BOOST_REQUIRE( tb.is_reference());
	BOOST_REQUIRE(!tb.is_null());
	BOOST_REQUIRE(!tb.is_void());
	BOOST_REQUIRE(!tb.is_int());
	BOOST_REQUIRE(!tb.is_boolean());
	BOOST_REQUIRE(!tb.is_user_defined());
}


BOOST_AUTO_TEST_CASE(properties_of_user_defined_types)
{
	auto pool = minijava::symbol_pool<>{};
	auto clazz = std::unique_ptr<ast::class_declaration>{};
	const auto tu = make_me_a_type_please(pool, false, clazz);
	BOOST_REQUIRE_EQUAL(clazz.get(), tu.declaration());
	BOOST_REQUIRE(!tu.is_builtin());
	BOOST_REQUIRE(!tu.is_primitive());
	BOOST_REQUIRE( tu.is_instantiable());
	BOOST_REQUIRE( tu.is_reference());
	BOOST_REQUIRE(!tu.is_null());
	BOOST_REQUIRE(!tu.is_void());
	BOOST_REQUIRE(!tu.is_int());
	BOOST_REQUIRE(!tu.is_boolean());
	BOOST_REQUIRE( tu.is_user_defined());
}
