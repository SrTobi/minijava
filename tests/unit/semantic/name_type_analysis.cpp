#include "semantic/name_type_analysis.hpp"

#include <memory>
#include <vector>

#define BOOST_TEST_MODULE  semantic_type_analysis
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include <boost/lexical_cast.hpp>

#include "parser/ast.hpp"
#include "semantic/semantic_error.hpp"
#include "semantic/type_info.hpp"

#include "testaux/ast_test_factory.cpp"
#include "testaux/unique_ptr_vector.hpp"

namespace ast = minijava::ast;
namespace sem = minijava::sem;


namespace /* anonymous */
{

	struct full_analysis
	{
		sem::class_definitions classes{};
		sem::globals_vector globals{};
		sem::type_attributes type_annotations{};
		sem::locals_attributes locals_annotations{};
		sem::vardecl_attributes vardecl_annotations{};
		sem::method_attributes method_annotations{};

		full_analysis() {}

		explicit full_analysis(const ast::program& ast) : full_analysis{}
		{
			(*this)(ast);
		}

		void operator()(const ast::program& ast)
		{
			sem::extract_type_info(ast, false, classes);
			sem::perform_full_name_type_analysis(
				ast, classes, globals, type_annotations, locals_annotations,
				vardecl_annotations, method_annotations
			);
		}

	};  // struct full_analysis

}  // namespace /* anonymous */


static const std::size_t some_ranks[] = {0, 1, 2, 3, 100};


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


BOOST_AUTO_TEST_CASE(type_stream_insertion)
{
	using namespace std::string_literals;
	using bti_type = minijava::sem::basic_type_info;
	const auto stream = [](auto&& x){
		return boost::lexical_cast<std::string>(x);
	};
	BOOST_CHECK_EQUAL("int"s, stream(sem::type{bti_type::make_int_type(), 0}));
	BOOST_CHECK_EQUAL("int[]"s, stream(sem::type{bti_type::make_int_type(), 1}));
	BOOST_CHECK_EQUAL("int[][]"s, stream(sem::type{bti_type::make_int_type(), 2}));
}


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
					tf.make_empty_main(),
					tf.make_empty_main()
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
			tf.as_class("A", tf.make_empty_main()),
			tf.as_class("B", tf.make_empty_main())
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


BOOST_AUTO_TEST_CASE(shallow_rejects_duplicate_methods_of_same_type)
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
	auto classes = sem::class_definitions{};
	auto type_annotations = sem::type_attributes{};
	sem::extract_type_info(*ast, false, classes);
	BOOST_REQUIRE_THROW(
		sem::perform_shallow_type_analysis(*ast, classes, type_annotations),
		minijava::semantic_error
	);
}


BOOST_AUTO_TEST_CASE(shallow_rejects_duplicate_methods_of_different_type)
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
	auto classes = sem::class_definitions{};
	auto type_annotations = sem::type_attributes{};
	sem::extract_type_info(*ast, false, classes);
	BOOST_REQUIRE_THROW(
		sem::perform_shallow_type_analysis(*ast, classes, type_annotations),
		minijava::semantic_error
	);
}


BOOST_AUTO_TEST_CASE(shallow_accepts_methods_with_same_name_in_different_classes)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			tf.as_class("A", tf.make_empty_method("foo")),
			tf.as_class("B", tf.make_empty_method("foo")),
			tf.as_class("C", tf.make_empty_main())
		)
	);
	auto classes = sem::class_definitions{};
	auto type_annotations = sem::type_attributes{};
	sem::extract_type_info(*ast, false, classes);
	sem::perform_shallow_type_analysis(*ast, classes, type_annotations);
}


// The MiniJava specification is unclear whether an instance method named
// 'main' should be allowed or not.  Our compiler allows this and doing so is
// the Right Thing to do.
BOOST_AUTO_TEST_CASE(shallow_accepts_instance_method_with_name_main)
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
	auto classes = sem::class_definitions{};
	auto type_annotations = sem::type_attributes{};
	sem::extract_type_info(*ast, false, classes);
	sem::perform_shallow_type_analysis(*ast, classes, type_annotations);
}


BOOST_AUTO_TEST_CASE(shallow_rejects_duplicate_fields_of_same_type)
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
					tf.make_declaration("foo", "Foo")
				),
				testaux::make_unique_ptr_vector<ast::instance_method>(),
				testaux::make_unique_ptr_vector<ast::main_method>(
					tf.make_empty_main()
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
	auto classes = sem::class_definitions{};
	auto type_annotations = sem::type_attributes{};
	sem::extract_type_info(*ast, false, classes);
	BOOST_REQUIRE_THROW(
		sem::perform_shallow_type_analysis(*ast, classes, type_annotations),
		minijava::semantic_error
	);
}


BOOST_DATA_TEST_CASE(shallow_rejects_fields_of_type_void, some_ranks)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
				tf.as_class("A", tf.make_declaration("x", ast::primitive_type::type_void, sample)),
			tf.as_class("B", tf.make_empty_main())
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


BOOST_DATA_TEST_CASE(shallow_rejects_parameters_of_type_void, some_ranks)
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
	auto classes = sem::class_definitions{};
	auto type_annotations = sem::type_attributes{};
	sem::extract_type_info(*ast, false, classes);
	BOOST_REQUIRE_THROW(
		sem::perform_shallow_type_analysis(*ast, classes, type_annotations),
		minijava::semantic_error
	);
}


BOOST_AUTO_TEST_CASE(shallow_rejects_method_of_type_void_array)
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
	auto classes = sem::class_definitions{};
	auto type_annotations = sem::type_attributes{};
	sem::extract_type_info(*ast, false, classes);
	BOOST_REQUIRE_THROW(
		sem::perform_shallow_type_analysis(*ast, classes, type_annotations),
		minijava::semantic_error
	);
}


BOOST_AUTO_TEST_CASE(shallow_extracts_field_types)
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
	auto classes = sem::class_definitions{};
	auto type_annotations = sem::type_attributes{};
	sem::extract_type_info(*ast, false, classes);
	sem::perform_shallow_type_analysis(*ast, classes, type_annotations);
	const auto expected_bti = sem::basic_type_info::make_boolean_type();
	const auto expected = sem::type{expected_bti, 7};
	const auto nodeptr = ast->classes().front()->fields().front().get();
	const auto actual = type_annotations.at(*nodeptr);
	BOOST_REQUIRE_EQUAL(expected, actual);
}


BOOST_AUTO_TEST_CASE(shallow_extracts_method_and_parameter_types)
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
	auto classes = sem::class_definitions{};
	auto type_annotations = sem::type_attributes{};
	sem::extract_type_info(*ast, false, classes);
	sem::perform_shallow_type_analysis(*ast, classes, type_annotations);
	{
		const auto expected_bti = sem::basic_type_info{*ast->classes().front(), false};
		const auto expected = sem::type{expected_bti, 1};
		const auto nodeptr = ast->classes().front()->instance_methods().front().get();
		const auto actual = type_annotations.at(*nodeptr);
		BOOST_REQUIRE_EQUAL(expected, actual);
	}
	{
		const auto expected_bti = sem::basic_type_info::make_int_type();
		const auto expected = sem::type{expected_bti, 0};
		const auto nodeptr = ast->classes().front()->instance_methods().front()
			->parameters().front().get();
		const auto actual = type_annotations.at(*nodeptr);
		BOOST_REQUIRE_EQUAL(expected, actual);
	}
}


BOOST_AUTO_TEST_CASE(shallow_extracts_main_types)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_hello_world();
	auto classes = sem::class_definitions{};
	auto type_annotations = sem::type_attributes{};
	sem::extract_type_info(*ast, false, classes);
	sem::perform_shallow_type_analysis(*ast, classes, type_annotations);
	{
		const auto expected_bti = sem::basic_type_info::make_void_type();
		const auto expected = sem::type{expected_bti, 0};
		const auto nodeptr = ast->classes().front()->main_methods().front().get();
		const auto actual = type_annotations.at(*nodeptr);
		BOOST_REQUIRE_EQUAL(expected, actual);
	}
	{
		// TODO: Here we would like to test that the parameter types are also
		//       set correctly.  Alas, due to a collusion of all involved
		//       players, they don't even exist in the first place...
	}
}


BOOST_DATA_TEST_CASE(full_rejects_local_variables_of_type_void, some_ranks)
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
	auto analysis = full_analysis{};
	BOOST_REQUIRE_THROW(analysis(*ast), minijava::semantic_error);
}


BOOST_AUTO_TEST_CASE(full_extracts_types_1st)
{
	auto tf = testaux::ast_test_factory{};
	const ast::integer_constant* nodeptr = nullptr;
	const auto ast = tf.as_program(tf.x(nodeptr, tf.make_literal("0")));
	const auto analysis = full_analysis{*ast};
	{
		const auto expected_bti = sem::basic_type_info::make_int_type();
		const auto expected = sem::type{expected_bti, 0};
		const auto actual = analysis.type_annotations.at(*nodeptr);
		BOOST_REQUIRE_EQUAL(expected, actual);
	}
}


BOOST_AUTO_TEST_CASE(full_extracts_types_2nd)
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
				tf.x(lit_0, tf.make_literal("0"))
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
						tf.x(lit_1, tf.make_literal("1"))
					)
				)
			)
		)
	);
	const auto analysis = full_analysis{*ast};
	const auto integer = sem::type{sem::basic_type_info::make_int_type(), 0};
	const auto boolean = sem::type{sem::basic_type_info::make_boolean_type(), 0};
	BOOST_REQUIRE_EQUAL(integer, analysis.type_annotations.at(*lit_0));
	BOOST_REQUIRE_EQUAL(integer, analysis.type_annotations.at(*lit_1));
	BOOST_REQUIRE_EQUAL(integer, analysis.type_annotations.at(*decl_zero));
	BOOST_REQUIRE_EQUAL(boolean, analysis.type_annotations.at(*rel_expr));
	BOOST_REQUIRE_EQUAL(boolean, analysis.type_annotations.at(*decl_broken));
}


BOOST_AUTO_TEST_CASE(full_extracts_types_3rd)
{
	auto tf = testaux::ast_test_factory{};
	const minijava::ast::variable_access* nodeptr = nullptr;
	const auto ast = tf.factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			tf.factory.make<ast::class_declaration>()(
				tf.pool.normalize("Test"),
				testaux::make_unique_ptr_vector<ast::var_decl>(
					tf.make_declaration("test", "Test")
				),
				testaux::make_unique_ptr_vector<ast::instance_method>(
					tf.factory.make<ast::instance_method>()(
						tf.pool.normalize("getTest"),
						tf.factory.make<ast::type>()(tf.pool.normalize("Test")),
						testaux::make_unique_ptr_vector<ast::var_decl>(),
						tf.factory.make<ast::block>()(
								testaux::make_unique_ptr_vector<ast::block_statement>(
								tf.factory.make<ast::return_statement>()(
									tf.x(nodeptr, tf.make_idref("test"))
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
	const auto analysis = full_analysis{*ast};
	//const auto expected = sem::basic_type_info::make_int_type();
	//  const auto expected = sem::type{expected_bti, 0};
	//  const auto actual = analysis.type_annotations.at(*nodeptr);
	//  BOOST_REQUIRE_EQUAL(expected, actual);
	// }
}
