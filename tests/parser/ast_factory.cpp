#include "parser/ast_factory.hpp"

#define BOOST_TEST_MODULE  parser_ast_factory
#include <boost/test/unit_test.hpp>

#include "parser/ast.hpp"

using pos = minijava::position;

BOOST_AUTO_TEST_CASE(builder_default)
{
	const auto integer = minijava::ast::primitive_type::type_int;
	const auto ab = minijava::ast_builder<minijava::ast::type>{};
	const auto node = ab(integer, 100);
	BOOST_REQUIRE_EQUAL(std::size_t{}, node->id());
	BOOST_REQUIRE_EQUAL(std::size_t{}, node->position().line());
	BOOST_REQUIRE_EQUAL(std::size_t{}, node->position().column());
	BOOST_REQUIRE(integer == boost::get<minijava::ast::primitive_type>(node->name()));
	BOOST_REQUIRE_EQUAL(std::size_t{100}, node->rank());
}


BOOST_AUTO_TEST_CASE(builder_id)
{
	const auto integer = minijava::ast::primitive_type::type_int;
	const auto ab = minijava::ast_builder<minijava::ast::type>{42};
	const auto node = ab(integer, 100);
	BOOST_REQUIRE_EQUAL(std::size_t{42}, node->id());
	BOOST_REQUIRE_EQUAL(std::size_t{}, node->position().line());
	BOOST_REQUIRE_EQUAL(std::size_t{}, node->position().column());
	BOOST_REQUIRE(integer == boost::get<minijava::ast::primitive_type>(node->name()));
	BOOST_REQUIRE_EQUAL(std::size_t{100}, node->rank());
}


BOOST_AUTO_TEST_CASE(builder_line)
{
	const auto integer = minijava::ast::primitive_type::type_int;
	auto ab = minijava::ast_builder<minijava::ast::type>{};
	const auto node = ab.at(pos(5, 0))(integer, 100);
	BOOST_REQUIRE_EQUAL(std::size_t{}, node->id());
	BOOST_REQUIRE_EQUAL(std::size_t{5}, node->position().line());
	BOOST_REQUIRE_EQUAL(std::size_t{0}, node->position().column());
	BOOST_REQUIRE(integer == boost::get<minijava::ast::primitive_type>(node->name()));
	BOOST_REQUIRE_EQUAL(std::size_t{100}, node->rank());
}


BOOST_AUTO_TEST_CASE(builder_column)
{
	const auto integer = minijava::ast::primitive_type::type_int;
	auto ab = minijava::ast_builder<minijava::ast::type>{};
	const auto node = ab.at(pos(0, 8))(integer, 100);
	BOOST_REQUIRE_EQUAL(std::size_t{}, node->id());
	BOOST_REQUIRE_EQUAL(std::size_t{0}, node->position().line());
	BOOST_REQUIRE_EQUAL(std::size_t{8}, node->position().column());
	BOOST_REQUIRE(integer == boost::get<minijava::ast::primitive_type>(node->name()));
	BOOST_REQUIRE_EQUAL(std::size_t{100}, node->rank());
}


BOOST_AUTO_TEST_CASE(builder_line_and_column)
{
	const auto integer = minijava::ast::primitive_type::type_int;
	auto ab = minijava::ast_builder<minijava::ast::type>{};
	const auto node = ab.at(pos(3, 6))(integer, 100);
	BOOST_REQUIRE_EQUAL(std::size_t{}, node->id());
	BOOST_REQUIRE_EQUAL(std::size_t{3}, node->position().line());
	BOOST_REQUIRE_EQUAL(std::size_t{6}, node->position().column());
	BOOST_REQUIRE(integer == boost::get<minijava::ast::primitive_type>(node->name()));
	BOOST_REQUIRE_EQUAL(std::size_t{100}, node->rank());
}


BOOST_AUTO_TEST_CASE(builder_column_and_line)
{
	const auto integer = minijava::ast::primitive_type::type_int;
	auto ab = minijava::ast_builder<minijava::ast::type>{};
	const auto node = ab.at(pos(3, 6))(integer, 100);
	BOOST_REQUIRE_EQUAL(std::size_t{}, node->id());
	BOOST_REQUIRE_EQUAL(std::size_t{3}, node->position().line());
	BOOST_REQUIRE_EQUAL(std::size_t{6}, node->position().column());
	BOOST_REQUIRE(integer == boost::get<minijava::ast::primitive_type>(node->name()));
	BOOST_REQUIRE_EQUAL(std::size_t{100}, node->rank());
}


BOOST_AUTO_TEST_CASE(builder_explicit)
{
	const auto integer = minijava::ast::primitive_type::type_int;
	auto ab = minijava::ast_builder<minijava::ast::type>{12};
	const auto node = ab.at(pos(23, 34))(integer, 100);
	BOOST_REQUIRE_EQUAL(std::size_t{12}, node->id());
	BOOST_REQUIRE_EQUAL(std::size_t{23}, node->position().line());
	BOOST_REQUIRE_EQUAL(std::size_t{34}, node->position().column());
	BOOST_REQUIRE(integer == boost::get<minijava::ast::primitive_type>(node->name()));
	BOOST_REQUIRE_EQUAL(std::size_t{100}, node->rank());
}


BOOST_AUTO_TEST_CASE(factory_created_nodes_have_successive_ids_starting_at_one)
{
	// We use an outer loop to make sure there is no static state.
	for (auto i = 0; i < 10; ++i) {
		auto af = minijava::ast_factory{};
		for (auto j = std::size_t{1}; j < std::size_t{10}; ++j) {
			auto np = af.make<minijava::ast::empty_statement>()();
			BOOST_REQUIRE_EQUAL(j, np->id());
		}
	}
}
