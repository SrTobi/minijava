#include "asm/basic_block.hpp"

#define BOOST_TEST_MODULE  asm_basic_block
#include <boost/test/unit_test.hpp>

#include <boost/mpl/list.hpp>

using both_register_types = boost::mpl::list<
	minijava::backend::virtual_register,
	minijava::backend::real_register
>;


BOOST_AUTO_TEST_CASE_TEMPLATE(basic_block_construction, RegT, both_register_types)
{
	using basic_block_type = minijava::backend::basic_block<RegT>;
	auto bb = basic_block_type{"func"};
	BOOST_REQUIRE_EQUAL("func", bb.label);
	BOOST_REQUIRE(bb.code.empty());
}
