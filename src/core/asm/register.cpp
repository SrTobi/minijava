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

		std::string name(const real_register /*reg*/, const bit_width /*width*/)
		{
			MINIJAVA_NOT_IMPLEMENTED();
		}

	}  // namespace backend

}  // namespace minijava
