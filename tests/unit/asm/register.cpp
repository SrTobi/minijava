#include "asm/register.hpp"

#define BOOST_TEST_MODULE  asm_register
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_CASE(make_virtual_register_and_number_round_trip)
{
	for (auto i = 0; i < 10000; ++i) {
		const auto reg = minijava::backend::make_virtual_register(i);
		const auto num = number(reg);
		BOOST_REQUIRE_EQUAL(i, num);
	}
}


BOOST_AUTO_TEST_CASE(make_virtual_register_is_constexpr_pure)
{
	constexpr auto first = minijava::backend::make_virtual_register(42);
	constexpr auto second = minijava::backend::make_virtual_register(42);
	static_assert(first == second, "");
}


BOOST_AUTO_TEST_CASE(next_virtual_register_is_constexpr_pure)
{
	constexpr auto reg = minijava::backend::make_virtual_register(42);
	static_assert(43 == number(next(reg)), "");
}


BOOST_AUTO_TEST_CASE(virtual_register_number_constexpr)
{
	constexpr auto reg = minijava::backend::make_virtual_register(42);
	static_assert(42 == number(reg), "");
}


BOOST_AUTO_TEST_CASE(real_register_number_constexpr)
{
	constexpr auto reg = minijava::backend::real_register::r10;
	static_assert(10 == number(reg), "");
}

BOOST_AUTO_TEST_CASE(real_registers_correct_size)
{
	auto registers = minijava::backend::real_registers();
	using array_type = decltype(registers);
	constexpr auto count = std::tuple_size<array_type>::value;
	static_assert(minijava::backend::real_register_count == count, "");
}


BOOST_AUTO_TEST_CASE(real_registers_correct_order)
{
	const auto registers = minijava::backend::real_registers();
	for (auto i = 0; i < minijava::backend::real_register_count; ++i) {
		const auto idx = static_cast<std::size_t>(i);
		BOOST_REQUIRE_EQUAL(i, number(registers.at(idx)));
	}
}
