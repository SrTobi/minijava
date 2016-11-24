#include "semantic/name_type_analysis.hpp"

#include <memory>
#include <vector>

#define BOOST_TEST_MODULE  semantic_type_analysis
#include <boost/test/unit_test.hpp>

#include "parser/ast.hpp"
#include "parser/ast_factory.hpp"
#include "semantic/semantic_error.hpp"
#include "symbol/symbol_pool.hpp"

#include "testaux/ast_test_factory.cpp"
#include "testaux/unique_ptr_vector.hpp"

namespace ast = minijava::ast;
namespace sem = minijava::sem;


BOOST_AUTO_TEST_CASE(shallow_rejects_empty_program)
{
	auto factory = minijava::ast_factory{};
	const auto ast = factory.make<ast::program>()(
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
	auto pool = minijava::symbol_pool<>{};
	auto factory = minijava::ast_factory{};
	const auto ast = testaux::make_hello_world("Test", pool, factory);
	auto classes = sem::class_definitions{};
	auto type_annotations = sem::type_attributes{};
	sem::extract_type_info(*ast, false, classes);
	sem::perform_shallow_type_analysis(*ast, classes, type_annotations);
}


BOOST_AUTO_TEST_CASE(shallow_rejects_bogus_main)
{
	auto pool = minijava::symbol_pool<>{};
	auto factory = minijava::ast_factory{};
	const auto ast = testaux::as_program(
		testaux::make_empty_main("notmain", pool, factory),
		pool, factory
	);
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
	auto pool = minijava::symbol_pool<>{};
	auto factory = minijava::ast_factory{};
	const auto ast = factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			factory.make<ast::class_declaration>()(
				pool.normalize("Test"),
				testaux::make_unique_ptr_vector<ast::var_decl>(),
				testaux::make_unique_ptr_vector<ast::instance_method>(),
				testaux::make_unique_ptr_vector<ast::main_method>(
					testaux::make_empty_main("main", pool, factory),
					testaux::make_empty_main("main", pool, factory)
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
	auto pool = minijava::symbol_pool<>{};
	auto factory = minijava::ast_factory{};
	const auto ast = factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			testaux::as_class("A", testaux::make_empty_main("main", pool, factory), pool, factory),
			testaux::as_class("B", testaux::make_empty_main("main", pool, factory), pool, factory)
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
	auto pool = minijava::symbol_pool<>{};
	auto factory = minijava::ast_factory{};
	const auto ast = factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			factory.make<ast::class_declaration>()(
				pool.normalize("Test"),
				testaux::make_unique_ptr_vector<ast::var_decl>(),
				testaux::make_unique_ptr_vector<ast::instance_method>(
					testaux::make_empty_method("foo", pool, factory),
					testaux::make_empty_method("foo", pool, factory)
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
	auto pool = minijava::symbol_pool<>{};
	auto factory = minijava::ast_factory{};
	const auto ast = factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			factory.make<ast::class_declaration>()(
				pool.normalize("Test"),
				testaux::make_unique_ptr_vector<ast::var_decl>(
					factory.make<ast::var_decl>()(
						factory.make<ast::type>()(ast::primitive_type::type_int),
						pool.normalize("foo")
					),
					factory.make<ast::var_decl>()(
						factory.make<ast::type>()(ast::primitive_type::type_int),
						pool.normalize("foo")
					)
				),
				testaux::make_unique_ptr_vector<ast::instance_method>(),
				testaux::make_unique_ptr_vector<ast::main_method>(
					testaux::make_empty_main("main", pool, factory)
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
	auto pool = minijava::symbol_pool<>{};
	auto factory = minijava::ast_factory{};
	const auto ast = factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			factory.make<ast::class_declaration>()(
				pool.normalize("Test"),
				testaux::make_unique_ptr_vector<ast::var_decl>(
					factory.make<ast::var_decl>()(
						factory.make<ast::type>()(pool.normalize("Test")),
						pool.normalize("foo")
					),
					factory.make<ast::var_decl>()(
						factory.make<ast::type>()(ast::primitive_type::type_boolean),
						pool.normalize("foo")
					)
				),
				testaux::make_unique_ptr_vector<ast::instance_method>(),
				testaux::make_unique_ptr_vector<ast::main_method>(
					testaux::make_empty_main("main", pool, factory)
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
	auto pool = minijava::symbol_pool<>{};
	auto factory = minijava::ast_factory{};
	const auto ast = factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			factory.make<ast::class_declaration>()(
				pool.normalize("Test"),
				testaux::make_unique_ptr_vector<ast::var_decl>(
					factory.make<ast::var_decl>()(
						factory.make<ast::type>()(pool.normalize("Foo")),
						pool.normalize("foo")
					)
				),
				testaux::make_unique_ptr_vector<ast::instance_method>(),
				testaux::make_unique_ptr_vector<ast::main_method>(
					testaux::make_empty_main("main", pool, factory)
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
	auto pool = minijava::symbol_pool<>{};
	auto factory = minijava::ast_factory{};
	const auto ast = factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			factory.make<ast::class_declaration>()(
				pool.normalize("Test"),
				testaux::make_unique_ptr_vector<ast::var_decl>(),
				testaux::make_unique_ptr_vector<ast::instance_method>(
					factory.make<ast::instance_method>()(
						pool.normalize("getObjectOfUdeclaredType"),
						factory.make<ast::type>()(pool.normalize("Foo")),
						testaux::make_unique_ptr_vector<ast::var_decl>(),
						factory.make<ast::block>()(
							testaux::make_unique_ptr_vector<ast::block_statement>()
						)
					)
				),
				testaux::make_unique_ptr_vector<ast::main_method>(
					testaux::make_empty_main("main", pool, factory)
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
	auto pool = minijava::symbol_pool<>{};
	auto factory = minijava::ast_factory{};
	const auto ast = factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			factory.make<ast::class_declaration>()(
				pool.normalize("Test"),
				testaux::make_unique_ptr_vector<ast::var_decl>(),
				testaux::make_unique_ptr_vector<ast::instance_method>(
					factory.make<ast::instance_method>()(
						pool.normalize("processObjectOfUdeclaredType"),
						factory.make<ast::type>()(ast::primitive_type::type_void),
						testaux::make_unique_ptr_vector<ast::var_decl>(
							factory.make<ast::var_decl>()(
								factory.make<ast::type>()(pool.normalize("Foo")),
								pool.normalize("foo")
							)
						),
						factory.make<ast::block>()(
							testaux::make_unique_ptr_vector<ast::block_statement>()
						)
					)
				),
				testaux::make_unique_ptr_vector<ast::main_method>(
					testaux::make_empty_main("main", pool, factory)
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
