#include "parser/pretty_printer.hpp"
#include "symbol_pool.hpp"

#include <memory>
#include <sstream>
#include <string>
#include <iostream>

#define BOOST_TEST_MODULE  parser_pretty_printer
#include <boost/test/unit_test.hpp>

namespace ast = minijava::ast;

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

BOOST_AUTO_TEST_CASE(pretty_print_class_with_method)
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

BOOST_AUTO_TEST_CASE(pretty_print_simple_conditional)
{
	using namespace std::string_literals;
	std::ostringstream oss {};
	auto pp = ast::pretty_printer{oss};
	auto pool = minijava::symbol_pool<>{};

	auto test_conditional = std::make_unique<ast::binary_expression>(
			ast::binary_operation_type::equal,
			std::make_unique<ast::variable_access>(
					nullptr, pool.normalize("i")
			),
			std::make_unique<ast::variable_access>(
					nullptr, pool.normalize("j")
			)
	);
	auto test_then_statement = std::make_unique<ast::expression_statement>(
		std::make_unique<ast::assignment_expression>(
				std::make_unique<ast::variable_access>(
						nullptr, pool.normalize("i")
				),
		        std::make_unique<ast::integer_constant>(pool.normalize("0"))
		)
	);
	auto test_conditional_block = std::make_unique<ast::if_statement>(
		std::move(test_conditional),
		std::move(test_then_statement),
	    std::make_unique<ast::empty_statement>()
	);

	pp.visit(*test_conditional_block);
	BOOST_REQUIRE_EQUAL("if (i == j)\n\ti = 0;\n"s, oss.str());
}

BOOST_AUTO_TEST_CASE(pretty_print_elseif_and_empty_conditional)
{
	using namespace std::string_literals;
	std::ostringstream oss {};
	auto pp = ast::pretty_printer{oss};
	auto pool = minijava::symbol_pool<>{};

	auto test_conditional = std::make_unique<ast::boolean_constant>(true);
	auto test_then_statement = std::make_unique<ast::empty_statement>();
	auto test_else_conditional = std::make_unique<ast::boolean_constant>(false);
	auto test_else_then_statement = std::make_unique<ast::block>();
	auto test_assignment = std::make_unique<ast::expression_statement>(
			std::make_unique<ast::assignment_expression>(
					std::make_unique<ast::variable_access>(
							nullptr, pool.normalize("i")
					),
					std::make_unique<ast::integer_constant>(pool.normalize("0"))
			)
	);
	test_else_then_statement->add_block_statement(std::move(test_assignment));
	auto test_else_statement = std::make_unique<ast::block>();
	auto test_else_conditional_block = std::make_unique<ast::if_statement>(
			std::move(test_else_conditional),
			std::move(test_else_then_statement),
			std::move(test_else_statement)
	);
	auto test_conditional_block = std::make_unique<ast::if_statement>(
			std::move(test_conditional),
			std::move(test_then_statement),
			std::move(test_else_conditional_block)
	);

	pp.visit(*test_conditional_block);
	BOOST_REQUIRE_EQUAL(
			"if (true)\n\t;\nelse if (false) {\n\ti = 0;\n} else { }\n"s,
			oss.str()
	);
}

BOOST_AUTO_TEST_CASE(pretty_print_statements_and_expressions)
{
	using namespace std::string_literals;
	std::ostringstream oss {};
	auto pp = ast::pretty_printer{oss};
	auto pool = minijava::symbol_pool<>{};

	auto test_block = std::make_unique<ast::block>();
	auto test_local = std::make_unique<ast::local_variable_statement>(
			std::make_unique<ast::var_decl>(
					std::make_unique<ast::type>(
						ast::primitive_type::type_int, 0
					),
					pool.normalize("i")
			),
			std::make_unique<ast::binary_expression>(
					ast::binary_operation_type::plus,
					std::make_unique<ast::binary_expression>(
							ast::binary_operation_type::multiply,
							std::make_unique<ast::integer_constant>(
									pool.normalize("3")
							),
							std::make_unique<ast::integer_constant>(
									pool.normalize("5")
							)
					),
					std::make_unique<ast::integer_constant>(
							pool.normalize("10")
					)
			)
	);
	auto test_empty = std::make_unique<ast::empty_statement>();
	auto test_array = std::make_unique<ast::array_instantiation>(
			std::make_unique<ast::type>(
					pool.normalize("bar"),
					2
			),
	        std::make_unique<ast::integer_constant>(
			        pool.normalize("2")
	        )
	);
	auto test_argument = std::make_unique<ast::array_access>(
			std::make_unique<ast::array_access>(
					std::move(test_array),
					std::make_unique<ast::integer_constant>(
							pool.normalize("1")
					)
			),
			std::make_unique<ast::variable_access>(
					nullptr,
					pool.normalize("i")
			)
	);
	std::vector<std::unique_ptr<ast::expression>> test_arguments {};
	test_arguments.push_back(std::move(test_argument));
	auto test_statement = std::make_unique<ast::expression_statement>(
			std::make_unique<ast::method_invocation>(
					nullptr,
			        pool.normalize("f"),
					std::move(test_arguments)
			)
	);
	test_block->add_block_statement(std::move(test_local));
	test_block->add_block_statement(std::move(test_empty));
	test_block->add_block_statement(std::move(test_statement));

	pp.visit(*test_block);
	BOOST_REQUIRE_EQUAL(
			"{\n\tint i = (3 * 5) + 10;\n\tf(((new bar[2][])[1])[i]);\n}\n"s,
	        oss.str()
	);
}
