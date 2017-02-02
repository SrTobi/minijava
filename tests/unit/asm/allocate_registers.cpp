#include "asm/allocate_registers.hpp"

#define BOOST_TEST_MODULE  asm_allocate_registers
#include <boost/test/unit_test.hpp>

#include "exceptions.hpp"


BOOST_AUTO_TEST_CASE(allocate_registers_for_empty_function)
{
	using opc = minijava::backend::opcode;
	auto virtasm = minijava::backend::virtual_assembly{};
	auto realasm = minijava::backend::real_assembly{};
	virtasm.emplace_back();
	virtasm.back().label = "foo";
	virtasm.emplace_back(opc::op_ret);
	BOOST_REQUIRE_THROW(
		minijava::backend::allocate_registers(virtasm, realasm),
		minijava::not_implemented_error
	);
}
