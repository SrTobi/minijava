#ifndef MINIJAVA_ASM_INCLUDED_FROM_REGISTER_HPP
#error "Never `#include <asm/register.tpp>` directly; `#include <asm/register.hpp>` instead."
#endif

#include <cassert>


namespace minijava
{

	namespace backend
	{

		constexpr virtual_register make_virtual_register(const int num)
		{
			assert(num >= 0);
			return static_cast<virtual_register>(num);
		}

		constexpr virtual_register next(const virtual_register reg)
		{
			return make_virtual_register(number(reg) + 1);
		}

		constexpr int number(const virtual_register reg)
		{
			const auto num = static_cast<int>(reg);
			assert(num >= 0);
			return num;
		}

		constexpr int number(const real_register reg)
		{
			const int num = static_cast<int>(reg);
			assert((num >= 0) && (num < real_register_count));
			return num;
		}

	}  // namespace backend

}  // namespace minijava
