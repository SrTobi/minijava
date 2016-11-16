#include "parser/ast.hpp"

#define BOOST_TEST_MODULE  parser_ast
#include <boost/test/unit_test.hpp>

#include <type_traits>

#include "testaux/meta.hpp"

namespace ast = minijava::ast;


namespace /* anonymous */
{

	using all_ast_node_types = testaux::meta::type_list
	<
		ast::node,
		ast::type,
		ast::var_decl,
		ast::expression,
		ast::binary_expression,
		ast::unary_expression,
		ast::object_instantiation,
		ast::array_instantiation,
		ast::array_access,
		ast::variable_access,
		ast::method_invocation,
		ast::this_ref,
		ast::constant,
		ast::boolean_constant,
		ast::integer_constant,
		ast::null_constant,
		ast::block_statement,
		ast::local_variable_statement,
		ast::statement,
		ast::expression_statement,
		ast::block,
		ast::if_statement,
		ast::while_statement,
		ast::return_statement,
		ast::empty_statement,
		ast::main_method,
		ast::method,
		ast::class_declaration,
		ast::program
	>;

}  // namespace /* anonymous */


BOOST_AUTO_TEST_CASE(type_checks)
{
	constexpr auto ants = all_ast_node_types{};
	static_assert(testaux::meta::all<std::is_class>(ants), "");
	static_assert(testaux::meta::all<std::has_virtual_destructor>(ants), "");
	static_assert(testaux::meta::none<std::is_copy_constructible>(ants), "");
	static_assert(testaux::meta::none<std::is_copy_assignable>(ants), "");
	static_assert(testaux::meta::none<std::is_move_constructible>(ants), "");
	static_assert(testaux::meta::none<std::is_move_assignable>(ants), "");
	// TODO: I'd also like to check that each class is either `final` or
	// abstract (has at least one pure `virtual` member function).  Alas, I
	// don't know how to check the latter predicate.
}
