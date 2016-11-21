#include "semantic/constant.hpp"

#include <cstdint>
#include <iostream>
#include <tuple>
#include <random>
#include <string>
#include <utility>

#define BOOST_TEST_MODULE  semantic_constant
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "parser/ast.hpp"
#include "parser/ast_factory.hpp"
#include "semantic/semantic_error.hpp"
#include "symbol/symbol_pool.hpp"

#include "testaux/random_tokens.hpp"
#include "testaux/unique_ptr_vector.hpp"


namespace ast = minijava::ast;

template <typename T>
using astbldr = minijava::ast_builder<T>;


namespace /* anonymous */
{

	constexpr auto max32 = minijava::ast_int_type{INT32_MAX};
	constexpr auto min32 = minijava::ast_int_type{INT32_MIN};

	std::string random_integer_literal(const std::size_t length)
	{
		static auto engine = std::default_random_engine{};
		return testaux::get_random_integer_literal(engine, length);
	}

}  // namespace /* anonymous */


static const std::tuple<std::string, minijava::ast_int_type> integer_literal_data[] = {
	{"0", 0},
	{"1", 1},
	{"42", 42},
	{"2147483647", INT32_C(2147483647)},
};

BOOST_AUTO_TEST_CASE(integer_literals)
{
	for (const auto& sample : integer_literal_data) {
		auto pool = minijava::symbol_pool<>{};
		auto factory = minijava::ast_factory{};
		const auto lexval = pool.normalize(std::get<0>(sample));
		const auto ast = factory.make<ast::integer_constant>()(lexval);
		const auto extracted = minijava::extract_constants(*ast);
		BOOST_REQUIRE_EQUAL(1, extracted.size());
		BOOST_REQUIRE_EQUAL(std::get<1>(sample), extracted.at(ast->id()));
	}
}


static const std::tuple<bool, minijava::ast_int_type> boolean_literal_data[] = {
	{false, 0},
	{true, 1},
};

BOOST_AUTO_TEST_CASE(boolean_literals)
{
	for (const auto& sample : boolean_literal_data) {
		auto factory = minijava::ast_factory{};
		const auto ast = factory.make<ast::boolean_constant>()(std::get<0>(sample));
		const auto extracted = minijava::extract_constants(*ast);
		BOOST_REQUIRE_EQUAL(1, extracted.size());
		BOOST_REQUIRE_EQUAL(std::get<1>(sample), extracted.at(ast->id()));
	}
}


static const std::string positve_integer_literal_overflow_data[] = {
	"2147483648",
	random_integer_literal(20),
	random_integer_literal(50),
	random_integer_literal(100),
	random_integer_literal(200),
	random_integer_literal(500),
	random_integer_literal(1000),
	random_integer_literal(2000),
	random_integer_literal(5000),
	random_integer_literal(10000),
};

BOOST_DATA_TEST_CASE(positve_integer_literal_overflow, positve_integer_literal_overflow_data)
{
	auto pool = minijava::symbol_pool<>{};
	auto factory = minijava::ast_factory{};
	const auto lexval = pool.normalize(sample);
	const auto ast = factory.make<ast::integer_constant>()(lexval);
	BOOST_REQUIRE_THROW(
		minijava::extract_constants(*ast),
		minijava::semantic_error
	);
}


static const std::tuple<std::string, minijava::ast_int_type> negative_integer_literal_data[] = {
	{"0", 0},
	{"1", 1},
	{"10", 10},
	{"2147483648", INT64_C(2147483648)},
};

BOOST_AUTO_TEST_CASE(negative_integer_literals)
{
	for (const auto& sample : negative_integer_literal_data) {
		auto pool = minijava::symbol_pool<>{};
		const auto lexval = pool.normalize(std::get<0>(sample));
		auto ast = astbldr<ast::unary_expression>{2}(
			ast::unary_operation_type::minus,
			astbldr<ast::integer_constant>{1}(lexval)
		);
		const auto extracted = minijava::extract_constants(*ast);
		BOOST_REQUIRE_EQUAL(2, extracted.size());
		BOOST_REQUIRE_EQUAL(+std::get<1>(sample), extracted.at(1));
		BOOST_REQUIRE_EQUAL(-std::get<1>(sample), extracted.at(2));
	}
}


static const std::tuple<
	ast::binary_operation_type,
	minijava::ast_int_type,
	minijava::ast_int_type,
	minijava::ast_int_type
> binop_data[] = {
	// ||
	{ast::binary_operation_type::logical_or, 0, 0, 0},
	{ast::binary_operation_type::logical_or, 0, 1, 1},
	{ast::binary_operation_type::logical_or, 1, 0, 1},
	{ast::binary_operation_type::logical_or, 1, 1, 1},
	{ast::binary_operation_type::logical_and, 0, 0, 0},
	{ast::binary_operation_type::logical_and, 0, 1, 0},
	{ast::binary_operation_type::logical_and, 1, 0, 0},
	{ast::binary_operation_type::logical_and, 1, 1, 1},
	// ==
	{ast::binary_operation_type::equal, max32, max32, 1},
	{ast::binary_operation_type::equal, 9, 12, 0},
	// !=
	{ast::binary_operation_type::not_equal, max32, max32, 0},
	{ast::binary_operation_type::not_equal, 4, 3, 1},
	// <
	{ast::binary_operation_type::less_than, 5, 7, 1},
	{ast::binary_operation_type::less_than, 5, 5, 0},
	{ast::binary_operation_type::less_than, 7, 5, 0},
	{ast::binary_operation_type::less_than, max32 - 1, max32, 1},
	{ast::binary_operation_type::less_than, max32, max32 - 1, 0},
	// <=
	{ast::binary_operation_type::less_equal, 5, 7, 1},
	{ast::binary_operation_type::less_equal, 5, 5, 1},
	{ast::binary_operation_type::less_equal, 7, 5, 0},
	{ast::binary_operation_type::less_equal, max32 - 1, max32, 1},
	{ast::binary_operation_type::less_equal, max32, max32 - 1, 0},
	// >=
	{ast::binary_operation_type::greater_equal, 5, 7, 0},
	{ast::binary_operation_type::greater_equal, 5, 5, 1},
	{ast::binary_operation_type::greater_equal, 7, 5, 1},
	{ast::binary_operation_type::greater_equal, max32 - 1, max32, 0},
	{ast::binary_operation_type::greater_equal, max32, max32 - 1, 1},
	// >
	{ast::binary_operation_type::greater_than, 5, 7, 0},
	{ast::binary_operation_type::greater_than, 5, 5, 0},
	{ast::binary_operation_type::greater_than, 7, 5, 1},
	{ast::binary_operation_type::greater_than, max32 - 1, max32, 0},
	{ast::binary_operation_type::greater_than, max32, max32 - 1, 1},
	// +
	{ast::binary_operation_type::plus, 5, 7, 12},
	{ast::binary_operation_type::plus, max32, 0, max32},
	// -
	{ast::binary_operation_type::minus, 5, 7, -2},
	{ast::binary_operation_type::minus, 0, max32, -max32},
	// *
	{ast::binary_operation_type::multiply, 5, 7, 35},
	{ast::binary_operation_type::multiply, 0, max32, 0},
	{ast::binary_operation_type::multiply, 1, max32, max32},
	// /
	{ast::binary_operation_type::divide, 5, 7, 0},
	{ast::binary_operation_type::divide, 7, 5, 1},
	{ast::binary_operation_type::divide, max32, max32, 1},
	{ast::binary_operation_type::divide, max32, 1, max32},
	{ast::binary_operation_type::divide, 0, max32, 0},
	{ast::binary_operation_type::divide, 0, 1, 0},
	{ast::binary_operation_type::divide, 10, 4, 2},
	// %
	{ast::binary_operation_type::modulo, 5, 7, 5},
	{ast::binary_operation_type::modulo, 7, 5, 2},
	{ast::binary_operation_type::modulo, 213, 213, 0},
	{ast::binary_operation_type::modulo, max32, max32, 0},
	{ast::binary_operation_type::modulo, max32, 1, 0},
	{ast::binary_operation_type::modulo, 0, max32, 0},
	{ast::binary_operation_type::modulo, 0, 1, 0},
	{ast::binary_operation_type::modulo, 10, 4, 2},
};

BOOST_AUTO_TEST_CASE(binary_operations)
{
	for (const auto& sample : binop_data) {
		auto pool = minijava::symbol_pool<>{};
		const auto bop = std::get<0>(sample);
		const auto lhs = std::get<1>(sample);
		const auto rhs = std::get<2>(sample);
		const auto res = std::get<3>(sample);
		const auto ast = astbldr<ast::binary_expression>{3}(
			bop,
			astbldr<ast::integer_constant>{1}(pool.normalize(std::to_string(lhs))),
			astbldr<ast::integer_constant>{2}(pool.normalize(std::to_string(rhs)))
		);
		const auto extracted = minijava::extract_constants(*ast);
		BOOST_REQUIRE_EQUAL(3, extracted.size());
		BOOST_REQUIRE_EQUAL(lhs, extracted.at(1));
		BOOST_REQUIRE_EQUAL(rhs, extracted.at(2));
		BOOST_REQUIRE_EQUAL(res, extracted.at(3));
	}
}


BOOST_AUTO_TEST_CASE(modulo_has_sign_of_dividend_1st)
{
	auto pool = minijava::symbol_pool<>{};
	const auto ast = astbldr<ast::binary_expression>{4}(
		ast::binary_operation_type::modulo,
		astbldr<ast::integer_constant>{1}(pool.normalize("10")),
		astbldr<ast::unary_expression>{3}(
			ast::unary_operation_type::minus,
			astbldr<ast::integer_constant>{2}(pool.normalize("3"))
		)
	);
	const auto extracted = minijava::extract_constants(*ast);
	BOOST_REQUIRE_EQUAL(1, extracted.at(4));
}


BOOST_AUTO_TEST_CASE(modulo_has_sign_of_dividend_2nd)
{
	auto pool = minijava::symbol_pool<>{};
	const auto ast = astbldr<ast::binary_expression>{4}(
		ast::binary_operation_type::modulo,
		astbldr<ast::unary_expression>{2}(
			ast::unary_operation_type::minus,
			astbldr<ast::integer_constant>{1}(pool.normalize("10"))
		),
		astbldr<ast::integer_constant>{3}(pool.normalize("3"))
	);
	const auto extracted = minijava::extract_constants(*ast);
	BOOST_REQUIRE_EQUAL(-1, extracted.at(4));
}


BOOST_AUTO_TEST_CASE(modulo_has_sign_of_dividend_3rd)
{
	auto pool = minijava::symbol_pool<>{};
	const auto ast = astbldr<ast::binary_expression>{5}(
		ast::binary_operation_type::modulo,
		astbldr<ast::unary_expression>{3}(
			ast::unary_operation_type::minus,
			astbldr<ast::integer_constant>{1}(pool.normalize("10"))
		),
		astbldr<ast::unary_expression>{4}(
			ast::unary_operation_type::minus,
			astbldr<ast::integer_constant>{2}(pool.normalize("3"))
		)
	);
	const auto extracted = minijava::extract_constants(*ast);
	BOOST_REQUIRE_EQUAL(-1, extracted.at(5));
}


BOOST_AUTO_TEST_CASE(unary_integer_invalid)
{
	auto pool = minijava::symbol_pool<>{};
	const auto lexval = pool.normalize(std::to_string(-min32));
	auto ast = astbldr<ast::unary_expression>{3}(
		ast::unary_operation_type::minus,
		astbldr<ast::unary_expression>{2}(
			ast::unary_operation_type::minus,
			astbldr<ast::integer_constant>{1}(lexval)
		)
	);
	auto problems = std::vector<std::size_t>{};
	auto handler = [&problems](const ast::node& n){ problems.push_back(n.id()); };
	const auto extracted = minijava::extract_constants(*ast, handler);
	BOOST_REQUIRE_EQUAL(2, extracted.size());
	BOOST_REQUIRE_EQUAL(-min32, extracted.at(1));
	BOOST_REQUIRE_EQUAL(min32, extracted.at(2));
	BOOST_REQUIRE_EQUAL(1, problems.size());
	BOOST_REQUIRE_EQUAL(3, problems.front());
}


static const std::tuple<
	ast::binary_operation_type,
	minijava::ast_int_type,
	minijava::ast_int_type
> binop_invalid_data[] = {
	{ast::binary_operation_type::plus, max32, 1},
	{ast::binary_operation_type::multiply, max32, max32},
	{ast::binary_operation_type::multiply, 1 + max32 / 2, max32 / 2},
	{ast::binary_operation_type::divide, 6, 0},
	{ast::binary_operation_type::divide, 0, 0},
	{ast::binary_operation_type::modulo, 0, 0},
	{ast::binary_operation_type::modulo, 4, 0},
};

BOOST_AUTO_TEST_CASE(binary_operations_invalid)
{
	for (const auto& sample : binop_invalid_data) {
		auto pool = minijava::symbol_pool<>{};
		const auto bop = std::get<0>(sample);
		const auto lhs = std::get<1>(sample);
		const auto rhs = std::get<2>(sample);
		const auto ast = astbldr<ast::binary_expression>{3}(
			bop,
			astbldr<ast::integer_constant>{1}(pool.normalize(std::to_string(lhs))),
			astbldr<ast::integer_constant>{2}(pool.normalize(std::to_string(rhs)))
		);
		auto problems = std::vector<std::size_t>{};
		auto handler = [&problems](const ast::node& n){ problems.push_back(n.id()); };
		const auto extracted = minijava::extract_constants(*ast, handler);
		BOOST_REQUIRE_EQUAL(2, extracted.size());
		BOOST_REQUIRE_EQUAL(lhs, extracted.at(1));
		BOOST_REQUIRE_EQUAL(rhs, extracted.at(2));
		BOOST_REQUIRE_EQUAL(1, problems.size());
		BOOST_REQUIRE_EQUAL(3, problems.front());
	}
}


BOOST_AUTO_TEST_CASE(complete_program)
{
	auto pool = minijava::symbol_pool<>{};
	auto ast = astbldr<ast::program>{1}(
		testaux::make_unique_ptr_vector<ast::class_declaration>(
			astbldr<ast::class_declaration>{2}(
				pool.normalize("Test"),
				testaux::make_unique_ptr_vector<ast::var_decl>(
					astbldr<ast::var_decl>{3}(
						astbldr<ast::type>{4}(ast::primitive_type::type_int),
						pool.normalize("whatever")
					)
				),
				testaux::make_unique_ptr_vector<ast::instance_method>(
					astbldr<ast::instance_method>{5}(
						pool.normalize("f"),
						astbldr<ast::type>{6}(ast::primitive_type::type_int),
						testaux::make_unique_ptr_vector<ast::var_decl>(),
						astbldr<ast::block>{7}(
							testaux::make_unique_ptr_vector<ast::block_statement>(
								astbldr<ast::local_variable_statement>{8}(
									astbldr<ast::var_decl>{9}(
										astbldr<ast::type>{10}(ast::primitive_type::type_int),
										pool.normalize("a")
									),
									astbldr<ast::binary_expression>{11}(
										ast::binary_operation_type::plus,
										astbldr<ast::integer_constant>{12}(pool.normalize("5")),
										astbldr<ast::integer_constant>{13}(pool.normalize("7"))
									)
								),
								astbldr<ast::return_statement>{14}(
									astbldr<ast::unary_expression>{15}(
										ast::unary_operation_type::minus,
										astbldr<ast::variable_access>{16}(
											std::unique_ptr<ast::expression>{},
											pool.normalize("a")
										)
									)
								)
							)
						)
					)
				),
				testaux::make_unique_ptr_vector<ast::main_method>()
			)
		)
	);
	const auto extracted = minijava::extract_constants(*ast);
	BOOST_REQUIRE_EQUAL(3, extracted.size());
	BOOST_REQUIRE_EQUAL(5, extracted.at(12));
	BOOST_REQUIRE_EQUAL(7, extracted.at(13));
	BOOST_REQUIRE_EQUAL(12, extracted.at(11));
}
