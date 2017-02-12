#include "asm/macros.hpp"

#define BOOST_TEST_MODULE  asm_macros
#include <boost/test/unit_test.hpp>


// scratch registers 14 and 15

BOOST_AUTO_TEST_CASE(expand_div)
{
	using op = minijava::backend::opcode;
	using rr = minijava::backend::real_register;
	using bw = minijava::backend::bit_width;
	auto assembly = minijava::backend::real_assembly{"func"};
	assembly.blocks.emplace_back(".L0");
	assembly.blocks.back().code.emplace_back(op::mac_div, bw::lxiv, rr::a, rr::b);
	minijava::backend::expand_macros(assembly);
}
