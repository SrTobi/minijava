#include "firm/firm.hpp"

#define BOOST_TEST_MODULE  firm_firm
#include <boost/test/unit_test.hpp>

#include <type_traits>

static_assert(!std::is_copy_assignable<minijava::firm_ir>{}, "");
static_assert(!std::is_copy_constructible<minijava::firm_ir>{}, "");
static_assert(!std::is_move_assignable<minijava::firm_ir>{}, "");
static_assert(std::is_move_constructible<minijava::firm_ir>{}, "");

BOOST_AUTO_TEST_CASE(firm_state_handling)
{
	minijava::firm_ir my_ir{};
	BOOST_REQUIRE_THROW(minijava::firm_ir second_ir{}, std::logic_error);
	minijava::firm_ir new_ir{std::move(my_ir)};
	BOOST_REQUIRE_THROW(
			minijava::firm_ir second_new_ir{std::move(my_ir)},
			std::logic_error
	);
}
