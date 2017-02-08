#include "asm/macros.hpp"

#include "exceptions.hpp"


namespace minijava
{

	namespace backend
	{

		namespace /*anonymous*/
		{

			void expand_macros(real_basic_block& src, real_basic_block& dst)
			{
				for (auto&& instr : src.code) {
					switch (instr.code) {
					case opcode::mac_call_aligned:
						dst.code.push_back(std::move(instr));
						dst.code.back().code = opcode::op_call;  // TODO: Actually align the call!
						break;
					case opcode::mac_div:
						if (instr.width != bit_width::lxiv) {
							MINIJAVA_NOT_IMPLEMENTED();
						}
						dst.code.emplace_back(opcode::op_push, bit_width::lxiv, real_register::d);
						dst.code.emplace_back(opcode::op_push, bit_width::lxiv, real_register::a);
						dst.code.emplace_back(opcode::op_mov, instr.width, *get_register(instr.op1), real_register::d);
						dst.code.emplace_back(opcode::op_cdq);
						dst.code.emplace_back(opcode::op_idiv, instr.width, *get_register(instr.op2));
						dst.code.emplace_back(opcode::op_mov, instr.width, real_register::a, *get_register(instr.op2));
						dst.code.emplace_back(opcode::op_pop, bit_width::lxiv, real_register::a);
						dst.code.emplace_back(opcode::op_pop, bit_width::lxiv, real_register::d);
						break;
					case opcode::mac_mod:
						if (instr.width != bit_width::lxiv) {
							MINIJAVA_NOT_IMPLEMENTED();
						}
						dst.code.emplace_back(opcode::op_push, bit_width::lxiv, real_register::d);
						dst.code.emplace_back(opcode::op_push, bit_width::lxiv, real_register::a);
						dst.code.emplace_back(opcode::op_mov, instr.width, *get_register(instr.op1), real_register::d);
						dst.code.emplace_back(opcode::op_cdq);
						dst.code.emplace_back(opcode::op_idiv, instr.width, *get_register(instr.op2));
						dst.code.emplace_back(opcode::op_mov, instr.width, real_register::d, *get_register(instr.op2));
						dst.code.emplace_back(opcode::op_pop, bit_width::lxiv, real_register::a);
						dst.code.emplace_back(opcode::op_pop, bit_width::lxiv, real_register::d);
						break;
					default:
						dst.code.push_back(std::move(instr));
					}
				}
			}

		}  // namespace /*anonymous*/

		void expand_macros(real_assembly& ass)
		{
			using std::swap;
			auto scratch = real_basic_block{""};
			for (auto&& bb : ass.blocks) {
				scratch.code.clear();
				expand_macros(bb, scratch);
				swap(bb.code, scratch.code);
			}
		}

	}  // namespace backend

}  // namespace minijava
