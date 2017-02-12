#include "asm/register.hpp"

#include <map>
#include <string>
#include <vector>

#define BOOST_TEST_MODULE  asm_register
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_CASE(virtual_general_registers_constexpr)
{
	constexpr auto gp_reg = minijava::backend::virtual_register::general;
	static_assert(1 == number(gp_reg), "");
	static_assert(minijava::backend::is_general_register(gp_reg), "");
	static_assert(not minijava::backend::is_argument_register(gp_reg), "");
	constexpr auto gp_reg_2 = next_general_register(gp_reg);
	static_assert(2 == number(gp_reg_2), "");
	static_assert(minijava::backend::is_general_register(gp_reg_2), "");
	static_assert(not minijava::backend::is_argument_register(gp_reg_2), "");
	constexpr auto gp_reg_3 = next_general_register(gp_reg_2);
	static_assert(3 == number(gp_reg_3), "");
	static_assert(minijava::backend::is_general_register(gp_reg_3), "");
	static_assert(not minijava::backend::is_argument_register(gp_reg_3), "");
}

BOOST_AUTO_TEST_CASE(virtual_argument_registers_constexpr)
{
	constexpr auto arg_reg = minijava::backend::virtual_register::argument;
	static_assert(1 == number(arg_reg), "");
	static_assert(minijava::backend::is_argument_register(arg_reg), "");
	static_assert(not minijava::backend::is_general_register(arg_reg), "");
	constexpr auto arg_reg_2 = next_argument_register(arg_reg);
	static_assert(2 == number(arg_reg_2), "");
	static_assert(minijava::backend::is_argument_register(arg_reg_2), "");
	static_assert(not minijava::backend::is_general_register(arg_reg_2), "");
	constexpr auto arg_reg_3 = next_argument_register(arg_reg_2);
	static_assert(3 == number(arg_reg_3), "");
	static_assert(minijava::backend::is_argument_register(arg_reg_3), "");
	static_assert(not minijava::backend::is_general_register(arg_reg_3), "");
}

BOOST_AUTO_TEST_CASE(virtual_special_registers_constexpr)
{
	constexpr auto dummy_reg = minijava::backend::virtual_register::dummy;
	static_assert(not minijava::backend::is_argument_register(dummy_reg), "");
	static_assert(not minijava::backend::is_general_register(dummy_reg), "");
	constexpr auto result_reg = minijava::backend::virtual_register::result;
	static_assert(not minijava::backend::is_argument_register(result_reg), "");
	static_assert(not minijava::backend::is_general_register(result_reg), "");
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
				"r8b", "r9b", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b"
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
