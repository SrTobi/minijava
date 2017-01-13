#include "semantic/name_type_analysis.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#define BOOST_TEST_MODULE  semantic_type_analysis
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "parser/ast.hpp"
#include "semantic/semantic_error.hpp"
#include "semantic/type_info.hpp"

#include "testaux/ast_test_factory.hpp"
#include "testaux/testaux.hpp"
#include "testaux/unique_ptr_vector.hpp"

namespace ast = minijava::ast;
namespace sem = minijava::sem;


namespace /* anonymous */
{

	struct analyzer
	{
		sem::class_definitions classes{};
		sem::globals_vector globals{};
		sem::type_attributes type_annotations{};
		sem::locals_attributes locals_annotations{};
		sem::vardecl_attributes vardecl_annotations{};
		sem::method_attributes method_annotations{};

		analyzer() {}

		explicit analyzer(const ast::program& ast) : analyzer{}
		{
			(*this)(ast);
		}

		void operator()(const ast::program& ast, const bool builtin = false)
		{
			sem::extract_type_info(ast, builtin, classes);
			sem::perform_name_type_analysis(
				ast, !builtin, classes, globals,
				type_annotations, locals_annotations,
				vardecl_annotations, method_annotations
			);
		}

	};  // struct analyzer

}  // namespace /* anonymous */


static const std::size_t some_ranks[] = {0, 1, 2, 3, 100};

static const bool false_and_true[] = {false, true};


BOOST_DATA_TEST_CASE(type_is_equal_to_self, some_ranks)
{
	using bti_type = minijava::sem::basic_type_info;
	const auto lhs = sem::type{bti_type::make_int_type(), sample};
	const auto rhs = lhs;  // deliberate copy
	BOOST_CHECK(lhs == rhs);
	BOOST_CHECK(!(lhs != rhs));
}


BOOST_DATA_TEST_CASE(type_is_not_equal_to_different_basic_type_with_same_rank, some_ranks)
{
	using bti_type = minijava::sem::basic_type_info;
	const auto lhs = sem::type{bti_type::make_int_type(), sample};
	const auto rhs = sem::type{bti_type::make_boolean_type(), sample};
	BOOST_CHECK(lhs != rhs);
	BOOST_CHECK(!(lhs == rhs));
}


BOOST_DATA_TEST_CASE(type_is_not_equal_to_type_with_different_rank, some_ranks)
{
	using bti_type = minijava::sem::basic_type_info;
	const auto lhs = sem::type{bti_type::make_int_type(), sample};
	const auto rhs = sem::type{bti_type::make_int_type(), sample + 1};
	BOOST_CHECK(lhs != rhs);
	BOOST_CHECK(!(lhs == rhs));
}


BOOST_AUTO_TEST_CASE(std_hash_no_terrible_collisions)
{
	using bti_type = minijava::sem::basic_type_info;
	auto tf = testaux::ast_test_factory{};
	const auto c1 = tf.make_empty_class();
	const auto c2 = tf.make_empty_class();
	const bti_type basics[] = {
		bti_type::make_boolean_type(),
		bti_type::make_int_type(),
		bti_type::make_null_type(),
		bti_type::make_void_type(),
		bti_type{*c1, false},
		bti_type{*c1, true},
		bti_type{*c2, false},
		bti_type{*c2, true},
	};
	const std::size_t ranks[] = {0, 1, 2, 5, 10, 20, 50, 100};
	auto hashes = std::vector<std::size_t>{};
	const auto hasher = std::hash<sem::type>{};
	for (const auto& bti : basics) {
		for (const auto& rank : ranks) {
			const auto typ = sem::type{bti, rank};
			hashes.push_back(hasher(typ));
		}
	}
	std::sort(std::begin(hashes), std::end(hashes));
	const auto pos = std::unique(std::begin(hashes), std::end(hashes));
	BOOST_REQUIRE(pos == std::end(hashes));
}


BOOST_AUTO_TEST_CASE(type_stream_insertion)
{
	using namespace std::string_literals;
	using bti_type = minijava::sem::basic_type_info;
	BOOST_CHECK_EQUAL("int"s, testaux::stream(sem::type{bti_type::make_int_type(), 0}));
	BOOST_CHECK_EQUAL("int[]"s, testaux::stream(sem::type{bti_type::make_int_type(), 1}));
	BOOST_CHECK_EQUAL("int[][]"s, testaux::stream(sem::type{bti_type::make_int_type(), 2}));
}


BOOST_AUTO_TEST_CASE(analysis_rejects_empty_program)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>()
	);
	BOOST_REQUIRE_THROW(analyzer{*ast}, minijava::semantic_error);
}


BOOST_AUTO_TEST_CASE(analysis_accepts_hello_world)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_hello_world("Test");
	const auto analysis = analyzer{*ast};
}


BOOST_AUTO_TEST_CASE(analysis_rejects_bogus_main)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.as_program(tf.make_empty_main("notmain"));
	BOOST_REQUIRE_THROW(analyzer{*ast}, minijava::semantic_error);
}


BOOST_AUTO_TEST_CASE(analysis_rejects_duplicate_main_in_same_class)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			tf.factory.make<ast::class_declaration>()(
				tf.pool.normalize("Test"),
				testaux::make_unique_ptr_vector<ast::var_decl>(),
				testaux::make_unique_ptr_vector<ast::instance_method>(),
				testaux::make_unique_ptr_vector<ast::main_method>(
					tf.make_empty_main(),
					tf.make_empty_main()
				)
			)
		)
	);
	BOOST_REQUIRE_THROW(analyzer{*ast}, minijava::semantic_error);
}


BOOST_AUTO_TEST_CASE(analysis_rejects_duplicate_main_in_different_classes)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			tf.as_class("A", tf.make_empty_main()),
			tf.as_class("B", tf.make_empty_main())
		)
	);
	BOOST_REQUIRE_THROW(analyzer{*ast}, minijava::semantic_error);
}


BOOST_AUTO_TEST_CASE(analysis_rejects_duplicate_methods_of_same_type)
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
				testaux::make_unique_ptr_vector<ast::main_method>(
					tf.make_empty_main()
				)
			)
		)
	);
	BOOST_REQUIRE_THROW(analyzer{*ast}, minijava::semantic_error);
}


BOOST_AUTO_TEST_CASE(analysis_rejects_duplicate_methods_of_different_type)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			tf.factory.make<ast::class_declaration>()(
				tf.pool.normalize("Test"),
				testaux::make_unique_ptr_vector<ast::var_decl>(),
				testaux::make_unique_ptr_vector<ast::instance_method>(
					tf.make_empty_method("foo"),
					tf.factory.make<ast::instance_method>()(
						tf.pool.normalize("foo"),
						tf.factory.make<ast::type>()(ast::primitive_type::type_int),
						testaux::make_unique_ptr_vector<ast::var_decl>(),
						tf.factory.make<ast::block>()(
								testaux::make_unique_ptr_vector<ast::block_statement>(
								tf.factory.make<ast::return_statement>()(
									tf.factory.make<ast::integer_constant>()(tf.pool.normalize("0"))
								)
							)
						)
					)
				),
				testaux::make_unique_ptr_vector<ast::main_method>(
					tf.make_empty_main()
				)
			)
		)
	);
	BOOST_REQUIRE_THROW(analyzer{*ast}, minijava::semantic_error);
}


BOOST_AUTO_TEST_CASE(analysis_accepts_methods_with_same_name_in_different_classes)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			tf.as_class("A", tf.make_empty_method("foo")),
			tf.as_class("B", tf.make_empty_method("foo")),
			tf.as_class("C", tf.make_empty_main())
		)
	);
	const auto analysis = analyzer{*ast};
}


// The MiniJava specification is unclear whether an instance method named
// 'main' should be allowed or not.  Our compiler allows this and doing so is
// the Right Thing to do.
BOOST_AUTO_TEST_CASE(analysis_accepts_instance_method_with_name_main)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			tf.factory.make<ast::class_declaration>()(
				tf.pool.normalize("Test"),
				testaux::make_unique_ptr_vector<ast::var_decl>(),
				testaux::make_unique_ptr_vector<ast::instance_method>(
					tf.make_empty_method("main")
				),
				testaux::make_unique_ptr_vector<ast::main_method>(
					tf.make_empty_main("main")
				)
			)
		)
	);
	const auto analysis = analyzer{*ast};
}


BOOST_AUTO_TEST_CASE(analysis_rejects_duplicate_fields_of_same_type)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			tf.factory.make<ast::class_declaration>()(
				tf.pool.normalize("Test"),
				testaux::make_unique_ptr_vector<ast::var_decl>(
					tf.make_declaration("foo", ast::primitive_type::type_int),
					tf.make_declaration("foo", ast::primitive_type::type_int)
				),
				testaux::make_unique_ptr_vector<ast::instance_method>(),
				testaux::make_unique_ptr_vector<ast::main_method>(
					tf.make_empty_main()
				)
			)
		)
	);
	BOOST_REQUIRE_THROW(analyzer{*ast}, minijava::semantic_error);
}


BOOST_AUTO_TEST_CASE(analysis_rejects_duplicate_fields_of_different_type)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			tf.factory.make<ast::class_declaration>()(
				tf.pool.normalize("Test"),
				testaux::make_unique_ptr_vector<ast::var_decl>(
					tf.make_declaration("foo", "Test"),
					tf.make_declaration("foo", ast::primitive_type::type_boolean)
				),
				testaux::make_unique_ptr_vector<ast::instance_method>(),
				testaux::make_unique_ptr_vector<ast::main_method>(
					tf.make_empty_main()
				)
			)
		)
	);
	BOOST_REQUIRE_THROW(analyzer{*ast}, minijava::semantic_error);
}


BOOST_AUTO_TEST_CASE(analysis_rejects_field_of_unknown_type)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			tf.factory.make<ast::class_declaration>()(
				tf.pool.normalize("Test"),
				testaux::make_unique_ptr_vector<ast::var_decl>(
					tf.make_declaration("foo", "Foo")
				),
				testaux::make_unique_ptr_vector<ast::instance_method>(),
				testaux::make_unique_ptr_vector<ast::main_method>(
					tf.make_empty_main()
				)
			)
		)
	);
	BOOST_REQUIRE_THROW(analyzer{*ast}, minijava::semantic_error);
}


BOOST_AUTO_TEST_CASE(analysis_rejects_method_of_unknown_type)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			tf.factory.make<ast::class_declaration>()(
				tf.pool.normalize("Test"),
				testaux::make_unique_ptr_vector<ast::var_decl>(),
				testaux::make_unique_ptr_vector<ast::instance_method>(
					tf.factory.make<ast::instance_method>()(
						tf.pool.normalize("getObjectOfUndeclaredType"),
						tf.factory.make<ast::type>()(tf.pool.normalize("Foo")),
						testaux::make_unique_ptr_vector<ast::var_decl>(),
						tf.factory.make<ast::block>()(
							testaux::make_unique_ptr_vector<ast::block_statement>()
						)
					)
				),
				testaux::make_unique_ptr_vector<ast::main_method>(
					tf.make_empty_main()
				)
			)
		)
	);
	BOOST_REQUIRE_THROW(analyzer{*ast}, minijava::semantic_error);
}


BOOST_AUTO_TEST_CASE(analysis_rejects_parameter_of_unknown_type)
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
							tf.make_declaration("foo", "Foo")
						),
						tf.factory.make<ast::block>()(
							testaux::make_unique_ptr_vector<ast::block_statement>()
						)
					)
				),
				testaux::make_unique_ptr_vector<ast::main_method>(
					tf.make_empty_main()
				)
			)
		)
	);
	BOOST_REQUIRE_THROW(analyzer{*ast}, minijava::semantic_error);
}


BOOST_DATA_TEST_CASE(analysis_rejects_fields_of_type_void, some_ranks)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
				tf.as_class("A", tf.make_declaration("x", ast::primitive_type::type_void, sample)),
			tf.as_class("B", tf.make_empty_main())
		)
	);
	BOOST_REQUIRE_THROW(analyzer{*ast}, minijava::semantic_error);
}


BOOST_DATA_TEST_CASE(analysis_rejects_parameters_of_type_void, some_ranks)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			tf.factory.make<ast::class_declaration>()(
				tf.pool.normalize("Test"),
				testaux::make_unique_ptr_vector<ast::var_decl>(),
				testaux::make_unique_ptr_vector<ast::instance_method>(
					tf.factory.make<ast::instance_method>()(
						tf.pool.normalize("notValid"),
						tf.factory.make<ast::type>()(ast::primitive_type::type_void),
						testaux::make_unique_ptr_vector<ast::var_decl>(
							tf.make_declaration("foo", ast::primitive_type::type_void, sample)
						),
						tf.factory.make<ast::block>()(
							testaux::make_unique_ptr_vector<ast::block_statement>()
						)
					)
				),
				testaux::make_unique_ptr_vector<ast::main_method>(tf.make_empty_main())
			)
		)
	);
	BOOST_REQUIRE_THROW(analyzer{*ast}, minijava::semantic_error);
}


BOOST_AUTO_TEST_CASE(analysis_rejects_method_of_type_void_array)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			tf.factory.make<ast::class_declaration>()(
				tf.pool.normalize("Test"),
				testaux::make_unique_ptr_vector<ast::var_decl>(),
				testaux::make_unique_ptr_vector<ast::instance_method>(
					tf.factory.make<ast::instance_method>()(
						tf.pool.normalize("questionable"),
						tf.factory.make<ast::type>()(ast::primitive_type::type_void, 1),
						testaux::make_unique_ptr_vector<ast::var_decl>(),
						tf.factory.make<ast::block>()(
							testaux::make_unique_ptr_vector<ast::block_statement>()
						)
					)
				),
				testaux::make_unique_ptr_vector<ast::main_method>(tf.make_empty_main())
			)
		)
	);
	BOOST_REQUIRE_THROW(analyzer{*ast}, minijava::semantic_error);
}


BOOST_AUTO_TEST_CASE(analysis_extracts_field_types)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			tf.factory.make<ast::class_declaration>()(
				tf.pool.normalize("Test"),
				testaux::make_unique_ptr_vector<ast::var_decl>(
					tf.make_declaration("x", ast::primitive_type::type_boolean, 7)
				),
				testaux::make_unique_ptr_vector<ast::instance_method>(),
				testaux::make_unique_ptr_vector<ast::main_method>(
					tf.make_empty_main()
				)
			)
		)
	);
	const auto analysis = analyzer{*ast};
	const auto expected_bti = sem::basic_type_info::make_boolean_type();
	const auto expected = sem::type{expected_bti, 7};
	const auto nodeptr = ast->classes().front()->fields().front().get();
	const auto actual = analysis.type_annotations.at(*nodeptr);
	BOOST_REQUIRE_EQUAL(expected, actual);
}


BOOST_AUTO_TEST_CASE(analysis_extracts_method_and_parameter_types)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			tf.factory.make<ast::class_declaration>()(
				tf.pool.normalize("Test"),
				testaux::make_unique_ptr_vector<ast::var_decl>(),
				testaux::make_unique_ptr_vector<ast::instance_method>(
					tf.factory.make<ast::instance_method>()(
						tf.pool.normalize("getTests"),
						tf.factory.make<ast::type>()(tf.pool.normalize("Test"), 1),
						testaux::make_unique_ptr_vector<ast::var_decl>(
							tf.make_declaration("n", ast::primitive_type::type_int)
						),
						tf.make_empty_block()
					)
				),
				testaux::make_unique_ptr_vector<ast::main_method>(
					tf.make_empty_main()
				)
			)
		)
	);
	const auto analysis = analyzer{*ast};
	{
		const auto expected_bti = sem::basic_type_info{*ast->classes().front(), false};
		const auto expected = sem::type{expected_bti, 1};
		const auto nodeptr = ast->classes().front()->instance_methods().front().get();
		const auto actual = analysis.type_annotations.at(*nodeptr);
		BOOST_REQUIRE_EQUAL(expected, actual);
	}
	{
		const auto expected_bti = sem::basic_type_info::make_int_type();
		const auto expected = sem::type{expected_bti, 0};
		const auto nodeptr = ast->classes().front()->instance_methods().front()
			->parameters().front().get();
		const auto actual = analysis.type_annotations.at(*nodeptr);
		BOOST_REQUIRE_EQUAL(expected, actual);
	}
}


BOOST_AUTO_TEST_CASE(analysis_extracts_main_types)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_hello_world();
	const auto analysis = analyzer{*ast};
	{
		const auto expected_bti = sem::basic_type_info::make_void_type();
		const auto expected = sem::type{expected_bti, 0};
		const auto nodeptr = ast->classes().front()->main_methods().front().get();
		const auto actual = analysis.type_annotations.at(*nodeptr);
		BOOST_REQUIRE_EQUAL(expected, actual);
	}
}


BOOST_DATA_TEST_CASE(analysis_rejects_local_variables_of_type_void, some_ranks)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.as_program(
		tf.as_block(
			tf.factory.make<ast::local_variable_statement>()(
				tf.make_declaration("x", ast::primitive_type::type_void, sample),
				std::unique_ptr<ast::expression>{}
			)
		)
	);
	auto analysis = analyzer{};
	BOOST_REQUIRE_THROW(analysis(*ast), minijava::semantic_error);
}


BOOST_AUTO_TEST_CASE(analysis_extracts_types_1st)
{
	auto tf = testaux::ast_test_factory{};
	const ast::integer_constant* nodeptr = nullptr;
	const auto ast = tf.as_program(tf.x(nodeptr, tf.make_integer("0")));
	auto analysis = analyzer{};
	analysis(*ast);
	{
		const auto expected_bti = sem::basic_type_info::make_int_type();
		const auto expected = sem::type{expected_bti, 0};
		const auto actual = analysis.type_annotations.at(*nodeptr);
		BOOST_REQUIRE_EQUAL(expected, actual);
	}
}


BOOST_AUTO_TEST_CASE(analysis_extracts_types_2nd)
{
	auto tf = testaux::ast_test_factory{};
	const ast::integer_constant* lit_0 = nullptr;
	const ast::integer_constant* lit_1 = nullptr;
	const ast::var_decl* decl_zero = nullptr;
	const ast::var_decl* decl_broken = nullptr;
	const ast::variable_access* ref_zero = nullptr;
	const ast::binary_expression* rel_expr = nullptr;
	const auto ast = tf.as_program(
		testaux::make_unique_ptr_vector<ast::block_statement>(
			tf.factory.make<ast::local_variable_statement>()(
				tf.x(decl_zero, tf.make_declaration("zero", ast::primitive_type::type_int)),
				tf.x(lit_0, tf.make_integer("0"))
			),
			tf.factory.make<ast::local_variable_statement>()(
				tf.x(decl_broken,
					 tf.make_declaration("mathIsBroken", ast::primitive_type::type_boolean)
				),
				tf.x(rel_expr,
					tf.factory.make<ast::binary_expression>()(
						ast::binary_operation_type::equal,
						tf.x(ref_zero, tf.factory.make<ast::variable_access>()(
							tf.nox(), tf.pool.normalize("zero"))
						),
						tf.x(lit_1, tf.make_integer("1"))
					)
				)
			)
		)
	);
	auto analysis = analyzer{};
	analysis(*ast);
	const auto integer = sem::type{sem::basic_type_info::make_int_type(), 0};
	const auto boolean = sem::type{sem::basic_type_info::make_boolean_type(), 0};
	BOOST_REQUIRE_EQUAL(integer, analysis.type_annotations.at(*lit_0));
	BOOST_REQUIRE_EQUAL(integer, analysis.type_annotations.at(*lit_1));
	BOOST_REQUIRE_EQUAL(integer, analysis.type_annotations.at(*decl_zero));
	BOOST_REQUIRE_EQUAL(boolean, analysis.type_annotations.at(*rel_expr));
	BOOST_REQUIRE_EQUAL(boolean, analysis.type_annotations.at(*decl_broken));
}


BOOST_AUTO_TEST_CASE(analysis_extracts_types_3rd)
{
	auto tf = testaux::ast_test_factory{};
	const minijava::ast::variable_access* p1 = nullptr;
	const minijava::ast::variable_access* p2 = nullptr;
	const auto ast = tf.factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			tf.factory.make<ast::class_declaration>()(
				tf.pool.normalize("Test"),
				testaux::make_unique_ptr_vector<ast::var_decl>(
					tf.make_declaration("test", "Test")
				),
				testaux::make_unique_ptr_vector<ast::instance_method>(
					tf.factory.make<ast::instance_method>()(
						tf.pool.normalize("test"),
						tf.factory.make<ast::type>()(ast::primitive_type::type_void),
						testaux::make_unique_ptr_vector<ast::var_decl>(
							tf.make_declaration("test", ast::primitive_type::type_int)
						),
						tf.factory.make<ast::block>()(
							testaux::make_unique_ptr_vector<ast::block_statement>(
								tf.factory.make<ast::expression_statement>()(
									tf.x(p1, tf.make_idref("test"))
								),
								tf.factory.make<ast::expression_statement>()(
									tf.x(p2, tf.make_idref_this("test"))
								)
							)
						)
					)
				),
				testaux::make_unique_ptr_vector<ast::main_method>(
					tf.make_empty_main()
				)
			)
		)
	);
	auto analysis = analyzer{};
	analysis(*ast);
	const auto test_class = ast->classes().front().get();
	const auto test_field = test_class->fields().front().get();
	const auto test_meth = test_class->instance_methods().front().get();
	const auto test_param = test_meth->parameters().front().get();
	const auto type_test = sem::type{sem::basic_type_info{*test_class, false}, 0};
	const auto type_int = sem::type{sem::basic_type_info::make_int_type(), 0};
	const auto type_void = sem::type{sem::basic_type_info::make_void_type(), 0};
	BOOST_REQUIRE_EQUAL(type_void, analysis.type_annotations.at(*test_meth));
	BOOST_REQUIRE_EQUAL(type_test, analysis.type_annotations.at(*test_field));
	BOOST_REQUIRE_EQUAL(type_int, analysis.type_annotations.at(*test_param));
	BOOST_REQUIRE_EQUAL(type_int, analysis.type_annotations.at(*p1));
	BOOST_REQUIRE_EQUAL(type_test, analysis.type_annotations.at(*p2));
	BOOST_REQUIRE(test_param == analysis.vardecl_annotations.at(*p1));
	BOOST_REQUIRE(test_field == analysis.vardecl_annotations.at(*p2));
}


BOOST_AUTO_TEST_CASE(analysis_rejects_access_to_undefined_variable_in_main)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.as_program(tf.make_idref("undefined"));
	auto analysis = analyzer{};
	BOOST_REQUIRE_THROW(analysis(*ast), minijava::semantic_error);
}


BOOST_AUTO_TEST_CASE(analysis_rejects_access_to_undefined_variable)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.as_program(
		tf.factory.make<ast::class_declaration>()(
			tf.pool.normalize("Test"),
			testaux::make_unique_ptr_vector<ast::var_decl>(),
			testaux::make_unique_ptr_vector<ast::instance_method>(
				tf.factory.make<ast::instance_method>()(
					tf.pool.normalize("useUndefined"),
					tf.factory.make<ast::type>()(ast::primitive_type::type_void),
					testaux::make_unique_ptr_vector<ast::var_decl>(),
					tf.as_block(tf.make_idref_this("undefined"))
				)
			),
			testaux::make_unique_ptr_vector<ast::main_method>(tf.make_empty_main())
		)
	);
	auto analysis = analyzer{};
	BOOST_REQUIRE_THROW(analysis(*ast), minijava::semantic_error);
}


BOOST_AUTO_TEST_CASE(analysis_rejects_call_of_undefined_method_in_main)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.as_program(tf.make_call("undefined"));
	auto analysis = analyzer{};
	BOOST_REQUIRE_THROW(analysis(*ast), minijava::semantic_error);
}


BOOST_AUTO_TEST_CASE(analysis_rejects_call_of_undefined_method)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.as_program(
		tf.factory.make<ast::class_declaration>()(
			tf.pool.normalize("Test"),
			testaux::make_unique_ptr_vector<ast::var_decl>(),
			testaux::make_unique_ptr_vector<ast::instance_method>(
				tf.factory.make<ast::instance_method>()(
					tf.pool.normalize("callUndefined"),
					tf.factory.make<ast::type>()(ast::primitive_type::type_void),
					testaux::make_unique_ptr_vector<ast::var_decl>(),
					tf.as_block(tf.make_call_this("undefined"))
				)
			),
			testaux::make_unique_ptr_vector<ast::main_method>(tf.make_empty_main())
		)
	);
	auto analysis = analyzer{};
	BOOST_REQUIRE_THROW(analysis(*ast), minijava::semantic_error);
}


BOOST_AUTO_TEST_CASE(analysis_rejects_access_to_args_in_main)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.as_program(
		tf.factory.make<ast::class_declaration>()(
			tf.pool.normalize("Test"),
			testaux::make_unique_ptr_vector<ast::var_decl>(),
			testaux::make_unique_ptr_vector<ast::instance_method>(),
			testaux::make_unique_ptr_vector<ast::main_method>(
				tf.factory.make<ast::main_method>()(
					tf.pool.normalize("main"),
					tf.pool.normalize("toxic"),  // deliberately not 'args'
					tf.as_block(tf.make_idref("quacks"))
				)
			)
		)
	);
	auto analysis = analyzer{};
	BOOST_REQUIRE_THROW(analysis(*ast), minijava::semantic_error);
}


BOOST_AUTO_TEST_CASE(analysis_rejects_access_to_fields_from_main)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.as_program(
		tf.factory.make<ast::class_declaration>()(
			tf.pool.normalize("Test"),
			testaux::make_unique_ptr_vector<ast::var_decl>(
				tf.make_declaration("field", ast::primitive_type::type_int)
			),
			testaux::make_unique_ptr_vector<ast::instance_method>(),
			testaux::make_unique_ptr_vector<ast::main_method>(
				tf.factory.make<ast::main_method>()(
					tf.pool.normalize("main"),
					tf.pool.normalize("args"),
					tf.as_block(tf.make_idref("field"))
				)
			)
		)
	);
	auto analysis = analyzer{};
	BOOST_REQUIRE_THROW(analysis(*ast), minijava::semantic_error);
}


BOOST_AUTO_TEST_CASE(analysis_accepts_args_that_is_not_args_in_main)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.as_program(
		tf.factory.make<ast::class_declaration>()(
			tf.pool.normalize("Test"),
			testaux::make_unique_ptr_vector<ast::var_decl>(),
			testaux::make_unique_ptr_vector<ast::instance_method>(),
			testaux::make_unique_ptr_vector<ast::main_method>(
				tf.factory.make<ast::main_method>()(
					tf.pool.normalize("main"),
					tf.pool.normalize("toxic"),
					tf.as_block(
						tf.factory.make<ast::local_variable_statement>()(
							tf.make_declaration("args", ast::primitive_type::type_int),
							tf.make_integer("0")
						)
					)
				)
			)
		)
	);
	auto analysis = analyzer{};
	analysis(*ast);
}


BOOST_DATA_TEST_CASE(analysis_rejects_assignment_to_global_but_allows_comparison, false_and_true)
{
	auto tf = testaux::ast_test_factory{};
	const auto builtin_ast = tf.as_program(tf.make_empty_class("Global"));
	auto analysis = analyzer{};
	analysis(*builtin_ast, true);
	analysis.globals.push_back(
		tf.factory.make<ast::var_decl>()(
			tf.factory.make<ast::type>()(tf.pool.normalize("Global")),
			tf.pool.normalize("global")
		)
	);
	const auto binary_operation = sample
		? ast::binary_operation_type::equal
		: ast::binary_operation_type::assign;
	const auto ast = tf.as_program(
		tf.factory.make<ast::expression_statement>()(
			tf.factory.make<ast::binary_expression>()(
				binary_operation,
				tf.make_idref("global"),
				tf.factory.make<ast::null_constant>()()
			)
		)
	);
	if (sample) {
		BOOST_REQUIRE_NO_THROW(analysis(*ast, false));
	} else {
		BOOST_REQUIRE_THROW(analysis(*ast, false), minijava::semantic_error);
	}
}


BOOST_AUTO_TEST_CASE(analysis_extracts_locals_correctly)
{
	for (auto n = 0; n < 10; ++n) {
		for (auto m = 0; m < 10; ++m) {
			auto tf = testaux::ast_test_factory{};
			auto paramdecls = std::vector<std::unique_ptr<ast::var_decl>>{};
			for (auto i = 1; i <= n; ++i) {
				const auto id = "p" + std::to_string(i);
				paramdecls.push_back(tf.make_declaration(id, ast::primitive_type::type_int));
			}
			auto blkstmts = std::vector<std::unique_ptr<ast::block_statement>>{};
			for (auto i = 1; i <= m; ++i) {
				const auto id = "v" + std::to_string(i);
				blkstmts.push_back(
					tf.factory.make<ast::local_variable_statement>()(
						tf.factory.make<ast::var_decl>()(
							tf.factory.make<ast::type>()(ast::primitive_type::type_int),
							tf.pool.normalize(id)
						),
						tf.nox()
					)
				);
			}
			blkstmts.push_back(
				tf.factory.make<ast::return_statement>()(tf.make_integer("0"))
			);
			const auto ast = tf.as_program(
				tf.factory.make<ast::class_declaration>()(
					tf.pool.normalize("Test"),
					testaux::make_unique_ptr_vector<ast::var_decl>(),
					testaux::make_unique_ptr_vector<ast::instance_method>(
						tf.factory.make<ast::instance_method>()(
							tf.pool.normalize("test"),
							tf.factory.make<ast::type>()(ast::primitive_type::type_int),
								std::move(paramdecls),
								tf.factory.make<ast::block>()(std::move(blkstmts))
							)
					),
					testaux::make_unique_ptr_vector<ast::main_method>(
						tf.make_empty_main()
					)
				)
			);
			auto analysis = analyzer{*ast};
			{
				const auto nodeptr = ast->classes().front()->instance_methods().front().get();
				BOOST_REQUIRE_EQUAL(n + m, analysis.locals_annotations.at(*nodeptr).size());
			}
			{
				const auto nodeptr = ast->classes().front()->main_methods().front().get();
				BOOST_REQUIRE_EQUAL(0, analysis.locals_annotations.at(*nodeptr).size());
			}
		}
	}
}


BOOST_AUTO_TEST_CASE(analysis_sets_method_annotations_correctly)
{
	using namespace std::string_literals;
	auto tf = testaux::ast_test_factory{};
	const ast::method_invocation* p1 = nullptr;
	const ast::method_invocation* p2 = nullptr;
	const auto ast = tf.factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			tf.factory.make<ast::class_declaration>()(
				tf.pool.normalize("Alpha"),
				testaux::make_unique_ptr_vector<ast::var_decl>(),
				testaux::make_unique_ptr_vector<ast::instance_method>(
					tf.factory.make<ast::instance_method>()(
						tf.pool.normalize("foo"),
						tf.factory.make<ast::type>()(ast::primitive_type::type_void),
						testaux::make_unique_ptr_vector<ast::var_decl>(),
						tf.as_block(tf.x(p1, tf.make_call("foo")))
					)
				),
				testaux::make_unique_ptr_vector<ast::main_method>()
			),
			tf.factory.make<ast::class_declaration>()(
				tf.pool.normalize("Beta"),
				testaux::make_unique_ptr_vector<ast::var_decl>(),
				testaux::make_unique_ptr_vector<ast::instance_method>(
					tf.make_empty_method("bar"),
					tf.make_empty_method("baz")
				),
				testaux::make_unique_ptr_vector<ast::main_method>(
					tf.factory.make<ast::main_method>()(
						tf.pool.normalize("main"),
						tf.pool.normalize("args"),
						tf.as_block(
							tf.x(p2,
								tf.factory.make<ast::method_invocation>()(
									tf.factory.make<ast::object_instantiation>()(tf.pool.normalize("Beta")),
									tf.pool.normalize("bar"),
									testaux::make_unique_ptr_vector<ast::expression>()
								)
							)
						)
					)
				)
			)
		)
	);
	const auto analysis = analyzer{*ast};
	BOOST_REQUIRE_EQUAL("foo"s, analysis.method_annotations.at(*p1)->name());
	BOOST_REQUIRE_EQUAL("bar"s, analysis.method_annotations.at(*p2)->name());
}

// TODO: Write unit tests for
//
//  - all kinds of annotations
//  - new expressions
//  - tests for (non-)required main
//  - tests for type checks on assignments, operations and returns
//  - lvalue / assignable tests
