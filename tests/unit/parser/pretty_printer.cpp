#include "parser/pretty_printer.hpp"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#define BOOST_TEST_MODULE  parser_pretty_printer
#include <boost/test/unit_test.hpp>

#include "symbol/symbol_pool.hpp"

#include "testaux/ast_test_factory.hpp"
#include "testaux/unique_ptr_vector.hpp"

namespace ast = minijava::ast;
using namespace std::string_literals;


namespace /* anonymous */
{

	// We're not using `to_text` from `ast_misc.hpp` here to avoid a cyclic
	// dependency.

	std::string pretty_print(const ast::node& ast)
	{
		std::ostringstream oss{};
		minijava::pretty_printer pp{oss};
		ast.accept(pp);
		return oss.str();
	}

}


BOOST_AUTO_TEST_CASE(pretty_print_integer)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_integer("123");
	BOOST_REQUIRE_EQUAL("123"s, pretty_print(*ast));
}

BOOST_AUTO_TEST_CASE(pretty_print_false)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_boolean(false);
	BOOST_REQUIRE_EQUAL("false"s, pretty_print(*ast));
}

BOOST_AUTO_TEST_CASE(pretty_print_true)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_boolean(true);
	BOOST_REQUIRE_EQUAL("true"s, pretty_print(*ast));
}

BOOST_AUTO_TEST_CASE(pretty_print_null)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_null();
	BOOST_REQUIRE_EQUAL("null"s, pretty_print(*ast));
}

BOOST_AUTO_TEST_CASE(pretty_print_this)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_this();
	BOOST_REQUIRE_EQUAL("this"s, pretty_print(*ast));
}

BOOST_AUTO_TEST_CASE(pretty_print_empty_program)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>()
	);
	BOOST_REQUIRE_EQUAL(""s, pretty_print(*ast));
}

BOOST_AUTO_TEST_CASE(pretty_print_single_class)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_empty_class("MyClass");
	BOOST_REQUIRE_EQUAL("class MyClass { }\n"s, pretty_print(*ast));
}

BOOST_AUTO_TEST_CASE(pretty_print_class_with_method)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.as_program(
		tf.as_class(
			"HelloWorld",
			tf.factory.make<ast::instance_method>()(
				tf.pool.normalize("foo"),
				tf.factory.make<ast::type>()(ast::primitive_type::type_int),
				testaux::make_unique_ptr_vector<ast::var_decl>(),
				tf.as_block(tf.make_integer("123"))
			)
		)
	);
	BOOST_REQUIRE_EQUAL(
		"class HelloWorld {\n\tpublic int foo() {\n\t\t123;\n\t}\n}\n"s,
		pretty_print(*ast)
	);
}

BOOST_AUTO_TEST_CASE(pretty_print_simple_conditional)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::if_statement>()(
		tf.equal(tf.make_idref("i"), tf.make_idref("j")),
		tf.make_assignment(tf.make_idref("i"), tf.make_integer("0")),
		tf.factory.make<ast::empty_statement>()()
	);
	// We're ignoring the spec here to achieve idempotence see COMPRAKT-4
	BOOST_REQUIRE_EQUAL("if (i == j)\n\ti = 0;\nelse\n\t;\n"s, pretty_print(*ast));
}

BOOST_AUTO_TEST_CASE(pretty_print_elseif_and_empty_conditional)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::if_statement>()(
		tf.make_boolean(true),
		tf.factory.make<ast::empty_statement>()(),
		tf.factory.make<ast::if_statement>()(
			tf.make_boolean(false),
			tf.as_block(tf.make_assignment(tf.make_idref("i"), tf.make_integer("0"))),
			tf.make_empty_block()
		)
	);
	BOOST_REQUIRE_EQUAL(
		"if (true)\n\t;\nelse if (false) {\n\ti = 0;\n} else { }\n"s,
		pretty_print(*ast)
	);
}

BOOST_AUTO_TEST_CASE(pretty_print_nested_if)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::if_statement>()(
		tf.make_boolean(false),
		tf.factory.make<ast::if_statement>()(
			tf.make_boolean(true),
			tf.factory.make<ast::return_statement>()(nullptr),
			tf.make_empty_block()
		),
		nullptr
	);
	BOOST_REQUIRE_EQUAL(
		"if (false)\n\tif (true)\n\t\treturn;\n\telse { }\n"s,
		pretty_print(*ast)
	);
}

BOOST_AUTO_TEST_CASE(pretty_print_simple_while)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::while_statement>()(
		tf.equal(tf.make_idref("i"), tf.make_idref("j")),
		tf.make_assignment(tf.make_idref("i"), tf.make_integer("0"))
	);
	BOOST_REQUIRE_EQUAL("while (i == j)\n\ti = 0;\n"s, pretty_print(*ast));
}

BOOST_AUTO_TEST_CASE(pretty_print_regular_while)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::while_statement>()(
		tf.equal(tf.make_idref("i"), tf.make_idref("j")),
		tf.as_block(tf.make_assignment(tf.make_idref("i"), tf.make_integer("0")))
	);
	BOOST_REQUIRE_EQUAL("while (i == j) {\n\ti = 0;\n}\n"s, pretty_print(*ast));
}

BOOST_AUTO_TEST_CASE(pretty_print_empty_while)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::while_statement>()(
		tf.equal(tf.make_idref("i"), tf.make_idref("j")),
		tf.make_empty_block()
	);
	BOOST_REQUIRE_EQUAL("while (i == j) { }\n"s, pretty_print(*ast));
}

BOOST_AUTO_TEST_CASE(pretty_print_statements_and_expressions)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::block>()(
		testaux::make_unique_ptr_vector<ast::block_statement>(
			tf.factory.make<ast::local_variable_statement>()(
				tf.make_declaration("i", ast::primitive_type::type_int),
				tf.plus(
					tf.multiply(tf.make_integer("3"), tf.make_integer("5")),
					tf.make_integer("10")
				)
			),
			tf.factory.make<ast::empty_statement>()(),
			tf.factory.make<ast::expression_statement>()(
				tf.factory.make<ast::method_invocation>()(
					nullptr,
					tf.pool.normalize("f"),
					testaux::make_unique_ptr_vector<ast::expression>(
						tf.factory.make<ast::array_access>()(
							tf.factory.make<ast::array_access>()(
								tf.factory.make<ast::array_instantiation>()(
									tf.factory.make<ast::type>()(tf.pool.normalize("bar"), 2),
									tf.make_integer("2")
								),
								tf.make_integer("1")
							),
							tf.make_idref("i")
						)
					)
				)
			)
		)
	);
	BOOST_REQUIRE_EQUAL(
		"{\n\tint i = (3 * 5) + 10;\n\tf(((new bar[2][])[1])[i]);\n}\n"s,
		pretty_print(*ast)
	);
}

BOOST_AUTO_TEST_CASE(class_with_var_decl)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::class_declaration>()(
		tf.pool.normalize("Test"),
		testaux::make_unique_ptr_vector<ast::var_decl>(
			tf.make_declaration("apple", "Apple"),
			tf.make_declaration("banana", ast::primitive_type::type_int),
			tf.make_declaration("cranberry", ast::primitive_type::type_boolean),
			tf.make_declaration("date", ast::primitive_type::type_void, 0),
			tf.make_declaration("date", ast::primitive_type::type_void, 1),
			tf.make_declaration("date", ast::primitive_type::type_void, 2),
			tf.make_declaration("date", ast::primitive_type::type_void, 3)
		),
		testaux::make_unique_ptr_vector<ast::instance_method>(),
		testaux::make_unique_ptr_vector<ast::main_method>()
	);
	const auto expected = ""s
		+ "class Test {\n"
		+ "\tpublic Apple apple;\n"
		+ "\tpublic int banana;\n"
		+ "\tpublic boolean cranberry;\n"
		+ "\tpublic void date;\n"
		+ "\tpublic void[] date;\n"
		+ "\tpublic void[][] date;\n"
		+ "\tpublic void[][][] date;\n"
		+ "}\n";
	BOOST_REQUIRE_EQUAL(expected, pretty_print(*ast));
}
