#include "parser/ast_factory.hpp"

#define BOOST_TEST_MODULE  parser_ast_factory
#include <boost/test/unit_test.hpp>

#include "parser/ast.hpp"


BOOST_AUTO_TEST_CASE(builder_default)
{
	const auto integer = minijava::ast::primitive_type::type_int;
	const auto ab = minijava::ast_builder<minijava::ast::type>{};
	const auto node = ab(integer, 100);
	BOOST_REQUIRE_EQUAL(std::size_t{}, node->id());
	BOOST_REQUIRE_EQUAL(std::size_t{}, node->line());
	BOOST_REQUIRE_EQUAL(std::size_t{}, node->column());
	BOOST_REQUIRE(integer == boost::get<minijava::ast::primitive_type>(node->name()));
	BOOST_REQUIRE_EQUAL(std::size_t{100}, node->rank());
}


BOOST_AUTO_TEST_CASE(builder_id)
{
	const auto integer = minijava::ast::primitive_type::type_int;
	const auto ab = minijava::ast_builder<minijava::ast::type>{42};
	const auto node = ab(integer, 100);
	BOOST_REQUIRE_EQUAL(std::size_t{42}, node->id());
	BOOST_REQUIRE_EQUAL(std::size_t{}, node->line());
	BOOST_REQUIRE_EQUAL(std::size_t{}, node->column());
	BOOST_REQUIRE(integer == boost::get<minijava::ast::primitive_type>(node->name()));
	BOOST_REQUIRE_EQUAL(std::size_t{100}, node->rank());
}


BOOST_AUTO_TEST_CASE(builder_line)
{
	const auto integer = minijava::ast::primitive_type::type_int;
	auto ab = minijava::ast_builder<minijava::ast::type>{};
	const auto node = ab.at_line(5)(integer, 100);
	BOOST_REQUIRE_EQUAL(std::size_t{}, node->id());
	BOOST_REQUIRE_EQUAL(std::size_t{5}, node->line());
	BOOST_REQUIRE_EQUAL(std::size_t{}, node->column());
	BOOST_REQUIRE(integer == boost::get<minijava::ast::primitive_type>(node->name()));
	BOOST_REQUIRE_EQUAL(std::size_t{100}, node->rank());
}


BOOST_AUTO_TEST_CASE(builder_column)
{
	const auto integer = minijava::ast::primitive_type::type_int;
	auto ab = minijava::ast_builder<minijava::ast::type>{};
	const auto node = ab.at_column(8)(integer, 100);
	BOOST_REQUIRE_EQUAL(std::size_t{}, node->id());
	BOOST_REQUIRE_EQUAL(std::size_t{}, node->line());
	BOOST_REQUIRE_EQUAL(std::size_t{8}, node->column());
	BOOST_REQUIRE(integer == boost::get<minijava::ast::primitive_type>(node->name()));
	BOOST_REQUIRE_EQUAL(std::size_t{100}, node->rank());
}


BOOST_AUTO_TEST_CASE(builder_line_and_column)
{
	const auto integer = minijava::ast::primitive_type::type_int;
	auto ab = minijava::ast_builder<minijava::ast::type>{};
	const auto node = ab.at_line(3).at_column(6)(integer, 100);
	BOOST_REQUIRE_EQUAL(std::size_t{}, node->id());
	BOOST_REQUIRE_EQUAL(std::size_t{3}, node->line());
	BOOST_REQUIRE_EQUAL(std::size_t{6}, node->column());
	BOOST_REQUIRE(integer == boost::get<minijava::ast::primitive_type>(node->name()));
	BOOST_REQUIRE_EQUAL(std::size_t{100}, node->rank());
}


BOOST_AUTO_TEST_CASE(builder_column_and_line)
{
	const auto integer = minijava::ast::primitive_type::type_int;
	auto ab = minijava::ast_builder<minijava::ast::type>{};
	const auto node = ab.at_column(6).at_line(3)(integer, 100);
	BOOST_REQUIRE_EQUAL(std::size_t{}, node->id());
	BOOST_REQUIRE_EQUAL(std::size_t{3}, node->line());
	BOOST_REQUIRE_EQUAL(std::size_t{6}, node->column());
	BOOST_REQUIRE(integer == boost::get<minijava::ast::primitive_type>(node->name()));
	BOOST_REQUIRE_EQUAL(std::size_t{100}, node->rank());
}


BOOST_AUTO_TEST_CASE(builder_explicit)
{
	const auto integer = minijava::ast::primitive_type::type_int;
	auto ab = minijava::ast_builder<minijava::ast::type>{12};
	const auto node = ab.at_line(23).at_column(34)(integer, 100);
	BOOST_REQUIRE_EQUAL(std::size_t{12}, node->id());
	BOOST_REQUIRE_EQUAL(std::size_t{23}, node->line());
	BOOST_REQUIRE_EQUAL(std::size_t{34}, node->column());
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


BOOST_AUTO_TEST_CASE(factory_can_start_at_explicit_id)
{
	const auto offset = std::size_t{1234};
	auto af = minijava::ast_factory{offset};
	for (auto i = std::size_t{1}; i < std::size_t{10}; ++i) {
		auto np = af.make<minijava::ast::empty_statement>()();
		BOOST_REQUIRE_EQUAL(offset + i, np->id());
	}
}
