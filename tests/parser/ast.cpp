#include "parser/ast.hpp"

#define BOOST_TEST_MODULE  parser_ast
#include <boost/test/unit_test.hpp>

#include <type_traits>

namespace ast = minijava::ast;


BOOST_AUTO_TEST_CASE(ast_nodes_noncopyable)
{
	static_assert(
			!std::is_move_constructible<ast::program>{},
			"nodes should not be copyable"
	);
}
