#include "asm/allocator.hpp"

#define BOOST_TEST_MODULE  asm_allocator
#include <boost/test/unit_test.hpp>

#include "exceptions.hpp"


BOOST_AUTO_TEST_CASE(allocate_registers_for_empty_function)
{
	const auto virtasm = minijava::backend::virtual_assembly{"foo"};
	const auto realasm = minijava::backend::allocate_registers(virtasm);
	BOOST_REQUIRE(!realasm.blocks.front().code.empty());
}
