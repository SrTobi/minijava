#include "asm/allocator.hpp"

#include <algorithm>
#include <cassert>
#include <map>
#include <numeric>
#include <utility>

#include "exceptions.hpp"


namespace /* anonymous*/
{

	namespace be = minijava::backend;
	using operand = be::operand<be::real_register>;


	bool is_address(const operand& op)
	{
		return be::get_address(op) != nullptr;
	}


	bool is_argument(const be::operand<be::virtual_register>& op)
	{
		if (auto reg = be::get_register(op)) {
			return be::is_argument_register(*reg);
		}
		return false;
	}


	/**
	 * Converts virtual operands to real operands
	 */
	struct op_visitor : public boost::static_visitor<operand>
	{

		operand operator()(std::int64_t imm)
		{
			return imm;
		}

		operand operator()(boost::string_ref name)
		{
			return name;
		}

		operand operator()(be::address<be::virtual_register>)
		{
			MINIJAVA_NOT_REACHED();
		}

		operand operator()(be::virtual_register reg)
		{
			if (be::is_argument_register(reg)) {
				auto num = number(reg);
				switch (num) {
				case 1:
					return be::real_register::di;
				case 2:
					return be::real_register::si;
				case 3:
					return be::real_register::d;
				case 4:
					return be::real_register::c;
				case 5:
					return be::real_register::r8;
				case 6:
					return be::real_register::r9;
				default:
					assert(num > 0);
					auto addr = be::address<be::real_register>{};
					addr.base = be::real_register::bp;
					addr.constant = (num - 6) * std::int64_t{8};
					return std::move(addr);
				}
			} else if (be::is_general_register(reg)) {
				auto addr = be::address<be::real_register>{};
				addr.base = be::real_register::bp;
				addr.constant = -(number(reg) * std::int64_t{8});
				return std::move(addr);
			} else if (reg == be::virtual_register::result) {
				return be::real_register::a;
			} else {
				MINIJAVA_THROW_ICE_MSG(
						minijava::internal_compiler_error,
						"encountered dummy register in virtual assembly"
				);
			}
		}

		operand operator()(boost::blank blank)
		{
			return blank;
		}

	};


	/**
	 * @brief scratch register (no special meaning, not preserved according to ABI)
	 */
	static const auto tmp_register = be::real_register::c;


	/**
	 * @brief
	 *     Helper function for performing an instruction which uses the scratch
	 *     register, if necessary.
	 *
	 * @param code  instruction vector
	 * @param opc   opcode
	 * @param width instruction width
	 * @param op1   first operand
	 * @param op2   second operatnd
	 */
	void add_instruction(std::vector<be::real_instruction>& code,
						 be::opcode opc, be::bit_width width, operand op1,
						 operand op2)
	{
		if (is_address(op1) && is_address(op2)) {
			code.emplace_back(be::opcode::op_mov, width, std::move(op1), tmp_register);
			code.emplace_back(opc, width, tmp_register, std::move(op2));
		} else {
			code.emplace_back(opc, width, std::move(op1), std::move(op2));
		}
	}

}  // namespace /* anonymous */


namespace minijava
{

	namespace backend
	{

		real_assembly allocate_registers(const virtual_assembly& virtasm)
		{
			int argument_count, general_count;
			std::tie(argument_count, general_count) = std::accumulate(
					virtasm.blocks.begin(), virtasm.blocks.end(),
					std::make_tuple(0, 0),
					[](auto const& cur_count, auto const& cur_block) {
						return std::accumulate(
								cur_block.code.begin(), cur_block.code.end(),
								cur_count,
								[](auto const& cur_cnt, auto const& cur_instr) {
									int argc, regc;
									std::tie(argc, regc) = cur_cnt;
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
			// for keeping track of function arguments
			auto next_call_args = std::map<int, operand<real_register>>{};
			auto assert_args_empty = [&next_call_args]() {
				if (!next_call_args.empty()) {
					MINIJAVA_THROW_ICE_MSG(
							minijava::internal_compiler_error,
							"unexpected virtual assembly instruction; function call or argument expected"
					);
				}
			};
			auto assert_args_complete = [&next_call_args]() {
				// no matter what order the arguments were originally specified
				// in, they should have consecutive numbers before a call
				auto size = next_call_args.size();
				auto last_arg = static_cast<std::size_t>(next_call_args.rbegin()->first);
				if (size != last_arg) {
					MINIJAVA_THROW_ICE_MSG(
							minijava::internal_compiler_error,
							"not all function arguments were specified in the virtual assembly"
					);
				}
			};
			// transform basic blocks
			auto visitor = op_visitor{};
			for (auto const& block : virtasm.blocks) {
				auto real_block = basic_block<real_register>{block.label};
				for (auto const& instr : block.code) {
					switch (instr.code) {
					case opcode::op_mov:
					{
						auto op1 = instr.op1.apply_visitor(visitor);
						if (is_argument(instr.op2)) {
							auto reg = get_register(instr.op2);
							assert(reg);
							next_call_args.emplace(number(*reg), op1);
						} else {
							assert_args_empty();
							auto op2 = instr.op2.apply_visitor(visitor);
							add_instruction(
									real_block.code, opcode::op_mov,
									instr.width, std::move(op1),
									std::move(op2)
							);
						}
						break;
					}
					case opcode::mac_call_aligned:
					case opcode::op_call:
					{
						assert_args_complete();
						auto call_argc = static_cast<int>(next_call_args.size());
						auto saved_registers = std::min(6, std::min(call_argc, argument_count));
						// save argument registers
						switch (saved_registers) {
						case 6:
							real_block.code.emplace_back(opcode::op_push, bit_width::lxiv, be::real_register::r9);
						case 5:
							real_block.code.emplace_back(opcode::op_push, bit_width::lxiv, be::real_register::r8);
						case 4:
							real_block.code.emplace_back(opcode::op_push, bit_width::lxiv, be::real_register::c);
						case 3:
							real_block.code.emplace_back(opcode::op_push, bit_width::lxiv, be::real_register::d);
						case 2:
							real_block.code.emplace_back(opcode::op_push, bit_width::lxiv, be::real_register::si);
						case 1:
							real_block.code.emplace_back(opcode::op_push, bit_width::lxiv, be::real_register::di);
						case 0:
							break;
						default:
							MINIJAVA_NOT_REACHED();
						}
						// push stack arguments (RTL)
						for (int i = call_argc; i > 6; --i) {
							real_block.code.emplace_back(opcode::op_push, bit_width::lxiv, next_call_args.at(i));
						}
						// set register arguments
						switch (std::min(call_argc, 6)) {
						case 6:
							real_block.code.emplace_back(opcode::op_mov, bit_width::lxiv, next_call_args.at(6), be::real_register::r9);
						case 5:
							real_block.code.emplace_back(opcode::op_mov, bit_width::lxiv, next_call_args.at(5), be::real_register::r8);
						case 4:
							real_block.code.emplace_back(opcode::op_mov, bit_width::lxiv, next_call_args.at(4), be::real_register::c);
						case 3:
							real_block.code.emplace_back(opcode::op_mov, bit_width::lxiv, next_call_args.at(3), be::real_register::d);
						case 2:
							real_block.code.emplace_back(opcode::op_mov, bit_width::lxiv, next_call_args.at(2), be::real_register::si);
						case 1:
							real_block.code.emplace_back(opcode::op_mov, bit_width::lxiv, next_call_args.at(1), be::real_register::di);
						case 0:
							break;
						default:
							MINIJAVA_NOT_REACHED();
						}
						// perform actual call
						auto target = get_name(instr.op1);
						if (target == nullptr) {
							MINIJAVA_THROW_ICE_MSG(
								minijava::internal_compiler_error,
								"call without target encountered"
							);
						}
						real_block.code.emplace_back(instr.code, instr.width, *target);
						// reset stack pointer (remove stack arguments)
						if (call_argc > 6) {
							real_block.code.emplace_back(opcode::op_add, bit_width::lxiv, std::int64_t{8} * (call_argc - 6), real_register::sp);
						}
						// restore argument registers
						switch (saved_registers) {
						case 6:
							real_block.code.emplace_back(opcode::op_pop, bit_width::lxiv, be::real_register::di);
							real_block.code.emplace_back(opcode::op_pop, bit_width::lxiv, be::real_register::si);
							real_block.code.emplace_back(opcode::op_pop, bit_width::lxiv, be::real_register::d);
							real_block.code.emplace_back(opcode::op_pop, bit_width::lxiv, be::real_register::c);
							real_block.code.emplace_back(opcode::op_pop, bit_width::lxiv, be::real_register::r8);
							real_block.code.emplace_back(opcode::op_pop, bit_width::lxiv, be::real_register::r9);
							break;
						case 5:
							real_block.code.emplace_back(opcode::op_pop, bit_width::lxiv, be::real_register::di);
							real_block.code.emplace_back(opcode::op_pop, bit_width::lxiv, be::real_register::si);
							real_block.code.emplace_back(opcode::op_pop, bit_width::lxiv, be::real_register::d);
							real_block.code.emplace_back(opcode::op_pop, bit_width::lxiv, be::real_register::c);
							real_block.code.emplace_back(opcode::op_pop, bit_width::lxiv, be::real_register::r8);
							break;
						case 4:
							real_block.code.emplace_back(opcode::op_pop, bit_width::lxiv, be::real_register::di);
							real_block.code.emplace_back(opcode::op_pop, bit_width::lxiv, be::real_register::si);
							real_block.code.emplace_back(opcode::op_pop, bit_width::lxiv, be::real_register::d);
							real_block.code.emplace_back(opcode::op_pop, bit_width::lxiv, be::real_register::c);
							break;
						case 3:
							real_block.code.emplace_back(opcode::op_pop, bit_width::lxiv, be::real_register::di);
							real_block.code.emplace_back(opcode::op_pop, bit_width::lxiv, be::real_register::si);
							real_block.code.emplace_back(opcode::op_pop, bit_width::lxiv, be::real_register::d);
							break;
						case 2:
							real_block.code.emplace_back(opcode::op_pop, bit_width::lxiv, be::real_register::di);
							real_block.code.emplace_back(opcode::op_pop, bit_width::lxiv, be::real_register::si);
							break;
						case 1:
							real_block.code.emplace_back(opcode::op_pop, bit_width::lxiv, be::real_register::di);
							break;
						case 0:
							break;
						default:
							MINIJAVA_NOT_REACHED();
						}
						// reset state
						next_call_args.clear();
						break;
					}
					case opcode::op_ret:
						assert_args_empty();
						// epilogue
						real_block.code.emplace_back(opcode::op_mov, bit_width::lxiv, real_register::bp, real_register::sp);
						real_block.code.emplace_back(opcode::op_pop, bit_width::lxiv, real_register::bp);
						real_block.code.emplace_back(opcode::op_ret);
						break;
					case opcode::op_add:
					case opcode::op_sub:
					case opcode::op_mul:
					case opcode::mac_div:
					case opcode::mac_mod:
					{
						assert_args_empty();
						auto op1 = instr.op1.apply_visitor(visitor);
						auto op2 = instr.op2.apply_visitor(visitor);
						add_instruction(
								real_block.code, instr.code, instr.width,
								std::move(op1), std::move(op2)
						);
						break;
					}
					default:
						MINIJAVA_NOT_REACHED();
					}
				}
				realasm.blocks.push_back(std::move(real_block));
			}
			return realasm;
		}

	}  // namespace backend

}  // namespace minijava
