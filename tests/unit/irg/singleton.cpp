#include "irg/singleton.hpp"

#include <type_traits>

#define BOOST_TEST_MODULE  firm_singleton
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_CASE(firm_state_move_only)
{
	static_assert(!std::is_copy_assignable<minijava::firm_global_state>{}, "");
	static_assert(!std::is_copy_constructible<minijava::firm_global_state>{}, "");
	static_assert(!std::is_move_assignable<minijava::firm_global_state>{}, "");
	static_assert(std::is_move_constructible<minijava::firm_global_state>{}, "");
}


BOOST_AUTO_TEST_CASE(attepting_to_create_another_firm_state_throws)
{
	minijava::firm_global_state state1st{};
	BOOST_REQUIRE_THROW(minijava::firm_global_state{}, std::logic_error);
	minijava::firm_global_state state3rd{std::move(state1st)};
	BOOST_REQUIRE_THROW(minijava::firm_global_state{}, std::logic_error);
}
