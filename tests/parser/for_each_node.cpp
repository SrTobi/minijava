#include "parser/for_each_node.hpp"

#define BOOST_TEST_MODULE  parser_for_each_node
#include <boost/test/unit_test.hpp>

#include "parser/ast.hpp"
#include "symbol/symbol_pool.hpp"
#include "testaux/unique_ptr_vector.hpp"

namespace ast = minijava::ast;


namespace /* anonymous */
{

	struct counting_visitor : minijava::for_each_node
	{

		void visit_node(const ast::node& /* node */) override
		{
			++tally;
		}

		int tally{};

	};

}

BOOST_AUTO_TEST_CASE(count_nodes_in_empty_ast)
{
	const auto ast = std::make_unique<ast::empty_statement>();
	auto cv = counting_visitor{};
	ast->accept(cv);
	BOOST_REQUIRE_EQUAL(1, cv.tally);
}


BOOST_AUTO_TEST_CASE(count_nodes_in_simple_ast)
{
	auto pool = minijava::symbol_pool<>{};
	const auto ast = std::make_unique<ast::program>(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			std::make_unique<ast::class_declaration>(
				pool.normalize("Test"),
				testaux::make_unique_ptr_vector<ast::var_decl>(
					std::make_unique<ast::var_decl>(
						std::make_unique<ast::type>(pool.normalize("Foo"), 1),
						pool.normalize("foos")
					)
				),
				testaux::make_unique_ptr_vector<ast::method>(),
				testaux::make_unique_ptr_vector<ast::main_method>()
			)
		)
	);
	auto cv = counting_visitor{};
	ast->accept(cv);
	BOOST_REQUIRE_EQUAL(4, cv.tally);
}
