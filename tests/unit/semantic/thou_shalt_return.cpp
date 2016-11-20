#include "semantic/thou_shalt_return.hpp"

#include <utility>

#define BOOST_TEST_MODULE  semantic_thou_shalt_return
#include <boost/test/unit_test.hpp>

#include "parser/ast.hpp"
#include "parser/ast_factory.hpp"
#include "semantic/semantic_error.hpp"
#include "symbol/symbol_pool.hpp"

#include "testaux/unique_ptr_vector.hpp"

namespace ast = minijava::ast;


namespace /* anonymous */
{

	auto g_pool = minijava::symbol_pool<>{};
	auto g_fact = minijava::ast_factory{};

	auto as_program(std::vector<std::unique_ptr<ast::block_statement>> body,
	                std::unique_ptr<ast::type> rettype)
	{
		auto meth = g_fact.make<ast::method>()(
			g_pool.normalize("test"),
			std::move(rettype),
			testaux::make_unique_ptr_vector<ast::var_decl>(),
			g_fact.make<ast::block>()(std::move(body))
		);
		return g_fact.make<ast::program>()(
			testaux::make_unique_ptr_vector<ast::class_declaration>(
				g_fact.make<ast::class_declaration>()(
					g_pool.normalize("Test"),
					testaux::make_unique_ptr_vector<ast::var_decl>(),
					testaux::make_unique_ptr_vector<ast::method>(std::move(meth)),
					testaux::make_unique_ptr_vector<ast::main_method>(
					g_fact.make<ast::main_method>()(
							g_pool.normalize("main"),
							g_pool.normalize("args"),
							g_fact.make<ast::block>()(
								testaux::make_unique_ptr_vector<ast::block_statement>()
							)
						)
					)
				)
			)
		);
	}

	auto as_program(std::vector<std::unique_ptr<ast::block_statement>> body)
	{
		auto rettype = g_fact.make<ast::type>()(ast::primitive_type::type_int);
		return as_program(std::move(body), std::move(rettype));
	}

}  // namespace /* anonymous */


BOOST_AUTO_TEST_CASE(empty_void_function_is_okay)
{
	const auto ast = as_program(
		testaux::make_unique_ptr_vector<ast::block_statement>(),
		g_fact.make<ast::type>()(ast::primitive_type::type_void)
	);
	minijava::check_return_paths(*ast);
}


BOOST_AUTO_TEST_CASE(empty_int_function_is_not_okay)
{
	const auto ast = as_program(
		testaux::make_unique_ptr_vector<ast::block_statement>()
	);
	BOOST_REQUIRE_THROW(
		minijava::check_return_paths(*ast),
		minijava::semantic_error
	);
}


BOOST_AUTO_TEST_CASE(empty_ref_function_is_not_okay)
{
	const auto ast = as_program(
		testaux::make_unique_ptr_vector<ast::block_statement>(),
		g_fact.make<ast::type>()(g_pool.normalize("Object"))
	);
	BOOST_REQUIRE_THROW(
		minijava::check_return_paths(*ast),
		minijava::semantic_error
	);
}


BOOST_AUTO_TEST_CASE(int_with_return_is_okay)
{
	const auto ast = as_program(
		testaux::make_unique_ptr_vector<ast::block_statement>(
			g_fact.make<ast::return_statement>()(
				g_fact.make<ast::integer_constant>()(g_pool.normalize("0"))
			)
		)
	);
	minijava::check_return_paths(*ast);
}


BOOST_AUTO_TEST_CASE(int_with_conditional_return_is_not_okay_1st)
{
	const auto ast = as_program(
		testaux::make_unique_ptr_vector<ast::block_statement>(
			g_fact.make<ast::if_statement>()(
				g_fact.make<ast::boolean_constant>()(true),
				g_fact.make<ast::return_statement>()(
					g_fact.make<ast::integer_constant>()(g_pool.normalize("0"))
				),
				std::unique_ptr<ast::statement>{}
			)
		)
	);
	BOOST_REQUIRE_THROW(
		minijava::check_return_paths(*ast),
		minijava::semantic_error
	);
}


BOOST_AUTO_TEST_CASE(int_with_conditional_return_is_not_okay_2nd)
{
	const auto ast = as_program(
		testaux::make_unique_ptr_vector<ast::block_statement>(
			g_fact.make<ast::if_statement>()(
				g_fact.make<ast::boolean_constant>()(true),
				g_fact.make<ast::return_statement>()(
					g_fact.make<ast::integer_constant>()(g_pool.normalize("0"))
				),
				g_fact.make<ast::empty_statement>()()
			)
		)
	);
	BOOST_REQUIRE_THROW(
		minijava::check_return_paths(*ast),
		minijava::semantic_error
	);
}


BOOST_AUTO_TEST_CASE(int_with_conditional_return_is_not_okay_3rd)
{
	const auto ast = as_program(
		testaux::make_unique_ptr_vector<ast::block_statement>(
			g_fact.make<ast::if_statement>()(
				g_fact.make<ast::boolean_constant>()(true),
				g_fact.make<ast::empty_statement>()(),
				g_fact.make<ast::return_statement>()(
					g_fact.make<ast::integer_constant>()(g_pool.normalize("0"))
				)
			)
		)
	);
	BOOST_REQUIRE_THROW(
		minijava::check_return_paths(*ast),
		minijava::semantic_error
	);
}


BOOST_AUTO_TEST_CASE(returning_once_in_a_while_is_not_sufficient)
{
	const auto ast = as_program(
		testaux::make_unique_ptr_vector<ast::block_statement>(
			g_fact.make<ast::while_statement>()(
				g_fact.make<ast::boolean_constant>()(true),
				g_fact.make<ast::return_statement>()(
					g_fact.make<ast::integer_constant>()(g_pool.normalize("0"))
				)
			)
		)
	);
	BOOST_REQUIRE_THROW(
		minijava::check_return_paths(*ast),
		minijava::semantic_error
	);
}


BOOST_AUTO_TEST_CASE(int_with_double_conditional_return_is_okay)
{
	const auto ast = as_program(
		testaux::make_unique_ptr_vector<ast::block_statement>(
			g_fact.make<ast::if_statement>()(
				g_fact.make<ast::boolean_constant>()(true),
				g_fact.make<ast::return_statement>()(
					g_fact.make<ast::integer_constant>()(g_pool.normalize("1"))
				),
				g_fact.make<ast::return_statement>()(
					g_fact.make<ast::integer_constant>()(g_pool.normalize("1"))
				)
			)
		)
	);
	minijava::check_return_paths(*ast);
}


BOOST_AUTO_TEST_CASE(int_with_triple_conditional_return_is_okay)
{
	const auto ast = as_program(
		testaux::make_unique_ptr_vector<ast::block_statement>(
			g_fact.make<ast::if_statement>()(
				g_fact.make<ast::boolean_constant>()(false),
				g_fact.make<ast::return_statement>()(
					g_fact.make<ast::integer_constant>()(g_pool.normalize("1"))
				),
				g_fact.make<ast::if_statement>()(
					g_fact.make<ast::boolean_constant>()(false),
					g_fact.make<ast::return_statement>()(
						g_fact.make<ast::integer_constant>()(g_pool.normalize("2"))
					),
					g_fact.make<ast::return_statement>()(
						g_fact.make<ast::integer_constant>()(g_pool.normalize("3"))
					)
				)
			)
		)
	);
	minijava::check_return_paths(*ast);
}


BOOST_AUTO_TEST_CASE(return_cascase_is_okay)
{
	const auto ast = as_program(
		testaux::make_unique_ptr_vector<ast::block_statement>(
			g_fact.make<ast::return_statement>()(
				g_fact.make<ast::integer_constant>()(g_pool.normalize("1"))
			),
			g_fact.make<ast::return_statement>()(
				g_fact.make<ast::integer_constant>()(g_pool.normalize("2"))
			),
			g_fact.make<ast::return_statement>()(
				g_fact.make<ast::integer_constant>()(g_pool.normalize("3"))
			),
			g_fact.make<ast::return_statement>()(
				g_fact.make<ast::integer_constant>()(g_pool.normalize("4"))
			),
			g_fact.make<ast::return_statement>()(
				g_fact.make<ast::integer_constant>()(g_pool.normalize("5"))
			),
			g_fact.make<ast::return_statement>()(
				g_fact.make<ast::integer_constant>()(g_pool.normalize("6"))
			)
		)
	);
	minijava::check_return_paths(*ast);
}


BOOST_AUTO_TEST_CASE(single_return_from_deeply_nested_block_is_okay)
{
	const auto ast = as_program(
		testaux::make_unique_ptr_vector<ast::block_statement>(
			g_fact.make<ast::block>()(
				testaux::make_unique_ptr_vector<ast::block_statement>(
					g_fact.make<ast::block>()(
						testaux::make_unique_ptr_vector<ast::block_statement>(
							g_fact.make<ast::block>()(
								testaux::make_unique_ptr_vector<ast::block_statement>(
									g_fact.make<ast::return_statement>()(
										g_fact.make<ast::integer_constant>()(g_pool.normalize("0"))
									)
								)
							)
						)
					)
				)
			)
		)
	);
	minijava::check_return_paths(*ast);
}
