#include "semantic/name_type_analysis.hpp"

#include <memory>
#include <vector>

#define BOOST_TEST_MODULE  semantic_type_analysis
#include <boost/test/unit_test.hpp>

#include "parser/ast.hpp"
#include "semantic/semantic_error.hpp"

#include "testaux/ast_test_factory.cpp"
#include "testaux/unique_ptr_vector.hpp"

namespace ast = minijava::ast;
namespace sem = minijava::sem;


BOOST_AUTO_TEST_CASE(shallow_rejects_empty_program)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>()
	);
	auto classes = sem::class_definitions{};
	auto type_annotations = sem::type_attributes{};
	sem::extract_type_info(*ast, false, classes);
	BOOST_REQUIRE_THROW(
		sem::perform_shallow_type_analysis(*ast, classes, type_annotations),
		minijava::semantic_error
	);
}


BOOST_AUTO_TEST_CASE(shallow_accepts_hello_world)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_hello_world("Test");
	auto classes = sem::class_definitions{};
	auto type_annotations = sem::type_attributes{};
	sem::extract_type_info(*ast, false, classes);
	sem::perform_shallow_type_analysis(*ast, classes, type_annotations);
}


BOOST_AUTO_TEST_CASE(shallow_rejects_bogus_main)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.as_program(tf.make_empty_main("notmain"));
	auto classes = sem::class_definitions{};
	auto type_annotations = sem::type_attributes{};
	sem::extract_type_info(*ast, false, classes);
	BOOST_REQUIRE_THROW(
		sem::perform_shallow_type_analysis(*ast, classes, type_annotations),
		minijava::semantic_error
	);
}


BOOST_AUTO_TEST_CASE(shallow_rejects_duplicate_main_in_same_class)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			tf.factory.make<ast::class_declaration>()(
				tf.pool.normalize("Test"),
				testaux::make_unique_ptr_vector<ast::var_decl>(),
				testaux::make_unique_ptr_vector<ast::instance_method>(),
				testaux::make_unique_ptr_vector<ast::main_method>(
					tf.make_empty_main("main"),
					tf.make_empty_main("main")
				)
			)
		)
	);
	auto classes = sem::class_definitions{};
	auto type_annotations = sem::type_attributes{};
	sem::extract_type_info(*ast, false, classes);
	BOOST_REQUIRE_THROW(
		sem::perform_shallow_type_analysis(*ast, classes, type_annotations),
		minijava::semantic_error
	);
}


BOOST_AUTO_TEST_CASE(shallow_rejects_duplicate_main_in_different_classes)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			tf.as_class("A", tf.make_empty_main("main")),
			tf.as_class("B", tf.make_empty_main("main"))
		)
	);
	auto classes = sem::class_definitions{};
	auto type_annotations = sem::type_attributes{};
	sem::extract_type_info(*ast, false, classes);
	BOOST_REQUIRE_THROW(
		sem::perform_shallow_type_analysis(*ast, classes, type_annotations),
		minijava::semantic_error
	);
}


BOOST_AUTO_TEST_CASE(shallow_rejects_duplicate_methods)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			tf.factory.make<ast::class_declaration>()(
				tf.pool.normalize("Test"),
				testaux::make_unique_ptr_vector<ast::var_decl>(),
				testaux::make_unique_ptr_vector<ast::instance_method>(
					tf.make_empty_method("foo"),
					tf.make_empty_method("foo")
				),
				testaux::make_unique_ptr_vector<ast::main_method>()
			)
		)
	);
	auto classes = sem::class_definitions{};
	auto type_annotations = sem::type_attributes{};
	sem::extract_type_info(*ast, false, classes);
	BOOST_REQUIRE_THROW(
		sem::perform_shallow_type_analysis(*ast, classes, type_annotations),
		minijava::semantic_error
	);
}


BOOST_AUTO_TEST_CASE(shallow_rejects_duplicate_fields_of_same_type)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			tf.factory.make<ast::class_declaration>()(
				tf.pool.normalize("Test"),
				testaux::make_unique_ptr_vector<ast::var_decl>(
					tf.factory.make<ast::var_decl>()(
						tf.factory.make<ast::type>()(ast::primitive_type::type_int),
						tf.pool.normalize("foo")
					),
					tf.factory.make<ast::var_decl>()(
						tf.factory.make<ast::type>()(ast::primitive_type::type_int),
						tf.pool.normalize("foo")
					)
				),
				testaux::make_unique_ptr_vector<ast::instance_method>(),
				testaux::make_unique_ptr_vector<ast::main_method>(
					tf.make_empty_main("main")
				)
			)
		)
	);
	auto classes = sem::class_definitions{};
	auto type_annotations = sem::type_attributes{};
	sem::extract_type_info(*ast, false, classes);
	BOOST_REQUIRE_THROW(
		sem::perform_shallow_type_analysis(*ast, classes, type_annotations),
		minijava::semantic_error
	);
}


BOOST_AUTO_TEST_CASE(shallow_rejects_duplicate_fields_of_different_type)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			tf.factory.make<ast::class_declaration>()(
				tf.pool.normalize("Test"),
				testaux::make_unique_ptr_vector<ast::var_decl>(
					tf.factory.make<ast::var_decl>()(
						tf.factory.make<ast::type>()(tf.pool.normalize("Test")),
						tf.pool.normalize("foo")
					),
					tf.factory.make<ast::var_decl>()(
						tf.factory.make<ast::type>()(ast::primitive_type::type_boolean),
						tf.pool.normalize("foo")
					)
				),
				testaux::make_unique_ptr_vector<ast::instance_method>(),
				testaux::make_unique_ptr_vector<ast::main_method>(
					tf.make_empty_main("main")
				)
			)
		)
	);
	auto classes = sem::class_definitions{};
	auto type_annotations = sem::type_attributes{};
	sem::extract_type_info(*ast, false, classes);
	BOOST_REQUIRE_THROW(
		sem::perform_shallow_type_analysis(*ast, classes, type_annotations),
		minijava::semantic_error
	);
}


BOOST_AUTO_TEST_CASE(shallow_rejects_field_of_unknown_type)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			tf.factory.make<ast::class_declaration>()(
				tf.pool.normalize("Test"),
				testaux::make_unique_ptr_vector<ast::var_decl>(
					tf.factory.make<ast::var_decl>()(
						tf.factory.make<ast::type>()(tf.pool.normalize("Foo")),
						tf.pool.normalize("foo")
					)
				),
				testaux::make_unique_ptr_vector<ast::instance_method>(),
				testaux::make_unique_ptr_vector<ast::main_method>(
					tf.make_empty_main("main")
				)
			)
		)
	);
	auto classes = sem::class_definitions{};
	auto type_annotations = sem::type_attributes{};
	sem::extract_type_info(*ast, false, classes);
	BOOST_REQUIRE_THROW(
		sem::perform_shallow_type_analysis(*ast, classes, type_annotations),
		minijava::semantic_error
	);
}


BOOST_AUTO_TEST_CASE(shallow_rejects_method_of_unknown_type)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			tf.factory.make<ast::class_declaration>()(
				tf.pool.normalize("Test"),
				testaux::make_unique_ptr_vector<ast::var_decl>(),
				testaux::make_unique_ptr_vector<ast::instance_method>(
					tf.factory.make<ast::instance_method>()(
						tf.pool.normalize("getObjectOfUdeclaredType"),
						tf.factory.make<ast::type>()(tf.pool.normalize("Foo")),
						testaux::make_unique_ptr_vector<ast::var_decl>(),
						tf.factory.make<ast::block>()(
							testaux::make_unique_ptr_vector<ast::block_statement>()
						)
					)
				),
				testaux::make_unique_ptr_vector<ast::main_method>(
					tf.make_empty_main("main")
				)
			)
		)
	);
	auto classes = sem::class_definitions{};
	auto type_annotations = sem::type_attributes{};
	sem::extract_type_info(*ast, false, classes);
	BOOST_REQUIRE_THROW(
		sem::perform_shallow_type_analysis(*ast, classes, type_annotations),
		minijava::semantic_error
	);
}


BOOST_AUTO_TEST_CASE(shallow_rejects_parameter_of_unknown_type)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			tf.factory.make<ast::class_declaration>()(
				tf.pool.normalize("Test"),
				testaux::make_unique_ptr_vector<ast::var_decl>(),
				testaux::make_unique_ptr_vector<ast::instance_method>(
					tf.factory.make<ast::instance_method>()(
						tf.pool.normalize("processObjectOfUdeclaredType"),
						tf.factory.make<ast::type>()(ast::primitive_type::type_void),
						testaux::make_unique_ptr_vector<ast::var_decl>(
							tf.factory.make<ast::var_decl>()(
								tf.factory.make<ast::type>()(tf.pool.normalize("Foo")),
								tf.pool.normalize("foo")
							)
						),
						tf.factory.make<ast::block>()(
							testaux::make_unique_ptr_vector<ast::block_statement>()
						)
					)
				),
				testaux::make_unique_ptr_vector<ast::main_method>(
					tf.make_empty_main("main")
				)
			)
		)
	);
	auto classes = sem::class_definitions{};
	auto type_annotations = sem::type_attributes{};
	sem::extract_type_info(*ast, false, classes);
	BOOST_REQUIRE_THROW(
		sem::perform_shallow_type_analysis(*ast, classes, type_annotations),
		minijava::semantic_error
	);
}
