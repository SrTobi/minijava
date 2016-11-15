#include "parser/pretty_printer.hpp"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#define BOOST_TEST_MODULE  parser_pretty_printer
#include <boost/test/unit_test.hpp>

#include "symbol/symbol_pool.hpp"

#include "testaux/unique_ptr_vector.hpp"

namespace ast = minijava::ast;
template<typename T>
using ast_vector = std::vector<std::unique_ptr<T>>;

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
	auto test_ast = std::make_unique<ast::program>(
			ast_vector<ast::class_declaration>{}
	);
	auto pp = ast::pretty_printer{oss};
	pp.visit(*test_ast);
	BOOST_REQUIRE_EQUAL(""s, oss.str());
}

BOOST_AUTO_TEST_CASE(pretty_print_single_class)
{
	using namespace std::string_literals;
	std::ostringstream oss {};
	auto pool = minijava::symbol_pool<>{};
	auto test_ast = std::make_unique<ast::program>(
			testaux::make_unique_ptr_vector<ast::class_declaration>(
					std::make_unique<ast::class_declaration>(
							pool.normalize("test_class"),
					        ast_vector<ast::var_decl>{},
					        ast_vector<ast::method>{},
					        ast_vector<ast::main_method>{}
					)
			)
	);
	auto pp = ast::pretty_printer{oss};
	pp.visit(*test_ast);
	BOOST_REQUIRE_EQUAL("class test_class { }\n"s, oss.str());
}

BOOST_AUTO_TEST_CASE(pretty_print_class_with_method)
{
	using namespace std::string_literals;
	std::ostringstream oss {};
	auto pool = minijava::symbol_pool<>{};

	auto foo_method = std::make_unique<ast::method>(
			pool.normalize("foo"),
			std::make_unique<ast::type>(ast::primitive_type::type_int, 0),
			ast_vector<ast::var_decl>{},
			std::make_unique<ast::block>(
					testaux::make_unique_ptr_vector<ast::block_statement>(
							std::make_unique<ast::expression_statement>(
									std::make_unique<ast::integer_constant>(pool.normalize("123"))
							)
					)
			)
	);
	auto hello_world_class = std::make_unique<ast::class_declaration>(
			pool.normalize("HelloWorld"),
	        ast_vector<ast::var_decl>{},
	        testaux::make_unique_ptr_vector<ast::method>(
			        std::move(foo_method)
	        ),
	        ast_vector<ast::main_method>{}
	);
	auto test_ast = std::make_unique<ast::program>(
			testaux::make_unique_ptr_vector<ast::class_declaration>(
					std::move(hello_world_class)
			)
	);
	auto pp = ast::pretty_printer{oss};
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
	auto test_else_then_statement = std::make_unique<ast::block>(
			testaux::make_unique_ptr_vector<ast::block_statement>(
					std::make_unique<ast::expression_statement>(
							std::make_unique<ast::binary_expression>(
									ast::binary_operation_type::assign,
									std::make_unique<ast::variable_access>(
											nullptr, pool.normalize("i")
									),
									std::make_unique<ast::integer_constant>(
											pool.normalize("0")
									)
							)
					)
			)
	);
	auto test_conditional_block = std::make_unique<ast::if_statement>(
			std::move(test_conditional),
			std::move(test_then_statement),
			std::make_unique<ast::if_statement>(
					std::move(test_else_conditional),
					std::move(test_else_then_statement),
					std::make_unique<ast::block>(
							ast_vector<ast::block_statement>{}
					)
			)
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
			        std::make_unique<ast::block>(
					        ast_vector<ast::block_statement>{}
			        )
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
	auto test_loop = std::make_unique<ast::while_statement>(
			std::move(test_conditional),
			std::make_unique<ast::block>(
					testaux::make_unique_ptr_vector<ast::block_statement>(
							std::move(test_loop_body)
					)
			)
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
			std::make_unique<ast::block>(
					ast_vector<ast::block_statement>{}
			)
	);

	pp.visit(*test_loop);
	BOOST_REQUIRE_EQUAL("while (i == j) { }\n"s, oss.str());
}

BOOST_AUTO_TEST_CASE(pretty_print_statements_and_expressions)
{
	using namespace std::string_literals;
	std::ostringstream oss {};
	auto pool = minijava::symbol_pool<>{};

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
	ast_vector<ast::expression> test_arguments {};
	test_arguments.push_back(std::move(test_argument));
	auto test_statement = std::make_unique<ast::expression_statement>(
			std::make_unique<ast::method_invocation>(
					nullptr,
			        pool.normalize("f"),
					std::move(test_arguments)
			)
	);
	auto test_block = std::make_unique<ast::block>(
			testaux::make_unique_ptr_vector<ast::block_statement>(
					std::move(test_local),
			        std::move(test_empty),
			        std::move(test_statement)
			)
	);

	auto pp = ast::pretty_printer{oss};
	pp.visit(*test_block);
	BOOST_REQUIRE_EQUAL(
			"{\n\tint i = (3 * 5) + 10;\n\tf(((new bar[2][])[1])[i]);\n}\n"s,
	        oss.str()
	);
}


namespace /* anonymous */
{
	std::string serialize(const ast::node& ast_node)
	{
		std::ostringstream oss {};
		auto pp = ast::pretty_printer{oss};
		ast_node.accept(pp);
		return oss.str();
	}
}


BOOST_AUTO_TEST_CASE(class_with_var_decl)
{
	using namespace std::string_literals;
	auto pool = minijava::symbol_pool<>{};
	auto cls = std::make_unique<ast::class_declaration>(
			pool.normalize("Test"),
	        testaux::make_unique_ptr_vector<ast::var_decl>(
			        std::make_unique<ast::var_decl>(
					        std::make_unique<ast::type>(
							        pool.normalize("Apple")
					        ),
					        pool.normalize("apple")
			        ),
			        std::make_unique<ast::var_decl>(
					        std::make_unique<ast::type>(
							        ast::primitive_type::type_int
					        ),
					        pool.normalize("banana")
			        ),
			        std::make_unique<ast::var_decl>(
					        std::make_unique<ast::type>(
							        ast::primitive_type::type_boolean
					        ),
					        pool.normalize("cranberry")
			        ),
			        std::make_unique<ast::var_decl>(
					        std::make_unique<ast::type>(
							        ast::primitive_type::type_void, 0
					        ),
					        pool.normalize("date")
			        ),
			        std::make_unique<ast::var_decl>(
					        std::make_unique<ast::type>(
							        ast::primitive_type::type_void, 1
					        ),
					        pool.normalize("date")
			        ),
			        std::make_unique<ast::var_decl>(
					        std::make_unique<ast::type>(
							        ast::primitive_type::type_void, 2
					        ),
					        pool.normalize("date")
			        ),
			        std::make_unique<ast::var_decl>(
					        std::make_unique<ast::type>(
							        ast::primitive_type::type_void, 3
					        ),
					        pool.normalize("date")
			        )
	        ),
	        ast_vector<ast::method>{},
	        ast_vector<ast::main_method>{}
	);
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
