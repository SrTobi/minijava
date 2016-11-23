#include "semantic/type_info.hpp"

#include <algorithm>
#include <memory>
#include <random>
#include <string>
#include <vector>

#define BOOST_TEST_MODULE  semantic_type_info
#include <boost/test/unit_test.hpp>

#include "symbol/symbol_pool.hpp"

#include "testaux/random_tokens.hpp"
#include "testaux/unique_ptr_vector.hpp"

namespace ast = minijava::ast;


namespace /* anonymous */
{

	template <typename PoolT>
	std::unique_ptr<ast::class_declaration>
	make_class_decl(PoolT& pool, const std::string& name)
	{
		return std::make_unique<ast::class_declaration>(
			pool.normalize(name),
			testaux::make_unique_ptr_vector<ast::var_decl>(),
			testaux::make_unique_ptr_vector<ast::instance_method>(),
			testaux::make_unique_ptr_vector<ast::main_method>()
		);
	}

	template <typename EngineT, typename PoolT>
	std::unique_ptr<ast::class_declaration>
	make_random_class_decl(EngineT& engine, PoolT& pool)
	{
		const auto name = testaux::get_random_identifier(engine);
		return make_class_decl(pool, name);
	}

}  // namespace /* anonymous */

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
	auto pool = minijava::symbol_pool<>{};
	auto clazz = make_class_decl(pool, "Elephant");
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
	auto pool = minijava::symbol_pool<>{};
	auto clazz = make_class_decl(pool, "Elephant");
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
	auto btis = std::vector<minijava::sem::basic_type_info>{
		minijava::sem::basic_type_info::make_null_type(),
		minijava::sem::basic_type_info::make_void_type(),
		minijava::sem::basic_type_info::make_int_type(),
		minijava::sem::basic_type_info::make_boolean_type(),
	};
	auto pool = minijava::symbol_pool<>{};
	auto classdecls = std::vector<std::unique_ptr<ast::class_declaration>>{};
	auto engine = std::default_random_engine{};
	auto builtindist = std::bernoulli_distribution{};
	std::generate_n(
		std::back_inserter(classdecls),
		100,
		[&engine, &pool](){ return make_random_class_decl(engine, pool); }
	);
	std::transform(
		std::begin(classdecls), std::end(classdecls),
		std::back_inserter(btis),
		[&engine, &builtindist](auto&& clsdcl){
			return minijava::sem::basic_type_info{*clsdcl, builtindist(engine)};
		}
	);
	for (auto it = std::begin(btis); it != std::end(btis); ++it) {
		const auto isme = [it](auto&& you){ return *it == you; };
		const auto clones = std::vector<minijava::sem::basic_type_info>(10, *it);
		BOOST_REQUIRE(std::all_of(std::begin(clones), std::end(clones), isme));
		for (auto jt = std::begin(btis); jt != std::end(btis); ++jt) {
			BOOST_REQUIRE_EQUAL((it == jt), (*it == *jt));
		}
	}
}
