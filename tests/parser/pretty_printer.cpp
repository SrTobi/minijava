#include "parser/pretty_printer.hpp"
#include "symbol_pool.hpp"

#include <memory>
#include <sstream>
#include <string>

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
	auto str = oss.str();
	BOOST_REQUIRE_EQUAL("class test_class {\n}\n"s, str);
}