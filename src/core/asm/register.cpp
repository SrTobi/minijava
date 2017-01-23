#include "asm/register.hpp"

#include "exceptions.hpp"


namespace minijava
{

	namespace backend
	{

		namespace /* anonymous */
		{

			template <std::size_t... Nums>
			constexpr std::array<real_register, sizeof...(Nums)>
			make_the_real_registers(std::index_sequence<Nums...>) noexcept
			{
				return {{static_cast<real_register>(Nums)...}};
			}

		}  // namespace /* anonymous */

		static constexpr auto the_real_registers = make_the_real_registers(
			std::make_index_sequence<real_register_count>{}
		);

		const std::array<real_register, real_register_count>&
		real_registers() noexcept
		{
			return the_real_registers;
		}

		namespace /* anonymous */
		{

			std::size_t get_table_index(const bit_width width)
			{
				switch (width) {
				case bit_width::viii:  return 0;
				case bit_width::xvi:   return 1;
				case bit_width::xxxii: return 2;
				case bit_width::lxiv:  return 3;
				}
				MINIJAVA_NOT_REACHED();
			}

		}  // namespace /* anonymous */

		const char* name(const real_register reg, const bit_width width)
		{
			static const char register_names[real_register_count][4][5] = {
				{"al",   "ax",   "eax",  "rax"},
				{"bl",   "bx",   "ebx",  "rbx"},
				{"cl",   "cx",   "ecx",  "rcx"},
				{"dl",   "dx",   "edx",  "rdx"},
				{"bpl",  "bp",   "ebp",  "rbp"},
				{"spl",  "sp",   "esp",  "rsp"},
				{"sil",  "si",   "esi",  "rsi"},
				{"dil",  "di",   "edi",  "rdi"},
				{"r8l",  "r8w",  "r8d",  "r8" },
				{"r9l",  "r9w",  "r9d",  "r9" },
				{"r10l", "r10w", "r10d", "r10"},
				{"r11l", "r11w", "r11d", "r11"},
				{"r12l", "r12w", "r12d", "r12"},
				{"r13l", "r13w", "r13d", "r13"},
				{"r14l", "r14w", "r14d", "r14"},
				{"r15l", "r15w", "r15d", "r15"},
			};
			const auto i = number(reg);
			const auto j = get_table_index(width);
			return register_names[i][j];
		}

	}  // namespace backend

}  // namespace minijava
