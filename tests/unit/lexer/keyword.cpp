#include "lexer/keyword.hpp"

#include <string>

#define BOOST_TEST_MODULE  lexer_keyword
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "testaux/cx_string.hpp"


BOOST_AUTO_TEST_CASE(classify_word_after_name_is_identity_for_keywords)
{
	for (const auto tt : minijava::all_token_types()) {
		if (category(tt) == minijava::token_category::keyword) {
			BOOST_REQUIRE_EQUAL(tt, minijava::classify_word(name(tt)));
		}
	}
}


static const std::string non_keyword_data[] = {
	// The behavior for non-word inputs (in particular, numbers and strings
	// with garbage characters) is undefined so we don't test it.
	"a", "ab", "abc", "a1", "_", "__magic__", "X", "XYZ", "the1st", "X31"
};

BOOST_DATA_TEST_CASE(lookup_non_keyword, non_keyword_data)
{
	const auto tt = minijava::classify_word(sample);
	BOOST_REQUIRE_EQUAL(minijava::token_type::identifier, tt);
}


// The `constexpr` interface is not publicly exposed so no other code should
// rely on it but its value for testing is great enough to use it here.

namespace /* anonymous */
{

	constexpr bool constexpr_test_keywords() noexcept
	{
		for (std::size_t i = 0; i < minijava::total_token_type_count; ++i) {
			const auto tt = minijava::token_type_at_index(i);
			if (category(tt) == minijava::token_category::keyword) {
				const auto text = name(tt);
				const auto len = testaux::cx_strlen(text);
				const auto actual = minijava::detail::classify_word(text, len);
				if (actual != tt) {
					return false;
				}
			}
		}
		return true;
	}

	constexpr bool constexpr_test_identifiers() noexcept
	{
		const auto text = "just_some_23_characters";
		const auto len = testaux::cx_strlen(text);
		for (std::size_t l = 1; l <= len; ++l) {
			const auto actual = minijava::detail::classify_word(text, l);
			if (actual != minijava::token_type::identifier) {
				return false;
			}
		}
		return true;
	}

}  // namespace /* anonymous */

BOOST_AUTO_TEST_CASE(constexpr_checks)
{
	namespace mjdet = minijava::detail;
	using tt = minijava::token_type;
	static_assert(constexpr_test_keywords(), "test failed");
	static_assert(constexpr_test_identifiers(), "test failed");
	static_assert(mjdet::classify_word("fortran", 2) == tt::identifier, "");
	static_assert(mjdet::classify_word("fortran", 3) == tt::kw_for, "");
	static_assert(mjdet::classify_word("fortran", 4) == tt::identifier, "");
}
