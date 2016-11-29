#include "parser/ast_misc.hpp"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <iterator>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#define BOOST_TEST_MODULE  parser_ast_misc
#include <boost/test/unit_test.hpp>

#include "testaux/ast_test_factory.hpp"
#include "testaux/random_tokens.hpp"
#include "testaux/unique_ptr_vector.hpp"

namespace ast = minijava::ast;
using namespace std::string_literals;


namespace /* anonymous */
{

	// Given a multi-line character string, breaks it into individual lines
	// (removing any trailing newline characters) and discards all lines that
	// consist solely of white-space.  Next, determines the number of leading
	// tabs in the first line.  Finally, removes that many characters from the
	// begin of each line and concatenates them back together, appending a
	// single new-line character to each line.  The behavior is undefined if
	// any subsequent line is indented with less tab characters than the first
	// line.  Lines with only white space don't count.
	std::string dedent(const std::string& input)
	{
		const auto isspace = [](const auto c){ return std::isspace(c); };
		auto iss = std::istringstream{input};
		auto result = std::string{};
		auto line = std::string{};
		auto ntabs = std::string::npos;
		while (std::getline(iss, line)) {
			if (std::all_of(std::begin(line), std::end(line), isspace)) {
				continue;
			}
			if (ntabs == std::string::npos) {
				ntabs = line.find_first_not_of('\t');
				assert(ntabs != std::string::npos);
			}
			assert(line.length() > ntabs);
			result.append(line, ntabs, std::string::npos);
			result.push_back('\n');
		}
		return result;
	}

}  // namespace /* anonymous */


BOOST_AUTO_TEST_CASE(serlialization_gives_program_text)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_hello_world();
	const auto actual = to_text(*ast);
	const auto expected = dedent(R"java(
		class MiniJava {
			public static void main(String[] args) { }
		}
	)java");
	BOOST_CHECK_EQUAL(expected, actual);
}


BOOST_AUTO_TEST_CASE(serlialization_and_stream_insertion_equivalent)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_hello_world();
	std::ostringstream oss{};
	oss << *ast;
	const auto streamed = oss.str();
	const auto serialized = to_text(*ast);
	BOOST_CHECK_EQUAL(streamed, serialized);
}


BOOST_AUTO_TEST_CASE(to_xml_looks_reasonable)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_hello_world();
	const auto expected = dedent(R"xml(
		<program>
			<classes>
				<class-declaration name='MiniJava'>
					<fields>
					</fields>
					<instance-methods>
					</instance-methods>
					<main-methods>
						<main-method name='main' argname='args'>
							<body>
								<block>
									<body>
									</body>
								</block>
							</body>
						</main-method>
					</main-methods>
				</class-declaration>
			</classes>
		</program>
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(simple_ast_equal_to_self)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_hello_world();
	BOOST_CHECK(*ast == *ast);
	BOOST_CHECK(!(*ast != *ast));
}


BOOST_AUTO_TEST_CASE(node_ids_not_considered)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast1st = tf.make_hello_world();
	const auto ast2nd = tf.make_hello_world();
	BOOST_REQUIRE_NE(ast1st->id(), ast2nd->id());
	BOOST_CHECK(*ast1st == *ast2nd);
	BOOST_CHECK(!(*ast1st != *ast2nd));
}


BOOST_AUTO_TEST_CASE(sloc_not_considered)
{
	using builder_type = minijava::ast_builder<ast::empty_statement>;
	const auto ast1st = builder_type{0}.at({1, 2})();
	const auto ast2nd = builder_type{0}.at({3, 4})();
	BOOST_CHECK(*ast1st == *ast1st);
	BOOST_CHECK(!(*ast2nd != *ast2nd));
}


BOOST_AUTO_TEST_CASE(different_pools_okay)
{
	auto tf1st = testaux::ast_test_factory{};
	auto tf2nd = testaux::ast_test_factory{};
	const auto ast1st = tf1st.make_hello_world();
	const auto ast2nd = tf2nd.make_hello_world();
	BOOST_CHECK(*ast1st == *ast2nd);
	BOOST_CHECK(!(*ast1st != *ast2nd));
}


// From now on, we use BOOST_CHECK_EQUAL and assume that operator == and !=
// correlate to each other in the obvious way.


BOOST_AUTO_TEST_CASE(different_identifiers_not_equal)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast1st = tf.make_idref("apple");
	const auto ast2nd = tf.make_idref("orange");
	BOOST_CHECK_NE(*ast1st, *ast2nd);
}


BOOST_AUTO_TEST_CASE(different_types_not_equal)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast1st = tf.factory.make<ast::empty_statement>()();
	const auto ast2nd = tf.make_this();
	BOOST_CHECK_NE(*ast1st, *ast2nd);
}


BOOST_AUTO_TEST_CASE(sensitive_to_unary_minus)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast1st = tf.make_integer("1", true);
	const auto ast2nd = tf.minus(tf.make_integer("1"));
	BOOST_CHECK_NE(*ast1st, *ast2nd);
}


// For the following tests that make sure that the ordering of classes and
// class members is not important, we create a large number of ASTs (each using
// a differnt symbol pool) with many random names and require that they all be
// equal.  This is not a super reliable test but the best I could come up with.

BOOST_AUTO_TEST_CASE(independent_of_name_ordering_program)
{
	auto engine = std::default_random_engine{};
	auto names = std::vector<std::string>{};
	auto factories = std::vector<testaux::ast_test_factory>(100);
	auto programs = std::vector<std::unique_ptr<ast::program>>{};
	std::generate_n(
		std::back_inserter(names), 1000,
		[&engine](){ return testaux::get_random_identifier(engine); }
	);
	for (auto& tf : factories) {
		auto classes = std::vector<std::unique_ptr<ast::class_declaration>>{};
		std::transform(
			std::begin(names), std::end(names),
			std::back_inserter(classes),
			[&tf](auto&& name){
				return tf.as_class("PopularClassName", tf.make_empty_method(name));
			}
		);
		std::shuffle(std::begin(classes), std::end(classes), engine);
		programs.push_back(tf.factory.make<ast::program>()(std::move(classes)));
	}
	const auto count = std::count_if(
		std::begin(programs), std::end(programs),
		[head = programs.front().get()](auto&& ast){ return *head != *ast; }
	);
	BOOST_REQUIRE_EQUAL(0, count);
}


BOOST_AUTO_TEST_CASE(independent_of_name_ordering_class)
{
	auto global_engine = std::default_random_engine{};
	auto factories = std::vector<testaux::ast_test_factory>(100);
	auto classes = std::vector<std::unique_ptr<ast::class_declaration>>{};
	for (auto& tf : factories) {
		auto fields = std::vector<std::unique_ptr<ast::var_decl>>{};
		auto instance_methods = std::vector<std::unique_ptr<ast::instance_method>>{};
		auto main_methods = std::vector<std::unique_ptr<ast::main_method>>{};
		std::generate_n(
			std::back_inserter(fields), 100,
			[&tf](){
				auto rankdist = std::uniform_int_distribution<std::size_t>{0, 10};
				const auto rank = rankdist(tf.engine);
				const auto type_name = testaux::get_random_identifier(tf.engine);
				return tf.make_declaration("popularFieldName", type_name, rank);
			}
		);
		std::generate_n(
			std::back_inserter(instance_methods), 100,
			[&tf](){
				auto rankdist = std::uniform_int_distribution<std::size_t>{0, 10};
				const auto rank = rankdist(tf.engine);
				const auto type_name = testaux::get_random_identifier(tf.engine);
				return tf.factory.make<ast::instance_method>()(
					tf.pool.normalize("popularInstanceMethodName"),
					tf.factory.make<ast::type>()(
						tf.pool.normalize(type_name), rank
					),
					testaux::make_unique_ptr_vector<ast::var_decl>(),
					tf.make_empty_block()
				);
			}
		);
		std::generate_n(
			std::back_inserter(main_methods), 100,
			[&tf](){
				return tf.factory.make<ast::main_method>()(
					tf.pool.normalize("popularMainMethodName"),
					tf.pool.normalize(testaux::get_random_identifier(tf.engine)),
					tf.make_empty_block()
				);
			}
		);
		std::shuffle(std::begin(fields), std::end(fields), global_engine);
		std::shuffle(std::begin(instance_methods), std::end(instance_methods), global_engine);
		std::shuffle(std::begin(main_methods), std::end(main_methods), global_engine);
		classes.push_back(
			tf.factory.make<ast::class_declaration>()(
				tf.pool.normalize(testaux::get_random_identifier(tf.engine)),
				std::move(fields),
				std::move(instance_methods),
				std::move(main_methods)
			)
		);
	}
	const auto count = std::count_if(
		std::begin(classes), std::end(classes),
		[head = classes.front().get()](auto&& ast){ return *head != *ast; }
	);
	BOOST_REQUIRE_EQUAL(0, count);
}


BOOST_AUTO_TEST_CASE(xml_type_1st)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::type>()(ast::primitive_type::type_int, 1);
	const auto expected = dedent(R"xml(
		<type name='int' primitive='true' rank='1' />
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_type_2st)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::type>()(ast::primitive_type::type_boolean, 2);
	const auto expected = dedent(R"xml(
		<type name='boolean' primitive='true' rank='2' />
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_type_3rd)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::type>()(ast::primitive_type::type_void, 3);
	const auto expected = dedent(R"xml(
		<type name='void' primitive='true' rank='3' />
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_type_4th)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::type>()(tf.pool.normalize("Foo"));
	const auto expected = dedent(R"xml(
		<type name='Foo' primitive='false' rank='0' />
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_var_decl_1st)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_declaration("foo", ast::primitive_type::type_int);
	const auto expected = dedent(R"xml(
		<var-decl name='foo'>
			<type name='int' primitive='true' rank='0' />
		</var-decl>
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_var_decl_2nd)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_declaration("foo", "Foo", 10);
	const auto expected = dedent(R"xml(
		<var-decl name='foo'>
			<type name='Foo' primitive='false' rank='10' />
		</var-decl>
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


static const auto xml_binary_expression_body = ""s
	+ "\t<lhs>\n"
	+ "\t\t<variable-access name='x'>\n"
	+ "\t\t\t<target />\n"
	+ "\t\t</variable-access>\n"
	+ "\t</lhs>\n"
	+ "\t<rhs>\n"
	+ "\t\t<variable-access name='y'>\n"
	+ "\t\t\t<target />\n"
	+ "\t\t</variable-access>\n"
	+ "\t</rhs>\n";

BOOST_AUTO_TEST_CASE(xml_binary_expression_assign)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.assign(tf.make_idref("x"), tf.make_idref("y"));
	const auto expected = ""s
		+ "<binary-expression operation='ASSIGN'>\n"
		+ xml_binary_expression_body
		+ "</binary-expression>\n";
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_binary_expression_logical_or)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.logical_or(tf.make_idref("x"), tf.make_idref("y"));
	const auto expected = ""s
		+ "<binary-expression operation='LOGICAL_OR'>\n"
		+ xml_binary_expression_body
		+ "</binary-expression>\n";
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_binary_expression_logical_and)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.logical_and(tf.make_idref("x"), tf.make_idref("y"));
	const auto expected = ""s
		+ "<binary-expression operation='LOGICAL_AND'>\n"
		+ xml_binary_expression_body
		+ "</binary-expression>\n";
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_binary_expression_equal)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.equal(tf.make_idref("x"), tf.make_idref("y"));
	const auto expected = ""s
		+ "<binary-expression operation='EQUAL'>\n"
		+ xml_binary_expression_body
		+ "</binary-expression>\n";
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_binary_expression_not_equal)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.not_equal(tf.make_idref("x"), tf.make_idref("y"));
	const auto expected = ""s
		+ "<binary-expression operation='NOT_EQUAL'>\n"
		+ xml_binary_expression_body
		+ "</binary-expression>\n";
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_binary_expression_less_than)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.less_than(tf.make_idref("x"), tf.make_idref("y"));
	const auto expected = ""s
		+ "<binary-expression operation='LESS_THAN'>\n"
		+ xml_binary_expression_body
		+ "</binary-expression>\n";
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_binary_expression_less_equal)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.less_equal(tf.make_idref("x"), tf.make_idref("y"));
	const auto expected = ""s
		+ "<binary-expression operation='LESS_EQUAL'>\n"
		+ xml_binary_expression_body
		+ "</binary-expression>\n";
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_binary_expression_greater_than)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.greater_than(tf.make_idref("x"), tf.make_idref("y"));
	const auto expected = ""s
		+ "<binary-expression operation='GREATER_THAN'>\n"
		+ xml_binary_expression_body
		+ "</binary-expression>\n";
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_binary_expression_greater_equal)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.greater_equal(tf.make_idref("x"), tf.make_idref("y"));
	const auto expected = ""s
		+ "<binary-expression operation='GREATER_EQUAL'>\n"
		+ xml_binary_expression_body
		+ "</binary-expression>\n";
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_binary_expression_plus)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.plus(tf.make_idref("x"), tf.make_idref("y"));
	const auto expected = ""s
		+ "<binary-expression operation='PLUS'>\n"
		+ xml_binary_expression_body
		+ "</binary-expression>\n";
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_binary_expression_minus)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.minus(tf.make_idref("x"), tf.make_idref("y"));
	const auto expected = ""s
		+ "<binary-expression operation='MINUS'>\n"
		+ xml_binary_expression_body
		+ "</binary-expression>\n";
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_binary_expression_multiply)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.multiply(tf.make_idref("x"), tf.make_idref("y"));
	const auto expected = ""s
		+ "<binary-expression operation='MULTIPLY'>\n"
		+ xml_binary_expression_body
		+ "</binary-expression>\n";
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_binary_expression_divide)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.divide(tf.make_idref("x"), tf.make_idref("y"));
	const auto expected = ""s
		+ "<binary-expression operation='DIVIDE'>\n"
		+ xml_binary_expression_body
		+ "</binary-expression>\n";
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_binary_expression_modulo)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.modulo(tf.make_idref("x"), tf.make_idref("y"));
	const auto expected = ""s
		+ "<binary-expression operation='MODULO'>\n"
		+ xml_binary_expression_body
		+ "</binary-expression>\n";
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


static const auto xml_unary_expression_body = ""s
	+ "\t<target>\n"
	+ "\t\t<variable-access name='x'>\n"
	+ "\t\t\t<target />\n"
	+ "\t\t</variable-access>\n"
	+ "\t</target>\n";

BOOST_AUTO_TEST_CASE(xml_unary_expression_logical_not)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.logical_not(tf.make_idref("x"));
	const auto expected = ""s
		+ "<unary-expression operation='LOGICAL_NOT'>\n"
		+ xml_unary_expression_body
		+ "</unary-expression>\n";
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_unary_expression_minus)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.minus(tf.make_idref("x"));
	const auto expected = ""s
		+ "<unary-expression operation='MINUS'>\n"
		+ xml_unary_expression_body
		+ "</unary-expression>\n";
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_object_instantiation)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_new("Object");
	const auto expected = dedent(R"xml(
		<object-instantiation class='Object' />
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_array_instantiation)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::array_instantiation>()(
		tf.factory.make<ast::type>()(ast::primitive_type::type_int, 3),
		tf.make_integer("42")
	);
	const auto expected = dedent(R"xml(
		<array-instantiation>
			<type>
				<type name='int' primitive='true' rank='3' />
			</type>
			<extent>
				<integer-constant literal='42' negative='false' />
			</extent>
		</array-instantiation>
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_array_access)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::array_access>()
		(tf.make_idref("array"), tf.make_integer("42"));
	const auto expected = dedent(R"xml(
		<array-access>
			<target>
				<variable-access name='array'>
					<target />
				</variable-access>
			</target>
			<index>
				<integer-constant literal='42' negative='false' />
			</index>
		</array-access>
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_variable_access_1st)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_idref("foo");
	const auto expected = dedent(R"xml(
		<variable-access name='foo'>
			<target />
		</variable-access>
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_variable_access_2nd)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_idref_this("foo");
	const auto expected = dedent(R"xml(
		<variable-access name='foo'>
			<target>
				<this-ref />
			</target>
		</variable-access>
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_method_invocation_1st)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_call("foo");
	const auto expected = dedent(R"xml(
		<method-invocation name='foo'>
			<target />
			<arguments>
			</arguments>
		</method-invocation>
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_method_invocation_2nd)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::method_invocation>()(
		tf.make_this(),
		tf.pool.normalize("foo"),
		testaux::make_unique_ptr_vector<ast::expression>(
			// Deliberately in this order to test that they are not re-ordered.
			tf.make_integer("3"),
			tf.make_integer("1"),
			tf.make_integer("2")
		)
	);
	const auto expected = dedent(R"xml(
		<method-invocation name='foo'>
			<target>
				<this-ref />
			</target>
			<arguments>
				<integer-constant literal='3' negative='false' />
				<integer-constant literal='1' negative='false' />
				<integer-constant literal='2' negative='false' />
			</arguments>
		</method-invocation>
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_this_ref)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_this();
	const auto expected = dedent(R"xml(
		<this-ref />
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_boolean_constant_1st)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_boolean(false);
	const auto expected = dedent(R"xml(
		<boolean-constant value='false' />
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_boolean_constant_2nd)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_boolean(true);
	const auto expected = dedent(R"xml(
		<boolean-constant value='true' />
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_integer_constant_1st)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_integer("123", false);
	const auto expected = dedent(R"xml(
		<integer-constant literal='123' negative='false' />
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_integer_constant_2nd)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_integer("456789", true);
	const auto expected = dedent(R"xml(
		<integer-constant literal='456789' negative='true' />
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_null_constant)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_null();
	const auto expected = dedent(R"xml(
		<null-constant />
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_local_variable_statement_1st)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::local_variable_statement>()(
		tf.make_declaration("bar", ast::primitive_type::type_int, 1),
		nullptr
	);
	const auto expected = dedent(R"xml(
		<local-variable-statement>
			<declaration>
				<var-decl name='bar'>
					<type name='int' primitive='true' rank='1' />
				</var-decl>
			</declaration>
			<initial-value />
		</local-variable-statement>
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_local_variable_statement_2nd)
{
	auto tf = testaux::ast_test_factory{};
	// There is of course no inheritance in MiniJava but the idea of this test
	// is to make sure that the types are not confused in the AST.
	const auto ast = tf.factory.make<ast::local_variable_statement>()(
		tf.make_declaration("thing", "Base"),
		tf.make_new("Derived")
	);
	const auto expected = dedent(R"xml(
		<local-variable-statement>
			<declaration>
				<var-decl name='thing'>
					<type name='Base' primitive='false' rank='0' />
				</var-decl>
			</declaration>
			<initial-value>
				<object-instantiation class='Derived' />
			</initial-value>
		</local-variable-statement>
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_expression_statement)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_assignment(
		tf.make_idref_this("thing"),
		tf.make_null()
	);
	const auto expected = dedent(R"xml(
		<expression-statement>
			<inner-expression>
				<binary-expression operation='ASSIGN'>
					<lhs>
						<variable-access name='thing'>
							<target>
								<this-ref />
							</target>
						</variable-access>
					</lhs>
					<rhs>
						<null-constant />
					</rhs>
				</binary-expression>
			</inner-expression>
		</expression-statement>
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_block_1st)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_empty_block();
	const auto expected = dedent(R"xml(
		<block>
			<body>
			</body>
		</block>
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_block_2nd)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<minijava::ast::block>()(
		testaux::make_unique_ptr_vector<minijava::ast::block_statement>(
			tf.make_empty_block(),
			tf.factory.make<ast::expression_statement>()(
				tf.make_this()
			),
			tf.make_empty_block()
		)
	);
	const auto expected = dedent(R"xml(
		<block>
			<body>
				<block>
					<body>
					</body>
				</block>
				<expression-statement>
					<inner-expression>
						<this-ref />
					</inner-expression>
				</expression-statement>
				<block>
					<body>
					</body>
				</block>
			</body>
		</block>
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_if_statement_1st)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::if_statement>()(
		tf.make_boolean(true),
		tf.factory.make<ast::expression_statement>()(tf.make_call("f")),
		nullptr
	);
	const auto expected = dedent(R"xml(
		<if-statement>
			<condition>
				<boolean-constant value='true' />
			</condition>
			<then>
				<expression-statement>
					<inner-expression>
						<method-invocation name='f'>
							<target />
							<arguments>
							</arguments>
						</method-invocation>
					</inner-expression>
				</expression-statement>
			</then>
			<else />
		</if-statement>
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_if_statement_2nd)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::if_statement>()(
		tf.make_call("f"),
		tf.make_empty_stmt(),
		tf.make_empty_block()
	);
	const auto expected = dedent(R"xml(
		<if-statement>
			<condition>
				<method-invocation name='f'>
					<target />
					<arguments>
					</arguments>
				</method-invocation>
			</condition>
			<then>
				<empty-statement />
			</then>
			<else>
				<block>
					<body>
					</body>
				</block>
			</else>
		</if-statement>
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_while_statement)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::while_statement>()(
		tf.make_boolean(true),
		tf.make_empty_stmt()
	);
	const auto expected = dedent(R"xml(
		<while-statement>
			<condition>
				<boolean-constant value='true' />
			</condition>
			<body>
				<empty-statement />
			</body>
		</while-statement>
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_return_statement_1st)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_return();
	const auto expected = dedent(R"xml(
		<return-statement>
			<value />
		</return-statement>
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_return_statement_2nd)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_return(tf.make_null());
	const auto expected = dedent(R"xml(
		<return-statement>
			<value>
				<null-constant />
			</value>
		</return-statement>
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_empty_statement)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_empty_stmt();
	const auto expected = dedent(R"xml(
		<empty-statement />
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_main_method_1st)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_empty_main();
	const auto expected = dedent(R"xml(
		<main-method name='main' argname='args'>
			<body>
				<block>
					<body>
					</body>
				</block>
			</body>
		</main-method>
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_main_method_2nd)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::main_method>()(
		tf.pool.normalize("lame"),
		tf.pool.normalize("xxx"),
		tf.as_block(tf.make_empty_block())
	);
	const auto expected = dedent(R"xml(
		<main-method name='lame' argname='xxx'>
			<body>
				<block>
					<body>
						<block>
							<body>
							</body>
						</block>
					</body>
				</block>
			</body>
		</main-method>
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_instance_method_1st)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_empty_method("func");
	const auto expected = dedent(R"xml(
		<instance-method name='func'>
			<return-type>
				<type name='void' primitive='true' rank='0' />
			</return-type>
			<parameters>
			</parameters>
			<body>
				<block>
					<body>
					</body>
				</block>
			</body>
		</instance-method>
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_instance_method_2nd)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::instance_method>()(
		tf.pool.normalize("makeArray"),
		tf.factory.make<ast::type>()(ast::primitive_type::type_int, 1),
		testaux::make_unique_ptr_vector<ast::var_decl>(
			tf.make_declaration("n", ast::primitive_type::type_int)
		),
		tf.as_block(
			tf.make_return(
				tf.factory.make<ast::array_instantiation>()(
					tf.factory.make<ast::type>()(ast::primitive_type::type_int, 1),
					tf.make_idref("n")
				)
			)
		)
	);
	const auto expected = dedent(R"xml(
		<instance-method name='makeArray'>
			<return-type>
				<type name='int' primitive='true' rank='1' />
			</return-type>
			<parameters>
				<var-decl name='n'>
					<type name='int' primitive='true' rank='0' />
				</var-decl>
			</parameters>
			<body>
				<block>
					<body>
						<return-statement>
							<value>
								<array-instantiation>
									<type>
										<type name='int' primitive='true' rank='1' />
									</type>
									<extent>
										<variable-access name='n'>
											<target />
										</variable-access>
									</extent>
								</array-instantiation>
							</value>
						</return-statement>
					</body>
				</block>
			</body>
		</instance-method>
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_class_declaration_1st)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::class_declaration>()(
		tf.pool.normalize("Test"),
		testaux::make_unique_ptr_vector<ast::var_decl>(),
		testaux::make_unique_ptr_vector<ast::instance_method>(),
		testaux::make_unique_ptr_vector<ast::main_method>()
	);
	const auto expected = dedent(R"xml(
		<class-declaration name='Test'>
			<fields>
			</fields>
			<instance-methods>
			</instance-methods>
			<main-methods>
			</main-methods>
		</class-declaration>
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_class_declaration_2nd)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::class_declaration>()(
		tf.pool.normalize("Test"),
		testaux::make_unique_ptr_vector<ast::var_decl>(
			// Return type 'boolean' should sort before 'int'.
			tf.make_declaration("field", ast::primitive_type::type_int),
			tf.make_declaration("field", ast::primitive_type::type_boolean)
		),
		testaux::make_unique_ptr_vector<ast::instance_method>(
			// Two exactly identical methods should not be a problem.
			tf.make_empty_method("method"),
			tf.make_empty_method("method")
		),
		testaux::make_unique_ptr_vector<ast::main_method>(
			// Parameter name 'abc' should sort before 'xyz'.
			tf.make_empty_main("main", "xyz"),
			tf.make_empty_main("main", "abc")
		)
	);
	const auto expected = dedent(R"xml(
		<class-declaration name='Test'>
			<fields>
				<var-decl name='field'>
					<type name='boolean' primitive='true' rank='0' />
				</var-decl>
				<var-decl name='field'>
					<type name='int' primitive='true' rank='0' />
				</var-decl>
			</fields>
			<instance-methods>
				<instance-method name='method'>
					<return-type>
						<type name='void' primitive='true' rank='0' />
					</return-type>
					<parameters>
					</parameters>
					<body>
						<block>
							<body>
							</body>
						</block>
					</body>
				</instance-method>
				<instance-method name='method'>
					<return-type>
						<type name='void' primitive='true' rank='0' />
					</return-type>
					<parameters>
					</parameters>
					<body>
						<block>
							<body>
							</body>
						</block>
					</body>
				</instance-method>
			</instance-methods>
			<main-methods>
				<main-method name='main' argname='abc'>
					<body>
						<block>
							<body>
							</body>
						</block>
					</body>
				</main-method>
				<main-method name='main' argname='xyz'>
					<body>
						<block>
							<body>
							</body>
						</block>
					</body>
				</main-method>
			</main-methods>
		</class-declaration>
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_program_1st)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>()
	);
	const auto expected = dedent(R"xml(
		<program>
			<classes>
			</classes>
		</program>
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}


BOOST_AUTO_TEST_CASE(xml_program_2nd)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.factory.make<ast::program>()(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			tf.as_class("Beta", tf.make_declaration("field", "ABC")),
			tf.as_class("Beta", tf.make_declaration("field", "XYZ")),
			tf.make_empty_class("Alpha")
		)
	);
	const auto expected = dedent(R"xml(
		<program>
			<classes>
				<class-declaration name='Alpha'>
					<fields>
					</fields>
					<instance-methods>
					</instance-methods>
					<main-methods>
					</main-methods>
				</class-declaration>
				<class-declaration name='Beta'>
					<fields>
						<var-decl name='field'>
							<type name='ABC' primitive='false' rank='0' />
						</var-decl>
					</fields>
					<instance-methods>
					</instance-methods>
					<main-methods>
					</main-methods>
				</class-declaration>
				<class-declaration name='Beta'>
					<fields>
						<var-decl name='field'>
							<type name='XYZ' primitive='false' rank='0' />
						</var-decl>
					</fields>
					<instance-methods>
					</instance-methods>
					<main-methods>
					</main-methods>
				</class-declaration>
			</classes>
		</program>
	)xml");
	BOOST_CHECK_EQUAL(expected, to_xml(*ast));
}
