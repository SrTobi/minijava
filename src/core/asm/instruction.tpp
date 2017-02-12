#ifndef MINIJAVA_ASM_INCLUDED_FROM_INSTRUCTION_HPP
#error "Never `#include <asm/instruction.tpp>` directly; `#include <asm/instruction.hpp>` instead."
#endif

#include <cassert>
#include <utility>


namespace minijava
{

	namespace backend
	{

		template <typename RegT>
		std::pair<bit_width, bit_width> get_operand_widths(const instruction<RegT>& instr)
		{
			switch (instr.code) {
			case opcode::op_movslq:
				assert(instr.width == bit_width{});
				return std::make_pair(bit_width::xxxii, bit_width::lxiv);
			case opcode::op_lea:
				assert(instr.width == bit_width{});
				return std::make_pair(bit_width::lxiv, bit_width::lxiv);
			case opcode::op_seta:
			case opcode::op_setae:
			case opcode::op_setb:
			case opcode::op_setbe:
			case opcode::op_sete:
			case opcode::op_setne:
				return std::make_pair(bit_width::viii, bit_width{});
				// TODO: Add other cases here even though we don't need them?
			default:
				return std::make_pair(instr.width, instr.width);
			}
		}

	}  // namespace backend

}  // namespace minijava
