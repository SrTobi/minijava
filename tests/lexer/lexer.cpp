#include "lexer/lexer.hpp"

#include <algorithm>
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

	struct success_test_data
	{
		std::string input{};
		minijava::symbol_pool<> pool{};
		std::vector<minijava::token> expected{};
	};

	class success_test
	{
	public:

		template <typename... ArgTs>
		success_test(const char * input, ArgTs&&... args)
		{
			_input = input;
			_expected = {_make_expected_token(std::forward<ArgTs>(args))...,
						 minijava::token::create(minijava::token_type::eof)};
		}

		success_test_data get() const
		{
			auto sample = success_test_data{};
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
		std::string _input{};
		std::vector<minijava::token> _expected{};

		friend std::ostream& operator<<(std::ostream& os, const success_test& test)
		{
			return os << test._input;
		}

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


BOOST_AUTO_TEST_CASE(sequence_of_identifiers_lexed_correctly)
{
	using namespace std::string_literals;
	auto pool = minijava::symbol_pool<>{};
	const auto ids = "alpha beta gamma delta"s;
	const minijava::token expected[] = {
		minijava::token::create_identifier(pool.normalize("alpha")),
		minijava::token::create_identifier(pool.normalize("beta")),
		minijava::token::create_identifier(pool.normalize("gamma")),
		minijava::token::create_identifier(pool.normalize("delta")),
		minijava::token::create(minijava::token_type::eof),
	};
	auto lex = minijava::make_lexer(std::begin(ids), std::end(ids), pool);
	BOOST_REQUIRE(std::equal(std::begin(expected), std::end(expected),
							 minijava::token_begin(lex), minijava::token_end(lex)));
}


static const success_test success_data[] = {
	{""},
	{"alpha", "alpha"},
	{"alpha beta", "alpha", "beta"},
};

BOOST_DATA_TEST_CASE(success, success_data)
{
	auto s = sample.get();
	auto lex = minijava::make_lexer(std::begin(s.input), std::end(s.input), s.pool);
	BOOST_REQUIRE(std::equal(std::begin(s.expected), std::end(s.expected),
							 minijava::token_begin(lex), minijava::token_end(lex)));

}

// TODO @Moritz Baumann: Beef this file up with thorough unit tests.
