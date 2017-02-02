#include "asm/allocate_registers.hpp"

#include <algorithm>
#include <numeric>

#include "exceptions.hpp"

namespace minijava
{

	namespace backend
	{

		void allocate_registers(const virtual_assembly& virtasm, real_assembly& realasm)
		{
			int argument_count, general_count;
			std::tie(argument_count, general_count) = std::accumulate(
					virtasm.begin(), virtasm.end(), std::make_tuple(0, 0),
					[](auto const& cur_count, auto const& cur_instr) {
						int argc, regc;
						std::tie(argc, regc) = cur_count;
						if (auto reg = get_register(cur_instr.op1)) {
							if (is_argument_register(*reg)) {
								argc = std::max(argc, number(*reg));
							} else if (is_general_register(*reg)) {
								regc = std::max(regc, number(*reg));
							}
						}
						if (auto reg = get_register(cur_instr.op2)) {
							if (is_argument_register(*reg)) {
								argc = std::max(argc, number(*reg));
							} else if (is_general_register(*reg)) {
								regc = std::max(regc, number(*reg));
							}
						}
						return std::make_tuple(argc, regc);
					}
			);
			// TODO: remove after refactoring (basic blocks)
			realasm.emplace_back();
			realasm.back().label = virtasm.front().label;
			// function prologue
			realasm.emplace_back(opcode::op_push, bit_width::lxiv, real_register::bp);
			realasm.emplace_back(opcode::op_mov, bit_width::lxiv, real_register::sp, real_register::bp);
			realasm.emplace_back(opcode::op_sub, bit_width::lxiv, std::int64_t{8} * general_count, real_register::sp);
			(void) argument_count;
			MINIJAVA_NOT_IMPLEMENTED();
		}

	}  // namespace backend

}  // namespace minijava
