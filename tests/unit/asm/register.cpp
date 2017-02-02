#include "asm/register.hpp"

#include <map>
#include <string>
#include <vector>

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


BOOST_AUTO_TEST_CASE(real_registers_name)
{
	const auto registers = std::map<minijava::backend::bit_width, std::vector<std::string>>
	{
		{
			minijava::backend::bit_width::lxiv,
			{
				"rax", "rbx", "rcx", "rdx", "rbp", "rsp", "rsi", "rdi",
				"r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
			}
		},
		{
			minijava::backend::bit_width::xxxii,
			{
				"eax", "ebx", "ecx", "edx", "ebp", "esp", "esi", "edi",
				"r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d"
			}
		},
		{
			minijava::backend::bit_width::xvi,
			{
				"ax", "bx", "cx", "dx", "bp", "sp", "si", "di",
				"r8w", "r9w", "r10w", "r11w", "r12w", "r13w", "r14w", "r15w"
			}
		},
		{
			minijava::backend::bit_width::viii,
			{
				"al", "bl", "cl", "dl", "bpl", "spl", "sil", "dil",
				"r8l", "r9l", "r10l", "r11l", "r12l", "r13l", "r14l", "r15l"
			}
		},
	};
	for (const auto& kv : registers) {
		const auto w = kv.first;
		for (std::size_t i = 0; i < minijava::backend::real_register_count; ++i) {
			const auto reg = minijava::backend::real_registers().at(i);
			const auto expected = kv.second.at(i);
			const auto actual = name(reg, w);
			BOOST_REQUIRE_EQUAL(expected, actual);
		}
	}
}
