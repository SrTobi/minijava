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
			assert(reg <= virtual_register::argument);
			return static_cast<virtual_register>(static_cast<int>(reg) - 1);
		}

		constexpr virtual_register next_general_register(const virtual_register reg)
		{
			assert(reg >= virtual_register::general);
			return static_cast<virtual_register>(static_cast<int>(reg) + 1);
		}

		constexpr int number(const virtual_register reg)
		{
			assert(reg >= virtual_register::general || reg <= virtual_register::argument);
			const auto num = static_cast<int>(reg);
			if (reg >= virtual_register::general) {
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
