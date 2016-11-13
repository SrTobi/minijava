#include "parser/pretty_printer.hpp"
#include "symbol_pool.hpp"

#include <memory>
#include <sstream>
#include <string>
#include <iostream>

#define BOOST_TEST_MODULE  parser_pretty_printer
#include <boost/test/unit_test.hpp>

namespace ast = minijava::ast;

BOOST_AUTO_TEST_CASE(pretty_print_empty_program)
{
	using namespace std::string_literals;
	std::ostringstream oss {};
	auto test_ast = std::make_unique<ast::program>();
	auto pp = ast::pretty_printer{oss};
	pp.visit(*test_ast);
	BOOST_REQUIRE_EQUAL(""s, oss.str());
}

BOOST_AUTO_TEST_CASE(pretty_print_single_class)
{
	using namespace std::string_literals;
	std::ostringstream oss {};
	auto pool = minijava::symbol_pool<>{};
	auto test_ast = std::make_unique<ast::program>();
	auto test_class = std::make_unique<ast::class_declaration>(pool.normalize("test_class"));
	auto pp = ast::pretty_printer{oss};

	test_ast->add_class(std::move(test_class));

	pp.visit(*test_ast);
	BOOST_REQUIRE_EQUAL("class test_class {\n}\n"s, oss.str());
}

BOOST_AUTO_TEST_CASE(pretty_print_method)
{
	using namespace std::string_literals;
	std::ostringstream oss {};
	auto pool = minijava::symbol_pool<>{};

	auto test_ast = std::make_unique<ast::program>();
	auto hello_world_class = std::make_unique<ast::class_declaration>(pool.normalize("HelloWorld"));
	auto bar_body = std::make_unique<ast::block>();
	bar_body->add_block_statement(std::make_unique<ast::expression_statement>(
		std::make_unique<ast::integer_constant>(pool.normalize("123"))
	));
	auto bar_method = std::make_unique<ast::method>(
			pool.normalize("foo"),
			std::make_unique<ast::type>(ast::primitive_type::type_int, 0),
			std::vector<std::unique_ptr<ast::var_decl>>(),
			std::move(bar_body));


	auto pp = ast::pretty_printer{oss};

	hello_world_class->add_method(std::move(bar_method));

	test_ast->add_class(std::move(hello_world_class));

	pp.visit(*test_ast);
	BOOST_REQUIRE_EQUAL("class HelloWorld {\n\tpublic int foo() {\n\t\t123;\n\t}\n}\n"s, oss.str());
}

BOOST_AUTO_TEST_CASE(pretty_print_integer)
{
	using namespace std::string_literals;
	std::ostringstream oss {};
	auto pool = minijava::symbol_pool<>{};
	auto pp = ast::pretty_printer{oss};

	auto integer_literal = std::make_unique<ast::integer_constant>(pool.normalize("123"));

	pp.visit(*integer_literal);
	BOOST_REQUIRE_EQUAL("123"s, oss.str());
}

BOOST_AUTO_TEST_CASE(pretty_print_false)
{
	using namespace std::string_literals;
	std::ostringstream oss {};
	auto pp = ast::pretty_printer{oss};

	auto bool_literal = std::make_unique<ast::boolean_constant>(false);
	pp.visit(*bool_literal);
	BOOST_REQUIRE_EQUAL("false"s, oss.str());
}

BOOST_AUTO_TEST_CASE(pretty_print_true)
{
	using namespace std::string_literals;
	std::ostringstream oss {};
	auto pp = ast::pretty_printer{oss};

	auto bool_literal = std::make_unique<ast::boolean_constant>(true);
	pp.visit(*bool_literal);
	BOOST_REQUIRE_EQUAL("true"s, oss.str());
}

BOOST_AUTO_TEST_CASE(pretty_print_null)
{
	using namespace std::string_literals;
	std::ostringstream oss {};
	auto pp = ast::pretty_printer{oss};

	auto null_constant = std::make_unique<ast::null_constant>();
	pp.visit(*null_constant);
	BOOST_REQUIRE_EQUAL("null"s, oss.str());
}