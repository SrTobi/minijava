#include "irg/global_firm_state.hpp"

#include <type_traits>

#define BOOST_TEST_MODULE  irg_global_firm_state
#include <boost/test/unit_test.hpp>

#include "firm.hpp"


BOOST_AUTO_TEST_CASE(constructible_and_destructible_but_not_copy_or_moveable)
{
	static_assert(std::is_default_constructible<minijava::global_firm_state>{}, "");
	static_assert(std::is_destructible<minijava::global_firm_state>{}, "");
	static_assert(!std::is_copy_assignable<minijava::global_firm_state>{}, "");
	static_assert(!std::is_copy_constructible<minijava::global_firm_state>{}, "");
	static_assert(!std::is_move_assignable<minijava::global_firm_state>{}, "");
	static_assert(!std::is_move_constructible<minijava::global_firm_state>{}, "");
}


BOOST_AUTO_TEST_CASE(what_little_we_can_test)
{
	{
		BOOST_REQUIRE_EQUAL(0, minijava::global_firm_state::program_count());
		minijava::global_firm_state state{};
		BOOST_REQUIRE_EQUAL(1, minijava::global_firm_state::program_count());
		const auto  ir0 = state.get_default_irp();
		BOOST_REQUIRE(ir0 == firm::get_irp());
		BOOST_REQUIRE(ir0 != nullptr);
		const auto ir1 = state.new_ir_prog("first");
		BOOST_REQUIRE(ir1 != ir0);
		BOOST_REQUIRE(ir1 == firm::get_irp());
		BOOST_REQUIRE_EQUAL(2, minijava::global_firm_state::program_count());
		const auto ir2 = state.new_ir_prog("second");
		BOOST_REQUIRE(ir1 != ir0);
		BOOST_REQUIRE(ir2 == firm::get_irp());
		BOOST_REQUIRE_EQUAL(3, minijava::global_firm_state::program_count());
		BOOST_REQUIRE(ir1 != ir2);
		state.free_ir_prog(ir1);
		BOOST_REQUIRE_EQUAL(2, minijava::global_firm_state::program_count());
		state.free_ir_prog(ir2);
		BOOST_REQUIRE_EQUAL(1, minijava::global_firm_state::program_count());
		BOOST_REQUIRE_THROW(minijava::global_firm_state{}, std::logic_error);
		const auto ir3 = state.new_ir_prog("third");
		BOOST_REQUIRE(ir3 != ir0);
		BOOST_REQUIRE(ir3 == firm::get_irp());
		BOOST_REQUIRE_EQUAL(2, minijava::global_firm_state::program_count());
		state.free_ir_prog(ir3);
		BOOST_REQUIRE_EQUAL(1, minijava::global_firm_state::program_count());
	}
	BOOST_REQUIRE_EQUAL(0, minijava::global_firm_state::program_count());
}
