#include "asm/opcode.hpp"

#define BOOST_TEST_MODULE  asm_opcode
#include <boost/test/unit_test.hpp>

#include "testaux/cx_string.hpp"


BOOST_AUTO_TEST_CASE(default_op_is_none)
{
	constexpr auto op = minijava::backend::opcode{};
	static_assert(op == minijava::backend::opcode::none, "");
}


BOOST_AUTO_TEST_CASE(mnemotic_constexpr_and_correct)
{
	constexpr auto op = minijava::backend::opcode::op_add;
	constexpr auto mn = mnemotic(op);
	static_assert(testaux::cx_strcmp(mn, "add") == 0, "");
}
