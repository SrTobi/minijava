#include "asm/allocator.hpp"

#include <algorithm>
#include <utility>
#include <numeric>

#include "exceptions.hpp"

namespace minijava
{

	namespace backend
	{

		real_assembly allocate_registers(const virtual_assembly& virtasm)
		{
			auto fixme_asap = std::vector<virtual_instruction>{};
			for (auto&& bb : virtasm.blocks) {
				for (auto&& in : bb.code) {
					fixme_asap.push_back(in);
				}
			}
			int argument_count, general_count;
			std::tie(argument_count, general_count) = std::accumulate(
					fixme_asap.begin(), fixme_asap.end(), std::make_tuple(0, 0),
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
			auto realasm = real_assembly{virtasm.ldname};
			{
				// function prologue
				auto prologue = basic_block<real_register>{""};
				prologue.code.emplace_back(opcode::op_push, bit_width::lxiv, real_register::bp);
				prologue.code.emplace_back(opcode::op_mov, bit_width::lxiv, real_register::sp, real_register::bp);
				prologue.code.emplace_back(opcode::op_sub, bit_width::lxiv, std::int64_t{8} * general_count, real_register::sp);
				realasm.blocks.push_back(std::move(prologue));
			}
			(void) argument_count;
			MINIJAVA_NOT_IMPLEMENTED();
		}

	}  // namespace backend

}  // namespace minijava
