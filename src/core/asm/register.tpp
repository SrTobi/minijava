#ifndef MINIJAVA_ASM_INCLUDED_FROM_REGISTER_HPP
#error "Never `#include <asm/register.tpp>` directly; `#include <asm/register.hpp>` instead."
#endif

#include <cassert>


namespace minijava
{

	namespace backend
	{

		constexpr virtual_register next_argument_register(const virtual_register reg)
		{
			assert(is_argument_register(reg));
			return static_cast<virtual_register>(static_cast<int>(reg) - 1);
		}

		constexpr virtual_register next_general_register(const virtual_register reg)
		{
			assert(is_general_register(reg));
			return static_cast<virtual_register>(static_cast<int>(reg) + 1);
		}

		constexpr bool is_argument_register(virtual_register reg)
		{
			return reg <= virtual_register::argument;
		}

		constexpr bool is_general_register(virtual_register reg)
		{
			return reg >= virtual_register::general;
		}

		constexpr int number(const virtual_register reg)
		{
			assert(is_argument_register(reg) || is_general_register(reg));
			const auto num = static_cast<int>(reg);
			if (is_general_register(reg)) {
				return num - static_cast<int>(virtual_register::general) + 1;
			} else {
				return -num;
			}
		}

		constexpr int number(const real_register reg)
		{
			const int num = static_cast<int>(reg);
			assert((num >= 0) && (num < real_register_count));
			return num;
		}

	}  // namespace backend

}  // namespace minijava
