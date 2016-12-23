/**
 * @file register.hpp
 *
 * @brief
 *     Virtual and real x64 registers.
 *
 */

#pragma once

#include <array>


namespace minijava
{

	namespace backend
	{

		/**
		 * @brief
		 *     Virtual (unlimited) registers.
		 *
		 * There are declared enumerators for well-known registers that must be
		 * used for their respective purpose.  Use the `make_virtual_register`
		 * function to obtain general-purpose registers without preset meaning.
		 *
		 */
		enum class virtual_register : int
		{
			dummy,          ///< garbage register (value is never read)
			stack_pointer,  ///< stack pointer
			base_pointer,   ///< base pointer
			result,         ///< function return value
			general,        ///< first general-purpose register
		};

		/**
		 * @brief
		 *     `return`s the virtual register with the given number.
		 *
		 * If the register number is negative, the behavior is undefined.
		 *
		 * @param num
		 *     non-negative number of the virtual register
		 *
		 * @returns
		 *     the virtual register
		 *
		 */
		constexpr virtual_register make_virtual_register(int num);

		/**
		 * @brief
		 *     `return`s the next virtual register after `reg`.
		 *
		 * This is a convenience function for
		 * `make_virtual_register(number(reg) + 1)`.  The behavior is undefined
		 * unless said call is well-defined.
		 *
		 * @param reg
		 *     register before the desired register
		 *
		 * @returns
		 *     the virtual register
		 *
		 */
		constexpr virtual_register next(virtual_register reg);

		/**
		 * @brief
		 *     `return`s the number of a virtual register.
		 *
		 * If the number of `reg` is negative, the behavior is undefined.
		 *
		 * @param reg
		 *     virtual register
		 *
		 * @returns
		 *     number of the register
		 *
		 */
		constexpr int number(virtual_register reg);

		/** @brief Number of real x64 integer registers. */
		constexpr int real_register_count = 16;

		/**
		 * @brief
		 *     Real x64 general-purpose registers.
		 *
		 */
		enum class real_register : signed char
		{
			a   =  0,  ///< register A
			b   =  1,  ///< register B
			c   =  2,  ///< register C
			d   =  3,  ///< register D
			bp  =  4,  ///< BP register (base pointer)
			sp  =  5,  ///< SP register (stack pointer)
			si  =  6,  ///< SI register
			di  =  7,  ///< DI register
			r8  =  8,  ///< register 8
			r9  =  9,  ///< register 9
			r10 = 10,  ///< register 10
			r11 = 11,  ///< register 11
			r12 = 12,  ///< register 12
			r13 = 13,  ///< register 13
			r14 = 14,  ///< register 14
			r15 = 15   ///< register 15
		};

		/**
		 * @brief
		 *     `return`s the number of a real x64 general-purpose register.
		 *
		 * If `reg` is not a declared enumerator, the behavior is undefined.
		 *
		 * @param reg
		 *     general-purpose register
		 *
		 * @returns
		 *     number of the register
		 *
		 */
		constexpr int number(real_register reg);

		/**
		 * @brief
		 *     `return`s an array containing all available x64 general-purpose
		 *     registers.
		 *
		 * The registers are sorted by register number which is the order of
		 * declaration of the respective enumerators.
		 *
		 * @returns
		 *     reference to statically allocated constant array of registers
		 *
		 */
		const std::array<real_register, real_register_count>&
		real_registers() noexcept;

		/**
		 * @brief
		 *     Width of a real register.
		 *
		 * The numeric values of the declared enumerators is guaranteed to be
		 * equal to the bit-width they represent.
		 *
		 * A value-initialized enumerator (zero) shall be used in places where
		 * no width is applicable (such as for a JMP instruction).
		 *
		 */
		enum class bit_width : signed char
		{
			viii  =  8,  ///< 8 bit (a.k.a. "byte" or "half word")
			xvi   = 16,  ///< 16 bit (a.k.a. "word")
			xxxii = 32,  ///< 32 bit (a.k.a. "long word" or "double word")
			lxiv  = 64,  ///< 64 bit (a.k.a. "quad word")
		};

		/**
		 * @brief
		 *     `return`s the name of a register with a specified width.
		 *
		 * For example, `name(real_register::a, width::xxxii)` will `return`
		 * the string `"EAX"`.
		 *
		 * If either argument is not a declared enumerator, the behavior is
		 * undefined.
		 *
		 * The `return` type of this function is a plain old `std::string`
		 * because the register names are short engough to easily fit into the
		 * embedded SSO buffer of every modern `std::string` implementation so
		 * there is no need for additional memory management optimizations.
		 *
		 * @param reg
		 *     register
		 *
		 * @param width
		 *     bit width
		 *
		 * @returns
		 *     complete name of the register
		 *
		 */
		std::string name(real_register reg, bit_width width);

	}  // namespace backend

}  // namespace minijava


#define MINIJAVA_ASM_INCLUDED_FROM_REGISTER_HPP
#include "asm/register.tpp"
#undef MINIJAVA_ASM_INCLUDED_FROM_REGISTER_HPP
