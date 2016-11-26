#include "semantic/semantic.hpp"

#include <string>
#include <type_traits>

#define BOOST_TEST_MODULE  semantic_semantic
#include <boost/test/unit_test.hpp>

#include "testaux/ast_test_factory.hpp"
#include "testaux/unique_ptr_vector.hpp"

namespace ast = minijava::ast;
using namespace std::string_literals;


BOOST_AUTO_TEST_CASE(semantic_info_type_sanity_checks)
{
	// Default-construct the parameters using AAA syntax.
	auto classes = minijava::semantic_info::class_definitions{};
	auto type_annotations = minijava::semantic_info::type_attributes{};
	auto locals_annotations = minijava::semantic_info::locals_attributes{};
	auto vardecl_annotations = minijava::semantic_info::vardecl_attributes{};
	auto method_annotations = minijava::semantic_info::method_attributes{};
	auto const_annotations = minijava::semantic_info::const_attributes{};
	// The following two are just implementation detauls.
	auto builtin_ast = std::make_unique<ast::program>(
		testaux::make_unique_ptr_vector<ast::class_declaration>()
	);
	auto globals = testaux::make_unique_ptr_vector<ast::var_decl>();
	// Construct the result object moving the containers in.
	const auto info = minijava::semantic_info{
		std::move(classes),
		std::move(type_annotations),
		std::move(locals_annotations),
		std::move(vardecl_annotations),
		std::move(method_annotations),
		std::move(const_annotations),
		std::move(builtin_ast),
		std::move(globals)
	};
	// Obtain them again, making copies.
	classes = info.classes();
	type_annotations = info.type_annotations();
	locals_annotations = info.locals_annotations();
	vardecl_annotations = info.vardecl_annotations();
	method_annotations = info.method_annotations();
	const_annotations = info.const_annotations();
}


BOOST_AUTO_TEST_CASE(check_program_hello_world)
{
	auto tf = testaux::ast_test_factory{};
	const ast::integer_constant* p_42 = nullptr;
	const ast::method_invocation* p_invoke = nullptr;
	const ast::variable_access* p_system = nullptr;
	const ast::variable_access* p_out = nullptr;
	const auto ast = tf.as_program(
		tf.x(
			p_invoke,
			tf.factory.make<ast::method_invocation>()(
				tf.x(
					p_out,
					tf.factory.make<ast::variable_access>()(
						tf.x(p_system, tf.make_idref("System")),
						tf.pool.normalize("out")
					)
				),
				tf.pool.normalize("println"),
				testaux::make_unique_ptr_vector<ast::expression>(
					tf.x(p_42, tf.make_literal("42"))
				)
			)
		)
	);
	const auto p_main = ast->classes().front()->main_methods().front().get();
	const auto info = minijava::check_program(*ast, tf.pool, tf.factory);
	const auto bt_void = minijava::sem::basic_type_info::make_void_type();
	// Check classes
	const auto test_bti = info.classes().at(tf.pool.normalize("Test"));
	BOOST_CHECK(test_bti.is_user_defined());
	BOOST_CHECK_EQUAL(ast->classes().front().get(), test_bti.declaration());
	// Check type annotations
	BOOST_CHECK_EQUAL(
		minijava::sem::type(bt_void, 0),
		info.type_annotations().at(*p_main)
	);
	BOOST_CHECK_EQUAL(
		minijava::sem::type(bt_void, 0),
		info.type_annotations().at(*p_invoke)
	);
	const auto jlsys_bti = info.classes().at(tf.pool.normalize("java.lang.System"));
	const auto jiops_bti = info.classes().at(tf.pool.normalize("java.io.PrintStream"));
	BOOST_CHECK(jlsys_bti.is_builtin() && jiops_bti.is_builtin());
	BOOST_CHECK_EQUAL(jlsys_bti, info.type_annotations().at(*p_system).info);
	BOOST_CHECK_EQUAL(jiops_bti, info.type_annotations().at(*p_out).info);
	// Check vardecl annotations
	const auto p_sysdecl = info.vardecl_annotations().at(*p_system);
	const auto p_outdecl = info.vardecl_annotations().at(*p_out);
	BOOST_CHECK(p_sysdecl != nullptr);
	BOOST_CHECK(p_outdecl != nullptr);
	BOOST_CHECK(info.is_global(p_sysdecl));
	BOOST_CHECK(!info.is_global(p_outdecl));
	BOOST_CHECK_EQUAL(
		"java.lang.System"s,
		boost::get<minijava::symbol>(p_sysdecl->var_type().name())
	);
	BOOST_CHECK_EQUAL(
		"java.io.PrintStream"s,
		boost::get<minijava::symbol>(p_outdecl->var_type().name())
	);
	// Check method annotations
	const auto p_println = info.method_annotations().at(*p_invoke);
	BOOST_CHECK_EQUAL("println"s, p_println->name());
	// Check locals annotation
	BOOST_CHECK_EQUAL(0, info.locals_annotations().at(*p_main).size());
	BOOST_CHECK_GE(1, info.locals_annotations().at(*p_println).size());
	// Check const annotations
	BOOST_CHECK_EQUAL(42, info.const_annotations().at(*p_42));
}


BOOST_AUTO_TEST_CASE(check_invalid_program)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.as_program(tf.make_call("undefined"));
	BOOST_REQUIRE_THROW(
		minijava::check_program(*ast, tf.pool, tf.factory),
		minijava::semantic_error
	);
}
