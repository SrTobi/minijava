#include "lexer/character.hpp"

#include <algorithm>
#include <cassert>
#include <climits>
#include <string>

#define BOOST_TEST_MODULE  lexer_character
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

using namespace std::string_literals;


namespace /* anonymous */
{

	// `return`s a string that contains all `char` values in sorted order.
	std::string all_bytes()
	{
		auto s = std::string{};
		for (int c = CHAR_MIN; c <= CHAR_MAX; ++c) {
			s.push_back(static_cast<char>(c));
		}
		return s;
	}

	// `return`s a string that contains all `char`s except for those present in
	// the string `without`.
	std::string char_complement(const std::string& without)
	{
		static const auto universe = all_bytes();
		auto sorted = without;
		auto difference = std::string{};
		std::sort(std::begin(sorted), std::end(sorted));
		std::set_difference(std::begin(universe), std::end(universe),
		                    std::begin(sorted), std::end(sorted),
		                    std::back_inserter(difference));
		assert(without.size() + difference.size() == universe.size());
		return difference;
	}

}


static const auto is_space_positive_data = " \n\r\t"s;

BOOST_DATA_TEST_CASE(is_space_positive, is_space_positive_data)
{
	const int c = static_cast<unsigned char>(sample);
	BOOST_REQUIRE(minijava::is_space(c));
}


static const auto is_space_negative_data = char_complement(is_space_positive_data);

BOOST_DATA_TEST_CASE(is_space_negative, is_space_negative_data)
{
	const int c = static_cast<unsigned char>(sample);
	BOOST_REQUIRE(!minijava::is_space(c));
}


static const auto is_digit_positive_data = "0123456789"s;

BOOST_DATA_TEST_CASE(is_digit_positive, is_digit_positive_data)
{
	const int c = static_cast<unsigned char>(sample);
	BOOST_REQUIRE(minijava::is_digit(c));
}


static const auto is_digit_negative_data = char_complement(is_digit_positive_data);

BOOST_DATA_TEST_CASE(is_digit_negative, is_digit_negative_data)
{
	const int c = static_cast<unsigned char>(sample);
	BOOST_REQUIRE(!minijava::is_digit(c));
}


static const auto is_word_head_positive_data = "_"s
	+ "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	+ "abcdefghijklmnopqrstuvwxyz";

BOOST_DATA_TEST_CASE(is_word_head_positive, is_word_head_positive_data)
{
	const int c = static_cast<unsigned char>(sample);
	BOOST_REQUIRE(minijava::is_word_head(c));
}


static const auto is_word_head_negative_data = char_complement(is_word_head_positive_data);

BOOST_DATA_TEST_CASE(is_word_head_negative, is_word_head_negative_data)
{
	const int c = static_cast<unsigned char>(sample);
	BOOST_REQUIRE(!minijava::is_word_head(c));
}


static const auto is_word_tail_positive_data = "_"s
	+ "0123456789"
	+ "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	+ "abcdefghijklmnopqrstuvwxyz";

BOOST_DATA_TEST_CASE(is_word_tail_positive, is_word_tail_positive_data)
{
	const int c = static_cast<unsigned char>(sample);
	BOOST_REQUIRE(minijava::is_word_tail(c));
}


static const auto is_word_tail_negative_data = char_complement(is_word_tail_positive_data);

BOOST_DATA_TEST_CASE(is_word_tail_negative, is_word_tail_negative_data)
{
	const int c = static_cast<unsigned char>(sample);
	BOOST_REQUIRE(!minijava::is_word_tail(c));
}
