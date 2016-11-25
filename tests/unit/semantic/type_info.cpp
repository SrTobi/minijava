#include "semantic/type_info.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#define BOOST_TEST_MODULE  semantic_type_info
#include <boost/test/unit_test.hpp>

#include <boost/lexical_cast.hpp>

#include "parser/ast.hpp"
#include "semantic/semantic_error.hpp"

#include "testaux/ast_test_factory.hpp"
#include "testaux/random_tokens.hpp"
#include "testaux/unique_ptr_vector.hpp"

namespace ast = minijava::ast;


BOOST_AUTO_TEST_CASE(properties_of_null_t)
{
	constexpr auto t = minijava::sem::basic_type_info::make_null_type();
	static_assert(nullptr == t.declaration(), "there ought to be no declaration of void");
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
	static_assert(nullptr == t.declaration(), "there ought to be no declaration of void");
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
	static_assert(nullptr == t.declaration(), "there ought to be no declaration of int");
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
	static_assert(nullptr == t.declaration(), "there ought to be no declaration of boolean");
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
	auto tf = testaux::ast_test_factory{};
	auto clazz = tf.make_empty_class();
	const auto tb = minijava::sem::basic_type_info{*clazz, true};
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
	auto tf = testaux::ast_test_factory{};
	auto clazz = tf.make_empty_class();
	const auto tu = minijava::sem::basic_type_info{*clazz, false};
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


BOOST_AUTO_TEST_CASE(types_are_equal_only_to_themselves)
{
	auto tf = testaux::ast_test_factory{};
	auto btis = std::vector<minijava::sem::basic_type_info>{
			minijava::sem::basic_type_info::make_null_type(),
			minijava::sem::basic_type_info::make_void_type(),
			minijava::sem::basic_type_info::make_int_type(),
			minijava::sem::basic_type_info::make_boolean_type(),
	};
	auto classdecls = std::vector<std::unique_ptr<ast::class_declaration>>{};
	auto engine = std::default_random_engine{};
	auto builtindist = std::bernoulli_distribution{};
	std::generate_n(
			std::back_inserter(classdecls),
			100,
			[&tf](){ return tf.make_empty_class(); }
	);
	std::transform(
			std::begin(classdecls), std::end(classdecls),
			std::back_inserter(btis),
			[&engine, &builtindist](auto&& clsdcl) {
				return minijava::sem::basic_type_info{*clsdcl, builtindist(engine)};
			}
	);
	for (auto it = std::begin(btis); it != std::end(btis); ++it) {
		const auto isme = [it](auto&& you) {
			return *it == you;
		};
		const auto clones = std::vector<minijava::sem::basic_type_info>(10, *it);
		BOOST_REQUIRE(std::all_of(std::begin(clones), std::end(clones), isme));
		for (auto jt = std::begin(btis); jt != std::end(btis); ++jt) {
			BOOST_REQUIRE_EQUAL((it == jt), (*it == *jt));
		}
	}
}


BOOST_AUTO_TEST_CASE(stream_insertion)
{
	using namespace std::string_literals;
	using bti_type = minijava::sem::basic_type_info;
	const auto stream = [](auto&& x){
		return boost::lexical_cast<std::string>(x);
	};
	BOOST_CHECK_EQUAL("__null_t"s, stream(bti_type::make_null_type()));
	BOOST_CHECK_EQUAL("void"s,     stream(bti_type::make_void_type()));
	BOOST_CHECK_EQUAL("int"s,      stream(bti_type::make_int_type()));
	BOOST_CHECK_EQUAL("boolean"s,  stream(bti_type::make_boolean_type()));
	auto tf = testaux::ast_test_factory{};
	const auto clsdecl = tf.make_empty_class("MyType");
	BOOST_CHECK_EQUAL("MyType"s,  stream(bti_type{*clsdecl, false}));
}


BOOST_AUTO_TEST_CASE(extract_type_info_success)
{
	auto tf = testaux::ast_test_factory{};
	auto classes = minijava::sem::class_definitions{};
	auto class_1 = tf.make_empty_class("builtin.Test");
	auto classp_1 = class_1.get();
	auto class_2 = tf.make_empty_class("builtin.My");
	auto classp_2 = class_2.get();
	auto class_3 = tf.make_empty_class("builtin.Class");
	auto classp_3 = class_3.get();
	auto program_builtin = std::make_unique<ast::program>(
			testaux::make_unique_ptr_vector<ast::class_declaration>(
					std::move(class_1),
					std::move(class_2),
					std::move(class_3)
			)
	);
	auto class_4 = tf.make_empty_class("Test");
	auto classp_4 = class_4.get();
	auto class_5 = tf.make_empty_class("My");
	auto classp_5 = class_5.get();
	auto class_6 = tf.make_empty_class("Class");
	auto classp_6 = class_6.get();
	auto program = std::make_unique<ast::program>(
			testaux::make_unique_ptr_vector<ast::class_declaration>(
					std::move(class_4),
					std::move(class_5),
					std::move(class_6)
			)
	);
	minijava::sem::extract_type_info(*program_builtin, true, classes);
	minijava::sem::extract_type_info(*program, false, classes);
	minijava::symbol s = tf.pool.normalize("builtin.Test");
	BOOST_CHECK(classes.find(s) != classes.end());
	BOOST_CHECK(classes.find(s)->second.is_builtin());
	BOOST_CHECK(!classes.find(s)->second.is_user_defined());
	BOOST_CHECK(classes.find(s)->second.declaration() == classp_1);
	s = tf.pool.normalize("builtin.My");
	BOOST_CHECK(classes.find(s) != classes.end());
	BOOST_CHECK(classes.find(s)->second.is_builtin());
	BOOST_CHECK(!classes.find(s)->second.is_user_defined());
	BOOST_CHECK(classes.find(s)->second.declaration() == classp_2);
	s = tf.pool.normalize("builtin.Class");
	BOOST_CHECK(classes.find(s) != classes.end());
	BOOST_CHECK(classes.find(s)->second.is_builtin());
	BOOST_CHECK(!classes.find(s)->second.is_user_defined());
	BOOST_CHECK(classes.find(s)->second.declaration() == classp_3);
	s = tf.pool.normalize("Test");
	BOOST_CHECK(classes.find(s) != classes.end());
	BOOST_CHECK(!classes.find(s)->second.is_builtin());
	BOOST_CHECK(classes.find(s)->second.is_user_defined());
	BOOST_CHECK(classes.find(s)->second.declaration() == classp_4);
	s = tf.pool.normalize("My");
	BOOST_CHECK(classes.find(s) != classes.end());
	BOOST_CHECK(!classes.find(s)->second.is_builtin());
	BOOST_CHECK(classes.find(s)->second.is_user_defined());
	BOOST_CHECK(classes.find(s)->second.declaration() == classp_5);
	s = tf.pool.normalize("Class");
	BOOST_CHECK(classes.find(s) != classes.end());
	BOOST_CHECK(!classes.find(s)->second.is_builtin());
	BOOST_CHECK(classes.find(s)->second.is_user_defined());
	BOOST_CHECK(classes.find(s)->second.declaration() == classp_6);
	s = tf.pool.normalize("builtin.Unknown");
	BOOST_CHECK(classes.find(s) == classes.end());
	s = tf.pool.normalize("Unknown");
	BOOST_CHECK(classes.find(s) == classes.end());
}


BOOST_AUTO_TEST_CASE(extract_type_info_class_name_clash)
{
	auto tf = testaux::ast_test_factory{};
	auto program = tf.factory.make<ast::program>()(
			testaux::make_unique_ptr_vector<ast::class_declaration>(
					tf.make_empty_class("Test"),
					tf.make_empty_class("My"),
					tf.make_empty_class("Class"),
					tf.make_empty_class("DoubleTrouble"),
					tf.make_empty_class("DoubleTrouble")
			)
	);
	auto classes = minijava::sem::class_definitions{};
	BOOST_CHECK_THROW(
			minijava::sem::extract_type_info(*program, false, classes),
			minijava::semantic_error
	);
	auto builtin_classes = minijava::sem::class_definitions{};
	BOOST_CHECK_THROW(
			minijava::sem::extract_type_info(*program, true, builtin_classes),
			minijava::semantic_error
	);
}


BOOST_AUTO_TEST_CASE(extract_type_info_empty)
{
	auto tf = testaux::ast_test_factory{};
	auto classes = minijava::sem::class_definitions{};
	auto program_empty = tf.factory.make<ast::program>()(
			testaux::make_unique_ptr_vector<ast::class_declaration>()
	);
	minijava::sem::extract_type_info(*program_empty, false, classes);
	auto engine = std::default_random_engine{};
	for (auto i = 0; i < 100; ++i) {
		const auto name = testaux::get_random_identifier(engine);
		BOOST_CHECK(classes.find(tf.pool.normalize(name.c_str())) == classes.end());
	}
}
