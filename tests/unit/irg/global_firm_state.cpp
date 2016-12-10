#include "irg/global_firm_state.hpp"

#include <type_traits>

#define BOOST_TEST_MODULE  firm_global_firm_state
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_CASE(firm_state_move_only)
{
	static_assert(!std::is_copy_assignable<minijava::global_firm_state>{}, "");
	static_assert(!std::is_copy_constructible<minijava::global_firm_state>{}, "");
	static_assert(std::is_move_assignable<minijava::global_firm_state>{}, "");
	static_assert(std::is_move_constructible<minijava::global_firm_state>{}, "");
}


BOOST_AUTO_TEST_CASE(attempting_to_create_another_firm_state_throws)
{
	minijava::global_firm_state state1st = minijava::initialize_firm();
	BOOST_REQUIRE_THROW(minijava::initialize_firm(), std::logic_error);
	minijava::global_firm_state state3rd{std::move(state1st)};
	BOOST_REQUIRE_THROW(minijava::global_firm_state state4th{std::move(state1st)}, std::logic_error);
	BOOST_REQUIRE_THROW(minijava::initialize_firm(), std::logic_error);
	minijava::global_firm_state state5th{std::move(state3rd)};
}
