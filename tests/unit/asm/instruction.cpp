#include "asm/instruction.hpp"

#include <type_traits>

#define BOOST_TEST_MODULE  asm_instruction
#include <boost/test/unit_test.hpp>


namespace /* anonymous */
{

	enum class mock_register {a, b, c};

}  // namespace /* anonymous */


BOOST_AUTO_TEST_CASE(address_type_traits)
{
	using address_type = minijava::backend::address<mock_register>;
	static_assert(std::is_default_constructible<address_type>{}, "");
}


BOOST_AUTO_TEST_CASE(address_initialization)
{
	using address_type = minijava::backend::address<mock_register>;
	auto addr = address_type{};
	addr = address_type{boost::none, mock_register::a, boost::none, boost::none};
	addr = address_type{-42, mock_register::a, boost::none, boost::none};
	addr = address_type{boost::none, mock_register::a, mock_register::b, boost::none};
	addr = address_type{0, mock_register::c, mock_register::c, 2};
	(void) addr;
}

BOOST_AUTO_TEST_CASE(operand_none)
{
	using operand_type = minijava::backend::operand<mock_register>;
	const auto operand = operand_type{};
	BOOST_REQUIRE(empty(operand));
	BOOST_REQUIRE(nullptr == get_immediate(operand));
	BOOST_REQUIRE(nullptr == get_register(operand));
	BOOST_REQUIRE(nullptr == get_address(operand));
	BOOST_REQUIRE(nullptr == get_name(operand));
}

BOOST_AUTO_TEST_CASE(operand_immediate)
{
	using operand_type = minijava::backend::operand<mock_register>;
	const auto operand = operand_type{42};
	BOOST_REQUIRE(!empty(operand));
	BOOST_REQUIRE(nullptr != get_immediate(operand));
	BOOST_REQUIRE_EQUAL(42, *get_immediate(operand));
	BOOST_REQUIRE(nullptr == get_register(operand));
	BOOST_REQUIRE(nullptr == get_address(operand));
	BOOST_REQUIRE(nullptr == get_name(operand));
}

BOOST_AUTO_TEST_CASE(operand_register)
{
	using operand_type = minijava::backend::operand<mock_register>;
	const auto operand = operand_type{mock_register::b};
	BOOST_REQUIRE(!empty(operand));
	BOOST_REQUIRE(nullptr == get_immediate(operand));
	BOOST_REQUIRE(nullptr != get_register(operand));
	BOOST_REQUIRE(mock_register::b == *get_register(operand));
	BOOST_REQUIRE(nullptr == get_address(operand));
	BOOST_REQUIRE(nullptr == get_name(operand));
}

BOOST_AUTO_TEST_CASE(operand_address)
{
	using operand_type = minijava::backend::operand<mock_register>;
	using address_type = minijava::backend::address<mock_register>;
	const auto operand = operand_type{address_type{}};
	BOOST_REQUIRE(!empty(operand));
	BOOST_REQUIRE(nullptr == get_immediate(operand));
	BOOST_REQUIRE(nullptr == get_register(operand));
	BOOST_REQUIRE(nullptr != get_address(operand));
	BOOST_REQUIRE(nullptr == get_name(operand));
}

BOOST_AUTO_TEST_CASE(operand_name)
{
	using operand_type = minijava::backend::operand<mock_register>;
	const auto label = "foo";
	const auto operand = operand_type{label};
	BOOST_REQUIRE(!empty(operand));
	BOOST_REQUIRE(nullptr == get_immediate(operand));
	BOOST_REQUIRE(nullptr == get_register(operand));
	BOOST_REQUIRE(nullptr == get_address(operand));
	BOOST_REQUIRE(nullptr != get_name(operand));
	BOOST_REQUIRE_EQUAL(label, *get_name(operand));
}
