#include "parser/pretty_printer.hpp"
#include "symbol/symbol_pool.hpp"

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

BOOST_AUTO_TEST_CASE(pretty_print_this)
{
	using namespace std::string_literals;
	std::ostringstream oss {};
	auto pp = ast::pretty_printer{oss};
	auto this_ref = std::make_unique<ast::this_ref>();
	pp.visit(*this_ref);
	BOOST_REQUIRE_EQUAL("this"s, oss.str());
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
	BOOST_REQUIRE_EQUAL("class test_class { }\n"s, oss.str());
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
		std::make_unique<ast::binary_expression>(
				ast::binary_operation_type::assign,
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
			std::make_unique<ast::binary_expression>(
					ast::binary_operation_type::assign,
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

BOOST_AUTO_TEST_CASE(pretty_print_nested_if)
{
	using namespace std::string_literals;
	std::ostringstream oss {};
	auto pp = ast::pretty_printer{oss};
	auto pool = minijava::symbol_pool<>{};

	auto test_conditional = std::make_unique<ast::if_statement>(
			std::make_unique<ast::boolean_constant>(false),
			std::make_unique<ast::if_statement>(
					std::make_unique<ast::boolean_constant>(true),
			        std::make_unique<ast::return_statement>(nullptr),
			        std::make_unique<ast::block>()
			),
	        nullptr
	);

	pp.visit(*test_conditional);
	BOOST_REQUIRE_EQUAL("if (false)\n\tif (true)\n\t\treturn;\n\telse { }\n"s, oss.str());
}

BOOST_AUTO_TEST_CASE(pretty_print_simple_while)
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
	auto test_loop_body = std::make_unique<ast::expression_statement>(
			std::make_unique<ast::binary_expression>(
					ast::binary_operation_type::assign,
					std::make_unique<ast::variable_access>(
							nullptr, pool.normalize("i")
					),
					std::make_unique<ast::integer_constant>(pool.normalize("0"))
			)
	);
	auto test_loop = std::make_unique<ast::while_statement>(
			std::move(test_conditional),
			std::move(test_loop_body)
	);

	pp.visit(*test_loop);
	BOOST_REQUIRE_EQUAL("while (i == j)\n\ti = 0;\n"s, oss.str());
}

BOOST_AUTO_TEST_CASE(pretty_print_regular_while)
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
	auto test_loop_body = std::make_unique<ast::expression_statement>(
			std::make_unique<ast::binary_expression>(
					ast::binary_operation_type::assign,
					std::make_unique<ast::variable_access>(
							nullptr, pool.normalize("i")
					),
					std::make_unique<ast::integer_constant>(pool.normalize("0"))
			)
	);
	auto test_loop_body_block = std::make_unique<ast::block>();
	test_loop_body_block->add_block_statement(std::move(test_loop_body));
	auto test_loop = std::make_unique<ast::while_statement>(
			std::move(test_conditional),
			std::move(test_loop_body_block)
	);

	pp.visit(*test_loop);
	BOOST_REQUIRE_EQUAL("while (i == j) {\n\ti = 0;\n}\n"s, oss.str());
}

BOOST_AUTO_TEST_CASE(pretty_print_empty_while)
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
	auto test_loop = std::make_unique<ast::while_statement>(
			std::move(test_conditional),
			std::make_unique<ast::block>()
	);

	pp.visit(*test_loop);
	BOOST_REQUIRE_EQUAL("while (i == j) { }\n"s, oss.str());
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


namespace /* anonymous */
{
	template <typename T>
	std::enable_if_t<std::is_base_of<ast::node, T>{} && std::is_final<T>{}, std::string>
	serialize(T& ast_node)
	{
		std::ostringstream oss {};
		auto pp = ast::pretty_printer{oss};
		pp.visit(ast_node);
		return oss.str();
	}
}


BOOST_AUTO_TEST_CASE(class_with_var_decl)
{
	using namespace std::string_literals;
	auto pool = minijava::symbol_pool<>{};
	auto cls = std::make_unique<ast::class_declaration>(pool.normalize("Test"));
	{
		const auto id = pool.normalize("apple");
		const auto tpnam = pool.normalize("Apple");
		auto typ = std::make_unique<ast::type>(tpnam);
		auto dcl = std::make_unique<ast::var_decl>(std::move(typ), id);
		cls->add_field(std::move(dcl));
	}
	{
		const auto id = pool.normalize("banana");
		auto typ = std::make_unique<ast::type>(ast::primitive_type::type_int);
		auto dcl = std::make_unique<ast::var_decl>(std::move(typ), id);
		cls->add_field(std::move(dcl));
	}
	{
		const auto id = pool.normalize("cranberry");
		auto typ = std::make_unique<ast::type>(ast::primitive_type::type_boolean);
		auto dcl = std::make_unique<ast::var_decl>(std::move(typ), id);
		cls->add_field(std::move(dcl));
	}
	for (auto rank = std::size_t{}; rank <= 3; ++rank) {
		const auto id = pool.normalize("date");
		auto typ = std::make_unique<ast::type>(ast::primitive_type::type_void, rank);
		auto dcl = std::make_unique<ast::var_decl>(std::move(typ), id);
		cls->add_field(std::move(dcl));
	}
	BOOST_REQUIRE_EQUAL(std::size_t{7}, cls->fields().size());  // sanity-check
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
	const auto actual = serialize(*cls);
	BOOST_REQUIRE_EQUAL(expected, actual);
}
