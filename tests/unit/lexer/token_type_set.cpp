#include "lexer/token_type_set.hpp"

#include <algorithm>
#include <utility>

#define BOOST_TEST_MODULE  lexer_token_type_set
#include <boost/test/unit_test.hpp>


using tt = minijava::token_type;

namespace /* anonymous */
{

	const auto tt_first = std::begin(minijava::all_token_types());
	const auto tt_last = std::end(minijava::all_token_types());

}  // namespace /* anonymous */


BOOST_AUTO_TEST_CASE(default_constructed_tts_is_empty)
{
	constexpr auto tts = minijava::token_type_set{};
	static_assert(tts.empty(), "");
}


BOOST_AUTO_TEST_CASE(default_constructed_tts_contains_nothing)
{
	const auto tts = minijava::token_type_set{};
	const auto pred = [&tts](auto tt){ return tts.contains(tt); };
	BOOST_REQUIRE(std::none_of(tt_first, tt_last, pred));
}


BOOST_AUTO_TEST_CASE(tts_contains_tt_it_was_constructed_with_and_nothing_else)
{
	constexpr auto tts = minijava::token_type_set{tt::kw_if, tt::kw_else};
	static_assert(!tts.empty(), "");
	static_assert(tts.contains(tt::kw_if), "");
	static_assert(tts.contains(tt::kw_else), "");
	static_assert(!tts.contains(tt::kw_assert), "");
	static_assert(!tts.contains(tt::assign), "");
	static_assert(!tts.contains(tt::eof), "");
}


BOOST_AUTO_TEST_CASE(duplicates_are_ignore_gracefully)
{
	constexpr auto tts = minijava::token_type_set{tt::kw_if, tt::kw_if, tt::kw_else, tt::kw_if};
	static_assert(!tts.empty(), "");
	static_assert(tts.contains(tt::kw_if), "");
	static_assert(tts.contains(tt::kw_else), "");
}


BOOST_AUTO_TEST_CASE(tts_contains_tt_after_it_was_added)
{
	auto tts = minijava::token_type_set{};
	tts.add(tt::plus);
	BOOST_REQUIRE(tts.contains(tt::plus));
	BOOST_REQUIRE(!tts.contains(tt::minus));
}


BOOST_AUTO_TEST_CASE(add_is_idempotent)
{
	auto tts = minijava::token_type_set{tt::plus};
	BOOST_REQUIRE(tts.contains(tt::plus));
	tts.add(tt::plus);
	BOOST_REQUIRE(tts.contains(tt::plus));
	tts.add(tt::plus);
	BOOST_REQUIRE(tts.contains(tt::plus));
}


BOOST_AUTO_TEST_CASE(tts_still_contains_previous_values_after_new_values_are_added)
{
	auto tts = minijava::token_type_set{tt::plus};
	tts.add(tt::minus);
	BOOST_REQUIRE(tts.contains(tt::plus));
	BOOST_REQUIRE(tts.contains(tt::minus));
}


BOOST_AUTO_TEST_CASE(tts_does_not_contain_tt_after_it_was_removed)
{
	auto tts = minijava::token_type_set{tt::semicolon};
	tts.remove(tt::semicolon);
	BOOST_REQUIRE(!tts.contains(tt::semicolon));
}


BOOST_AUTO_TEST_CASE(remove_is_idempotent)
{
	auto tts = minijava::token_type_set{};
	BOOST_REQUIRE(!tts.contains(tt::minus));
	tts.remove(tt::minus);
	BOOST_REQUIRE(!tts.contains(tt::minus));
	tts.remove(tt::minus);
	BOOST_REQUIRE(!tts.contains(tt::minus));
}


BOOST_AUTO_TEST_CASE(tts_still_contains_other_values_after_some_values_are_removed)
{
	auto tts = minijava::token_type_set{tt::identifier, tt::integer_literal, tt::eof};
	tts.remove(tt::identifier);
	BOOST_REQUIRE(!tts.contains(tt::identifier));
	BOOST_REQUIRE(tts.contains(tt::integer_literal) && tts.contains(tt::eof));
}


BOOST_AUTO_TEST_CASE(after_removing_everything_the_tts_is_empty)
{
	auto tts = minijava::token_type_set{tt::kw_assert, tt::kw_package};
	tts.remove(tt::kw_assert);
	tts.remove(tt::kw_package);
	BOOST_REQUIRE(tts.empty());
}


BOOST_AUTO_TEST_CASE(empty_tts_compare_equal)
{
	constexpr auto tts1 = minijava::token_type_set{};
	constexpr auto tts2 = minijava::token_type_set{};
	static_assert(minijava::token_type_set::equal(tts1, tts2), "");
	static_assert(tts1 == tts2, "");
	static_assert(!(tts1 != tts2), "");
}


BOOST_AUTO_TEST_CASE(tts_with_different_elements_compare_not_equal)
{
	constexpr auto tts1 = minijava::token_type_set{tt::kw_if, tt::kw_else};
	constexpr auto tts2 = minijava::token_type_set{tt::plus};
	static_assert(!minijava::token_type_set::equal(tts1, tts2), "");
	static_assert(tts1 != tts2, "");
	static_assert(!(tts1 == tts2), "");
}


BOOST_AUTO_TEST_CASE(tts_singletons_with_different_elements_compare_not_equal)
{
	constexpr auto tts1 = minijava::token_type_set{tt::kw_do};
	constexpr auto tts2 = minijava::token_type_set{tt::kw_while};
	static_assert(!minijava::token_type_set::equal(tts1, tts2), "");
	static_assert(tts1 != tts2, "");
	static_assert(!(tts1 == tts2), "");
}


BOOST_AUTO_TEST_CASE(union_assign_of_tts)
{
	auto tts = minijava::token_type_set{tt::plus, tt::minus, tt::eof};
	tts |= minijava::token_type_set{tt::multiply, tt::eof, tt::divide};
	const auto expected = minijava::token_type_set{
		tt::plus, tt::minus, tt::multiply, tt::divide, tt::eof
	};
	BOOST_REQUIRE(tts == expected);
}


BOOST_AUTO_TEST_CASE(free_union)
{
	constexpr auto tts1 = minijava::token_type_set{tt::dot, tt::eof, tt::semicolon};
	constexpr auto tts2 = minijava::token_type_set{tt::dot, tt::kw_for, tt::semicolon};
	constexpr auto expected = minijava::token_type_set{tt::dot, tt::eof, tt::kw_for, tt::semicolon};
	constexpr auto actual = tts1 | tts2;
	static_assert(expected == actual, "");
}


BOOST_AUTO_TEST_CASE(free_intersection)
{
	constexpr auto tts1 = minijava::token_type_set{tt::dot, tt::eof, tt::semicolon};
	constexpr auto tts2 = minijava::token_type_set{tt::dot, tt::kw_for, tt::semicolon};
	constexpr auto expected = minijava::token_type_set{tt::dot, tt::semicolon};
	constexpr auto actual = tts1 & tts2;
	static_assert(expected == actual, "");
}


BOOST_AUTO_TEST_CASE(intersection_assign_of_tts)
{
	auto tts = minijava::token_type_set{tt::plus, tt::minus, tt::eof};
	tts &= minijava::token_type_set{tt::multiply, tt::eof, tt::divide};
	const auto expected = minijava::token_type_set{tt::eof};
	BOOST_REQUIRE(tts == expected);
}
