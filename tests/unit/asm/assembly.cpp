#include "asm/assembly.hpp"

#define BOOST_TEST_MODULE  asm_assembly
#include <boost/test/unit_test.hpp>

#include <boost/mpl/list.hpp>

using both_register_types = boost::mpl::list<
	minijava::backend::virtual_register,
	minijava::backend::real_register
>;


BOOST_AUTO_TEST_CASE_TEMPLATE(assembly_ctor, RegT, both_register_types)
{
	using assembly_type = minijava::backend::assembly<RegT>;
	auto as = assembly_type{"foo"};
	BOOST_REQUIRE_EQUAL("foo", as.ldname);
	BOOST_REQUIRE(as.blocks.empty());
}
