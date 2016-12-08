#include "lexer/token_type.hpp"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <limits>
#include <numeric>
#include <set>
#include <sstream>
#include <string>
#include <tuple>

#define BOOST_TEST_MODULE  lexer_token_type
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "testaux/cx_string.hpp"

using namespace std::string_literals;


static const auto undeclared_token_type_1st = static_cast<minijava::token_type>(0x0000);
static const auto undeclared_token_type_2nd = static_cast<minijava::token_type>(0x2000);
static const auto undeclared_token_type_3rd = static_cast<minijava::token_type>(0x2002);
static const auto undeclared_token_type_4th = static_cast<minijava::token_type>(0x4001);
static const auto undeclared_token_type_5th = static_cast<minijava::token_type>(0x5000);
static const auto undeclared_token_type_6th = static_cast<minijava::token_type>(0x6011);
static const auto undeclared_token_type_7th = static_cast<minijava::token_type>(0xffff);


BOOST_AUTO_TEST_CASE(there_is_no_default_token_type)
{
	const auto tt = minijava::token_type{};
	const auto first = std::begin(minijava::all_token_types());
	const auto last = std::end(minijava::all_token_types());
	BOOST_REQUIRE(std::find(first, last, tt) == last);
}


BOOST_AUTO_TEST_CASE(there_is_no_default_token_category)
{
	const auto cat = minijava::token_category{};
	BOOST_REQUIRE_NE(cat, minijava::token_category::identifier);
	BOOST_REQUIRE_NE(cat, minijava::token_category::literal);
	BOOST_REQUIRE_NE(cat, minijava::token_category::keyword);
	BOOST_REQUIRE_NE(cat, minijava::token_category::punctuation);
	BOOST_REQUIRE_NE(cat, minijava::token_category::synthetic);
}


BOOST_AUTO_TEST_CASE(name_returns_correct_string_for_declared_token_type)
{
	constexpr auto tt = minijava::token_type::kw_else;
	constexpr auto text = name(tt);
	static_assert(testaux::cx_strcmp("else", text) == 0, "");
}


BOOST_AUTO_TEST_CASE(name_returns_correct_string_for_declared_token_category)
{
	using tc = minijava::token_category;
	static_assert(testaux::cx_strcmp("identifier",  name(tc::identifier))  == 0, "");
	static_assert(testaux::cx_strcmp("literal",     name(tc::literal))     == 0, "");
	static_assert(testaux::cx_strcmp("keyword",     name(tc::keyword))     == 0, "");
	static_assert(testaux::cx_strcmp("punctuation", name(tc::punctuation)) == 0, "");
	static_assert(testaux::cx_strcmp("synthetic",   name(tc::synthetic))   == 0, "");
}


BOOST_AUTO_TEST_CASE(name_returns_nullptr_for_default_constructed_token_type)
{
	constexpr auto tt = minijava::token_type{};
	constexpr auto text = name(tt);
	static_assert(text == nullptr, "");
}


BOOST_AUTO_TEST_CASE(name_returns_nullptr_for_undeclared_token_type)
{
	static_assert(nullptr == name(undeclared_token_type_1st), "");
	static_assert(nullptr == name(undeclared_token_type_2nd), "");
	static_assert(nullptr == name(undeclared_token_type_3rd), "");
	static_assert(nullptr == name(undeclared_token_type_4th), "");
	static_assert(nullptr == name(undeclared_token_type_5th), "");
	static_assert(nullptr == name(undeclared_token_type_6th), "");
	static_assert(nullptr == name(undeclared_token_type_7th), "");
}


BOOST_AUTO_TEST_CASE(token_type_names_are_unique)
{
	auto names = std::set<std::string>{};
	const auto first = std::begin(minijava::all_token_types());
	const auto last = std::end(minijava::all_token_types());
	const auto lambda = [](auto tt){ return name(tt); };
	std::transform(first, last, std::inserter(names, names.cend()), lambda);
	const auto expected = static_cast<std::size_t>(std::distance(first, last));
	const auto actual = names.size();
	BOOST_REQUIRE_EQUAL(expected, actual);
}


BOOST_AUTO_TEST_CASE(name_returns_nullptr_for_default_constructed_token_category)
{
	constexpr auto cat = minijava::token_category{};
	constexpr auto text = name(cat);
	static_assert(text == nullptr, "");
}


BOOST_AUTO_TEST_CASE(streaming_declared_token_type_inserts_the_correct_string)
{
	const auto tt = minijava::token_type::kw_else;
	auto oss = std::ostringstream{};
	oss << tt;
	BOOST_REQUIRE(oss);
	BOOST_REQUIRE_EQUAL("else"s, oss.str());
}


BOOST_AUTO_TEST_CASE(streaming_declared_token_category_inserts_the_correct_string)
{
	const auto cat = minijava::token_category::literal;
	auto oss = std::ostringstream{};
	oss << cat;
	BOOST_REQUIRE(oss);
	BOOST_REQUIRE_EQUAL("literal"s, oss.str());
}


BOOST_AUTO_TEST_CASE(streaming_undeclared_token_type_inserts_some_string)
{
	const auto tt = minijava::token_type{};
	auto oss = std::ostringstream{};
	oss << tt;
	BOOST_REQUIRE(oss);
	BOOST_REQUIRE_EQUAL("token_type(0)"s, oss.str());
}


BOOST_AUTO_TEST_CASE(streaming_undeclared_token_category_inserts_some_string)
{
	const auto cat = minijava::token_category{};
	auto oss = std::ostringstream{};
	oss << cat;
	BOOST_REQUIRE(oss);
	BOOST_REQUIRE_EQUAL("token_category(0)"s, oss.str());
}


BOOST_AUTO_TEST_CASE(total_token_type_count_is_correct)
{
	BOOST_REQUIRE_EQUAL(
		minijava::total_token_type_count,
		minijava::all_token_types().size()
	);
}


BOOST_AUTO_TEST_CASE(token_types_are_unique)
{
	const auto first = std::begin(minijava::all_token_types());
	const auto last = std::end(minijava::all_token_types());
	const auto tts = std::set<minijava::token_type>{first, last};
	BOOST_REQUIRE_EQUAL(last - first, tts.size());
}


BOOST_AUTO_TEST_CASE(token_types_are_monotonic)
{
	const auto first = std::begin(minijava::all_token_types());
	const auto last = std::end(minijava::all_token_types());
	BOOST_REQUIRE(std::is_sorted(first, last));
}


BOOST_AUTO_TEST_CASE(token_types_are_sorted_by_category)
{
	auto categories = std::vector<minijava::token_category>(minijava::total_token_type_count);
	std::transform(
		std::begin(minijava::all_token_types()),
		std::end(minijava::all_token_types()),
		std::begin(categories),
		[](const auto tt){ return category(tt); }
	);
	BOOST_REQUIRE(std::is_sorted(std::begin(categories), std::end(categories)));
}


BOOST_AUTO_TEST_CASE(token_type_minus_token_category_is_index)
{
	using raw_type = std::underlying_type_t<minijava::token_type>;
	using raw_cat = std::underlying_type_t<minijava::token_category>;
	auto expected = std::vector<raw_type>(minijava::total_token_type_count);
	std::iota(std::begin(expected), std::end(expected), raw_type{});
	auto actual = std::vector<raw_type>(minijava::total_token_type_count);
	std::transform(
		std::begin(minijava::all_token_types()),
		std::end(minijava::all_token_types()),
		std::begin(actual),
		[](const auto tt){
			const auto cat = category(tt);
			const auto rt = static_cast<raw_type>(tt);
			const auto rc = static_cast<raw_cat>(cat);
			return raw_type((0U + rt) - (0U + rc));
		});
	BOOST_REQUIRE(std::equal(std::begin(expected), std::end(expected),
	                         std::begin(actual), std::end(actual)));
}


// This list contains the first and the last member of each category and two in
// between (if the category has that many members).  We could add more but this
// might get pointless at some point.
static const std::tuple<minijava::token_type, minijava::token_category, std::string> type_category_name_data[] = {
	{minijava::token_type::identifier,      minijava::token_category::identifier,  "identifier"s},
	{minijava::token_type::integer_literal, minijava::token_category::literal,     "integer literal"s},
	{minijava::token_type::kw_abstract,     minijava::token_category::keyword,     "abstract"s},
	{minijava::token_type::kw_goto,         minijava::token_category::keyword,     "goto"s},
	{minijava::token_type::kw_protected,    minijava::token_category::keyword,     "protected"s},
	{minijava::token_type::kw_while,        minijava::token_category::keyword,     "while"s},
	{minijava::token_type::not_equal,       minijava::token_category::punctuation, "!="s},
	{minijava::token_type::plus,            minijava::token_category::punctuation, "+"s},
	{minijava::token_type::dot,             minijava::token_category::punctuation, "."s},
	{minijava::token_type::bit_or,          minijava::token_category::punctuation, "|"s},
	{minijava::token_type::eof,             minijava::token_category::synthetic,   "EOF"s},
};

// We're not using a `BOOST_DATA_TEST_CASE` here because it doesn't like
// `std::tuple`.  Presumably because it can't stream it but the error message
// is too long for me to be sure...
BOOST_AUTO_TEST_CASE(type_category_name)
{
	for (const auto& sample : type_category_name_data) {
		const auto tt = std::get<0>(sample);
		const auto expected_category = std::get<1>(sample);
		const auto expected_name = std::get<2>(sample);
		const auto actual_category = category(tt);
		const auto actual_name = name(tt);
		BOOST_REQUIRE_EQUAL(expected_category, actual_category);
		BOOST_REQUIRE_EQUAL(expected_name, actual_name);
	}
}


BOOST_AUTO_TEST_CASE(category_is_well_behaved_for_undeclared_enumerators)
{
	// Since the result is unspecified in this case, we cannot test a whole
	// lot.  But at least, the compiler must not crash.  We assign the results
	// to a `volatile` sink so the compiler cannot optimize their computation
	// away.
	volatile minijava::token_category sink;
	constexpr auto cat_1st = category(undeclared_token_type_1st);
	sink = cat_1st;
	constexpr auto cat_2nd = category(undeclared_token_type_2nd);
	sink = cat_2nd;
	constexpr auto cat_3rd = category(undeclared_token_type_3rd);
	sink = cat_3rd;
	constexpr auto cat_4th = category(undeclared_token_type_4th);
	sink = cat_4th;
	constexpr auto cat_5th = category(undeclared_token_type_5th);
	sink = cat_5th;
	constexpr auto cat_6th = category(undeclared_token_type_6th);
	sink = cat_6th;
	constexpr auto cat_7th = category(undeclared_token_type_7th);
	sink = cat_7th;
	// And since GCC still warns about `sink` beeing unused even though it is
	// declared as `volatile`, we also have to do this...
	(void) sink;
}


BOOST_AUTO_TEST_CASE(index_function_returns_index_in_array)
{
	const auto all = minijava::all_token_types();
	for (const auto tt : all) {
		const auto idx = index(tt);
		BOOST_REQUIRE_EQUAL(tt, all.at(idx));
	}
}


BOOST_AUTO_TEST_CASE(index_function_returns_tttc_for_default_constructed_token_type)
{
	constexpr auto tt = minijava::token_type{};
	constexpr auto idx = index(tt);
	static_assert(idx == minijava::total_token_type_count, "");
}


BOOST_AUTO_TEST_CASE(index_function_returns_tttc_for_undeclared_enumerator)
{
	static_assert(minijava::total_token_type_count == index(undeclared_token_type_1st), "");
	static_assert(minijava::total_token_type_count == index(undeclared_token_type_2nd), "");
	static_assert(minijava::total_token_type_count == index(undeclared_token_type_3rd), "");
	static_assert(minijava::total_token_type_count == index(undeclared_token_type_4th), "");
	static_assert(minijava::total_token_type_count == index(undeclared_token_type_5th), "");
	static_assert(minijava::total_token_type_count == index(undeclared_token_type_6th), "");
	static_assert(minijava::total_token_type_count == index(undeclared_token_type_7th), "");
}


BOOST_AUTO_TEST_CASE(token_type_at_index_same_as_array_lookup)
{
	for (std::size_t i = 0; i < minijava::total_token_type_count; ++i) {
		const auto expected = minijava::all_token_types().at(i);
		const auto actual = minijava::token_type_at_index(i);
		BOOST_REQUIRE_EQUAL(expected, actual);
	}
}


static const std::size_t invalid_indices[] = {
	minijava::total_token_type_count,
	minijava::total_token_type_count + 1,
	minijava::total_token_type_count + 2,
	minijava::total_token_type_count + 3,
	minijava::total_token_type_count + 100,
	minijava::total_token_type_count + 1000000,
	std::numeric_limits<std::size_t>::max() - 1,
	std::numeric_limits<std::size_t>::max(),
};

BOOST_DATA_TEST_CASE(token_type_at_index_returns_zero_for_out_of_range, invalid_indices)
{
	const auto expected = minijava::token_type{};
	const auto actual = minijava::token_type_at_index(sample);
	BOOST_REQUIRE_EQUAL(expected, actual);
}


BOOST_AUTO_TEST_CASE(index_functions_are_constexpr_1st)
{
	constexpr auto expected = minijava::token_type::unsigned_right_shift;
	constexpr auto actual = minijava::token_type_at_index(index(expected));
	static_assert(actual == expected, "");
}


BOOST_AUTO_TEST_CASE(index_functions_are_constexpr_2nd)
{
	constexpr auto expected = std::size_t{42};
	constexpr auto actual = index(minijava::token_type_at_index(expected));
	static_assert(actual == expected, "");
}
