#include "parser/ast_misc.hpp"

#include <sstream>
#include <string>

#define BOOST_TEST_MODULE  parser_ast_misc
#include <boost/test/unit_test.hpp>

#include "testaux/ast_test_factory.hpp"

using namespace std::string_literals;


BOOST_AUTO_TEST_CASE(serlialization_gives_program_text)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_hello_world();
	const auto actual = to_string(*ast);
	const auto expected = ""s
		+ "class MiniJava {\n"
		+ "\tpublic static void main(String[] args) { }\n"
		+ "}\n";
	BOOST_CHECK_EQUAL(expected, actual);
}


BOOST_AUTO_TEST_CASE(serlialization_and_stream_insertion_equivalent)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_hello_world();
	std::ostringstream oss{};
	oss << *ast;
	const auto streamed = oss.str();
	const auto serialized = to_string(*ast);
	BOOST_CHECK_EQUAL(streamed, serialized);
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
	using builder_type = minijava::ast_builder<minijava::ast::empty_statement>;
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
	const auto ast1st = tf.factory.make<minijava::ast::empty_statement>()();
	const auto ast2nd = tf.make_this();
	BOOST_CHECK_NE(*ast1st, *ast2nd);
}
