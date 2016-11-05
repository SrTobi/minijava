#include "lexer/lexer.hpp"

#include <algorithm>
#include <forward_list>
#include <string>
#include <iostream>

#define BOOST_TEST_MODULE  lexer_lexer
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "lexer/token.hpp"
#include "lexer/token_iterator.hpp"
#include "symbol_pool.hpp"

#include "../testaux/token_string.hpp"


using namespace std::string_literals; // for strings with embedded null bytes
using testaux::id;
using testaux::lit;
using tt = minijava::token_type;


namespace /* anonymous */
{

	struct lexer_test_data
	{
		std::string input{};
		minijava::symbol_pool<> pool{};
		std::vector<minijava::token> expected{};
	};

	class failure_test
	{
	public:

		template <typename... ArgTs>
		failure_test(std::string&& input, ArgTs&&... args) : _input{std::move(input)}
		{
			_expected = {testaux::make_token(_pool, std::forward<ArgTs>(args))...};
		}

		lexer_test_data get() const
		{
			auto sample = lexer_test_data{};
			sample.input = _input;
			for (const auto& t : _expected) {
				auto copy = t;
				// move symbols to correct pools
				if (t.type() == tt::identifier) {
					auto canon = sample.pool.normalize(t.lexval().c_str());
					copy = minijava::token::create_identifier(canon);
				} else if (t.type() == tt::integer_literal) {
					auto canon = sample.pool.normalize(t.lexval().c_str());
					copy = minijava::token::create_integer_literal(canon);
				}
				sample.expected.push_back(std::move(copy));
			}
			return sample;
		}

	private:

		static minijava::symbol_pool<> _pool;
		std::string _input;
		std::vector<minijava::token> _expected{};

		friend std::ostream& operator<<(std::ostream& os, const failure_test& test)
		{
			return os << test._input;
		}

	};

	minijava::symbol_pool<> failure_test::_pool;

	class success_test : public failure_test
	{
	public:

		template <typename... ArgTs>
		success_test(std::string&& input, ArgTs&&... args)
				: failure_test(std::move(input), std::forward<ArgTs>(args)..., tt::eof) { }
	};

}  // namespace /* anonymous */


BOOST_AUTO_TEST_CASE(empty_input_starts_with_eof_and_stays_there)
{
	const auto epsilon = ""s;
	auto pool = minijava::symbol_pool<>{};
	auto lex = minijava::make_lexer(std::begin(epsilon), std::end(epsilon), pool, pool);
	for (auto i = 0; i < 100; ++i) {
		BOOST_REQUIRE_EQUAL(tt::eof, lex.current_token().type());
		BOOST_REQUIRE(lex.current_token_is_eof());
	}
}


static const auto single_token_data = minijava::all_token_types();

BOOST_DATA_TEST_CASE(single_tokens_are_lexed_correctly, single_token_data)
{
	const auto punct = minijava::token_category::punctuation;
	const auto kw = minijava::token_category::keyword;
	const auto cat = minijava::category(sample);
	if ((cat == punct) || (cat == kw)) {
		const auto text = std::string{minijava::name(sample)};
		const auto input = std::forward_list<char>{std::begin(text), std::end(text)};
		auto pool = minijava::symbol_pool<>{};
		try {
			auto lex = minijava::make_lexer(std::begin(input), std::end(input), pool, pool);
			BOOST_REQUIRE_EQUAL(sample, lex.current_token().type());
			lex.advance();
			BOOST_REQUIRE(lex.current_token_is_eof());
		} catch (const minijava::lexical_error& e) {
			BOOST_FAIL("Unexpected lexical error for input '" << text << "'" << " (" << text.length()
															  << " characters): " << e.what());
		}
	}
}


BOOST_AUTO_TEST_CASE(identifiers_are_put_into_the_correct_pool)
{
	const auto input = "foo"s;
	auto id_pool = minijava::symbol_pool<>{};
	auto lit_pool = minijava::symbol_pool<>{};
	auto lex = minijava::make_lexer(std::begin(input), std::end(input),
	                                id_pool, lit_pool);
	const auto tok = lex.current_token();
	BOOST_REQUIRE(id_pool.is_normalized(tok.lexval().c_str()));
	BOOST_REQUIRE(!lit_pool.is_normalized(tok.lexval().c_str()));
}


BOOST_AUTO_TEST_CASE(integer_literals_are_put_into_the_correct_pool)
{
	const auto input = "42"s;
	auto id_pool = minijava::symbol_pool<>{};
	auto lit_pool = minijava::symbol_pool<>{};
	auto lex = minijava::make_lexer(std::begin(input), std::end(input),
	                                id_pool, lit_pool);
	const auto tok = lex.current_token();
	BOOST_REQUIRE(!id_pool.is_normalized(tok.lexval().c_str()));
	BOOST_REQUIRE(lit_pool.is_normalized(tok.lexval().c_str()));
}


static const success_test success_data[] = {
		// empty input
		{""},
		{" \t\r\n"},

		// identifiers
		{"alpha", id("alpha")},
		{"alpha beta", id("alpha"), id("beta")},
		{"alpha beta gamma delta", id("alpha"), id("beta"), id("gamma"), id("delta")},
		{"alpha6_b3ta123_", id("alpha6_b3ta123_")},

		// comments
		{"/**/"},
		{"/**/alpha beta", id("alpha"), id("beta")},
		{"alpha/**/beta", id("alpha"), id("beta")},
		{"alpha beta/**/", id("alpha"), id("beta")},
		{"/* * / */"},
		{"/*/**/*/", tt::multiply, tt::divides},
		{"**/*= */*", tt::multiply, tt::multiply, tt::multiply},
		{"false/*/***** const auto >= false static[] *\x7F/ ()\0\b\"\xFF ***/="s, tt::kw_false, tt::assign},

		// integer literals
		{"0", lit("0")},
		{"15", lit("15")},
		{"0/**/509720", lit("0"), lit("509720")},
		{"-42 -0 --15", tt::minus, lit("42"), tt::minus, lit("0"), tt::decrement, lit("15")},
		{"00", lit("0"), lit("0")},
		{"000", lit("0"), lit("0"), lit("0")},
		{"*012356--", tt::multiply, lit("0"), lit("12356"), tt::decrement},
		{"<001true", tt::less_than, lit("0"), lit("0"), lit("1"), tt::kw_true},
		{
				"102984084080850832452705977991283408000810923847581234123412341234123412341242134",
				lit("102984084080850832452705977991283408000810923847581234123412341234123412341242134")
		},

		// combinations of identifier, keyword, number and operator
		{
				"constauto static0void private_break _public do1 44true 0for while.if synchronized[] (abstract)",
				// checks that keywords are recognized correctly
				id("constauto"), id("static0void"), id("private_break"), id("_public"), id("do1"), lit("44"),
				tt::kw_true, lit("0"), tt::kw_for, tt::kw_while, tt::dot, tt::kw_if, tt::kw_synchronized,
				tt::left_bracket, tt::right_bracket, tt::left_paren, tt::kw_abstract, tt::right_paren
		},
		{
				"asdf00001 0myvar< test4>=",
				// checks that identifiers are recognized correctly in combination with numbers and operators
				id("asdf00001"), lit("0"), id("myvar"), tt::less_than, id("test4"), tt::greater_equal
		},

		// operators and space types
		{
				">>>>===----=**&&&&&===>>>=&&=&&&>><<<",
				tt::unsigned_right_shift, tt::greater_equal, tt::equal, tt::decrement, tt::decrement, tt::assign,
				tt::multiply, tt::multiply, tt::logical_and, tt::logical_and, tt::bit_and_assign, tt::equal,
				tt::unsigned_right_shift_assign, tt::logical_and, tt::assign, tt::logical_and, tt::bit_and,
				tt::right_shift, tt::left_shift, tt::less_than
		},
		{">/*>>>=*/>>=", tt::greater_than, tt::right_shift_assign},
		{"*\t= =\r=\n=\t\r\n=", tt::multiply, tt::assign, tt::assign, tt::assign, tt::assign, tt::assign},
};

BOOST_DATA_TEST_CASE(input_lexed_correctly, success_data)
{
	auto s = sample.get();
	auto lex = minijava::make_lexer(std::begin(s.input), std::end(s.input), s.pool, s.pool);
	BOOST_CHECK(std::equal(std::begin(s.expected), std::end(s.expected),
						   minijava::token_begin(lex), minijava::token_end(lex)));
}


static const failure_test failure_data[] = {
		// invalid spaces
		{"*\v=", tt::multiply},
		{"*\f=", tt::multiply},

		// random null bytes are not misinterpreted as EOF and token before is returned correctly
		{"1234 \0 false"s, lit("1234")},
		{"1234\0 false"s, lit("1234")},
		{"ident\0 false"s, id("ident")},
		{">\0 false"s, tt::greater_than},
		{":\0 false"s, tt::colon},

		// other bad characters in various environments
		{"\b"},
		{"asdfghewr\\0a", id("asdfghewr")},
		{"void\"", tt::kw_void},
		{"1234\x7F", lit("1234")},

		// invalid comments
		{"/*"},
};

BOOST_DATA_TEST_CASE(incorrect_input_lexed_correctly, failure_data)
{
	auto s = sample.get();
	if (s.expected.empty()) {
		BOOST_CHECK_THROW(
			minijava::make_lexer(std::begin(s.input), std::end(s.input), s.pool, s.pool),
			minijava::lexical_error
		);
	} else {
		auto lex = minijava::make_lexer(std::begin(s.input), std::end(s.input), s.pool, s.pool);
		for (std::size_t i = 0; i < s.expected.size() - 1; ++i, lex.advance()) {
			BOOST_CHECK_EQUAL(s.expected[i], lex.current_token());
		}
		BOOST_CHECK_EQUAL(s.expected.back(), lex.current_token());
		BOOST_CHECK_THROW(lex.advance(), minijava::lexical_error);
	}
}


BOOST_AUTO_TEST_CASE(excessive_sequence_of_block_comments_does_not_crash_lexer)
{
	const auto input = [](){
		auto in = ""s;
		for (auto i = 0L; i < 10'000'000L; ++i) { in += "/**/"; }
		in += ";";
		return in;
	}();
	auto pool = minijava::symbol_pool<>{};
	auto lex = minijava::make_lexer(std::begin(input), std::end(input), pool, pool);
	BOOST_REQUIRE_EQUAL(tt::semicolon, lex.current_token().type());
}

BOOST_AUTO_TEST_CASE(line_and_column_test)
{
	int expected[][2] = {{1,1}, {1,3}, {2,1}, {2,2}, {2,4}, {3,2}};
	std::string input = "a b\n+a hallo\n\twelt";
	auto pool = minijava::symbol_pool<>{};
	auto lex = minijava::make_lexer(std::begin(input), std::end(input), pool, pool);

	auto size = sizeof(expected) / sizeof(expected[0]);
	for (uint32_t i = 0; i < size; i++) {
		auto current = lex.current_token();
		BOOST_REQUIRE_EQUAL(current.line(), expected[i][0]);
		BOOST_REQUIRE_EQUAL(current.column(), expected[i][1]);
		lex.advance();
	}

}
