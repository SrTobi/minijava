#include "asm/allocator.hpp"

#define BOOST_TEST_MODULE  asm_allocator
#include <boost/test/unit_test.hpp>

#include "exceptions.hpp"


BOOST_AUTO_TEST_CASE(allocate_registers_for_empty_function)
{
	using opc = minijava::backend::opcode;
	using bw = minijava::backend::bit_width;
	using vreg = minijava::backend::virtual_register;
	auto virtasm = minijava::backend::virtual_assembly{};
	auto realasm = minijava::backend::real_assembly{};
	virtasm.emplace_back();
	virtasm.back().label = "foo";
	virtasm.emplace_back(opc::op_push, bw::lxiv, vreg::base_pointer);
	virtasm.emplace_back(opc::op_mov, bw::lxiv, vreg::stack_pointer, vreg::base_pointer);
	virtasm.emplace_back(opc::op_sub, bw::lxiv, 0, vreg::stack_pointer);
	virtasm.emplace_back(opc::op_mov, bw::lxiv, vreg::base_pointer, vreg::stack_pointer);
	virtasm.emplace_back(opc::op_pop, bw::lxiv, vreg::base_pointer);
	virtasm.emplace_back(opc::op_ret);
	BOOST_REQUIRE_THROW(
		minijava::backend::allocate_registers(virtasm, realasm),
		minijava::not_implemented_error
	);
}
