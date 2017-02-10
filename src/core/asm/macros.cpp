#include "asm/macros.hpp"

#include <cassert>

#include "exceptions.hpp"


namespace minijava
{

	namespace backend
	{

		namespace /*anonymous*/
		{

			void expand_call_aligned_macro(const real_instruction& call, real_basic_block& dst)
			{
				assert(call.code == opcode::mac_call_aligned);
				auto atsp = real_address{};
				atsp.base = real_register::sp;
				dst.code.emplace_back(opcode::op_push, bit_width::lxiv, real_register::sp);
				dst.code.emplace_back(opcode::op_push, bit_width::lxiv, atsp);
				dst.code.emplace_back(opcode::op_and, bit_width::lxiv, -16, real_register::sp);
				dst.code.emplace_back(opcode::op_call, bit_width{}, call.op1);
				atsp.constant = 8;
				dst.code.emplace_back(opcode::op_mov, bit_width::lxiv, atsp, real_register::sp);
			}

			void expand_div_macro(const real_instruction& div, real_basic_block& dst)
			{
				assert(div.code == opcode::mac_div);
				auto dst_reg = get_register(div.op2);
				assert(dst_reg);
				if (div.width != bit_width::lxiv) {
					MINIJAVA_NOT_IMPLEMENTED();
				}
				dst.code.emplace_back(opcode::op_push, bit_width::lxiv, real_register::a);
				dst.code.emplace_back(opcode::op_push, bit_width::lxiv, real_register::d);
				dst.code.emplace_back(opcode::op_mov, div.width, div.op1, real_register::d);
				dst.code.emplace_back(opcode::op_cdq);
				dst.code.emplace_back(opcode::op_idiv, div.width, *dst_reg);
				dst.code.emplace_back(opcode::op_mov, div.width, real_register::a, *dst_reg);
				dst.code.emplace_back(opcode::op_pop, bit_width::lxiv, real_register::d);
				dst.code.emplace_back(opcode::op_pop, bit_width::lxiv, real_register::a);
			}

			void expand_mod_macro(const real_instruction& mod, real_basic_block& dst)
			{
				assert(mod.code == opcode::mac_mod);
				auto dst_reg = get_register(mod.op2);
				assert(dst_reg);
				if (mod.width != bit_width::lxiv) {
					MINIJAVA_NOT_IMPLEMENTED();
				}
				dst.code.emplace_back(opcode::op_push, bit_width::lxiv, real_register::a);
				dst.code.emplace_back(opcode::op_push, bit_width::lxiv, real_register::d);
				dst.code.emplace_back(opcode::op_mov, mod.width, mod.op1, real_register::d);
				dst.code.emplace_back(opcode::op_cdq);
				dst.code.emplace_back(opcode::op_idiv, mod.width, *dst_reg);
				dst.code.emplace_back(opcode::op_mov, mod.width, real_register::d, *dst_reg);
				dst.code.emplace_back(opcode::op_pop, bit_width::lxiv, real_register::d);
				dst.code.emplace_back(opcode::op_pop, bit_width::lxiv, real_register::a);
			}

			void expand_macros(real_basic_block& src, real_basic_block& dst)
			{
				for (auto&& instr : src.code) {
					switch (instr.code) {
					case opcode::mac_call_aligned:
						expand_call_aligned_macro(instr, dst);
						break;
					case opcode::mac_div:
						expand_div_macro(instr, dst);
						break;
					case opcode::mac_mod:
						expand_mod_macro(instr, dst);
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
