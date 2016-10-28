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
			_expected = {_make_expected_token(std::forward<ArgTs>(args))...,
						 minijava::token::create(minijava::token_type::eof)};
		}

		lexer_test_data get() const
		{
			auto sample = lexer_test_data{};
			sample.input = _input;
			for (const auto& t : _expected) {
				auto copy = t;
				if (t.type() == minijava::token_type::identifier) {
					auto canon = sample.pool.normalize(t.name().c_str());
					copy = minijava::token::create_identifier(canon);
				}
				sample.expected.push_back(std::move(copy));
			}
			return sample;
		}

	private:

		auto _make_expected_token(const char *const name)
		{
			const auto canon = _pool.normalize(name);
			return minijava::token::create_identifier(canon);
		}

		auto _make_expected_token(const std::uint32_t value)
		{
			return minijava::token::create_integer_literal(value);
		}

		auto _make_expected_token(const minijava::token_type tt)
		{
			return minijava::token::create(tt);
		}

		minijava::symbol_pool<> _pool{};
		std::string _input;
		std::vector<minijava::token> _expected{};

		friend std::ostream& operator<<(std::ostream& os, const failure_test& test)
		{
			return os << test._input;
		}

	};

	class success_test : public failure_test
	{
	public:

		template <typename... ArgTs>
		success_test(std::string&& input, ArgTs&&... args)
				: failure_test(std::move(input), std::forward<ArgTs>(args)..., minijava::token_type::eof) { }
	};

}  // namespace /* anonymous */


BOOST_AUTO_TEST_CASE(empty_input_starts_with_eof_and_stays_there)
{
	using namespace std::string_literals;
	const auto epsilon = ""s;
	auto pool = minijava::symbol_pool<>{};
	auto lex = minijava::make_lexer(std::begin(epsilon), std::end(epsilon), pool);
	for (auto i = 0; i < 100; ++i) {
		BOOST_REQUIRE_EQUAL(minijava::token_type::eof, lex.current_token().type());
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
			auto lex = minijava::make_lexer(std::begin(input), std::end(input), pool);
			BOOST_REQUIRE_EQUAL(sample, lex.current_token().type());
			lex.advance();
			BOOST_REQUIRE(lex.current_token_is_eof());
		} catch (const minijava::lexical_error& e) {
			BOOST_FAIL(
					"Unexpected lexical error for input '" << text << "'"
														   << " (" << text.length() << " characters): " << e.what()
			);
		}
	}
}


using tt = minijava::token_type;

static const success_test success_data[] = {
		// empty input
		{""},

		// identifiers
		{"alpha", "alpha"},
		{"alpha beta", "alpha", "beta"},
		{"alpha beta gamma delta", "alpha" "beta" "gamma" "delta"},
		{"alpha6_b3ta123_", "alpha6_b3ta123_"},

		// comments
		{"/**/"},
		{"/* * / */"},
		{"**/*= */*", tt::multiply, tt::multiply, tt::multiply},
		{std::string("false/*/***** const auto >= false static[] *\x7F/ ()\0\b\"\xFF ***/="), tt::kw_false, tt::assign},

		// integer literals
		{"0", std::uint32_t{0}},
		{"15", std::uint32_t{15}},
		{"0/**/509720", std::uint32_t{0}, std::uint32_t{509720}},
		{"-42 -0 --15", tt::minus, std::uint32_t{42}, tt::minus, std::uint32_t{0}, tt::decrement, std::uint32_t{15}},
		// TODO @Moritz Baumann: Add large integer test as soon as the token implementation is fixed
		//{"102984084080850832452705977991283408000810923847581234123412341234123412341242134", ?},

		// combinations of identifier, keyword, number and operator
		{
				"constauto static0void private_break _public do1 44true 0for while.if synchronized[] (abstract)",
				// checks that keywords are recognized correctly
				"constauto", "static0void", "private_break", "_public", "do1", std::uint32_t{44}, tt::kw_true,
				std::uint32_t{0}, tt::kw_for, tt::kw_while, tt::dot, tt::kw_if, tt::kw_synchronized, tt::left_bracket,
				tt::right_bracket, tt::left_paren, tt::kw_abstract, tt::right_paren
		},
		{
				"asdf00001 0myvar< test4>=",
				// checks that identifiers are recognized correctly in combination with numbers and operators
				"asdf00001", std::uint32_t{0}, "myvar", tt::less_than, "test4", tt::greater_equal
		},

		// operators and space types
		{
				">>>>===----=**&&&&&===>>>=&&=&&&>><<<",
				tt::unsigned_right_shift, tt::greater_equal, tt::equal, tt::decrement, tt::decrement, tt::assign,
				tt::multiply, tt::multiply, tt::logical_and, tt::logical_and, tt::bit_and_assign, tt::equal,
				tt::unsigned_right_shift_assign, tt::logical_and, tt::assign, tt::logical_and, tt::bit_and,
				tt::right_shift, tt::left_shift, tt::less_than
		},
		{">/*>>>=*/>>=", tt::greater_than, tt::left_shift_assign},
		{"*\t= =\r=\n=\t\r\n=", tt::multiply, tt::assign, tt::assign, tt::assign, tt::assign, tt::assign},
};

BOOST_DATA_TEST_CASE(input_lexed_correctly, success_data)
{
	auto s = sample.get();
	auto lex = minijava::make_lexer(std::begin(s.input), std::end(s.input), s.pool);
	BOOST_REQUIRE(std::equal(std::begin(s.expected), std::end(s.expected),
	              minijava::token_begin(lex), minijava::token_end(lex)));
}
