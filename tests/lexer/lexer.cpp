#include "lexer/lexer.hpp"

#include <algorithm>
#include <forward_list>
#include <string>

#define BOOST_TEST_MODULE  lexer_lexer
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "lexer/token.hpp"
#include "lexer/token_iterator.hpp"
#include "symbol_pool.hpp"


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


static const auto single_token_data = minijava::all_token_types();

BOOST_DATA_TEST_CASE(single_tokens_are_lexed_correctly, single_token_data)
{
	const auto punct = minijava::token_category::punctuation;
	/* const auto kw = minijava::token_category::keyword; */
	const auto cat = category(sample);
	if ((cat == punct) /* || (cat == kw) */) {
		const auto text = std::string{name(sample)};
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


// TODO @Moritz Baumann: Beef this file up with thorough unit tests.
