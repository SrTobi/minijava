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
	 * @brief scratch register (no special meaning, not preserved according to ABI)
	 */
	static const auto tmp_register = be::real_register::r10;

	/**
	 * @brief scratch register for address calculation (no special meaning, not preserved according to ABI)
	 */
	static const auto tmp_address_register = be::real_register::r11;

	/**
	 * @brief
	 *     Returns the argument register for the argument at the given position.
	 *
	 * @param id argument position
	 * @return argument register
	 */
	be::real_register get_argument_register(int id)
	{
		assert(id > 0 && id < 7);
		switch (id) {
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
			MINIJAVA_NOT_REACHED();
		}
	}


	/**
	 * @brief
	 *     Converts virtual operands to real operands.
	 *
	 * Uses the temporary address register for address calculations, if
	 * necessary.
	 */
	struct op_visitor : public boost::static_visitor<operand>
	{

		op_visitor(std::vector<be::real_instruction>& code) : _code{code} {}

		operand operator()(std::int64_t imm)
		{
			return imm;
		}

		operand operator()(const std::string& name)
		{
			return name;
		}

		operand operator()(be::virtual_address vaddr)
		{
			assert(!vaddr.index);
			if (auto base = vaddr.base) {
				// check nature of the base register to determine whether an
				// additional mov is necessary to get the base address
				auto base_op = operator()(*base);
				if (is_address(base_op)) {
					_code.emplace_back(
							be::opcode::op_mov, be::bit_width::lxiv,
							base_op, tmp_address_register
					);
					return be::real_address{
							vaddr.constant, tmp_address_register, boost::none,
							vaddr.scale
					};
				} else {
					auto base_reg = be::get_register(base_op);
					assert(base_reg);
					return be::real_address{
							vaddr.constant, *base_reg, boost::none, vaddr.scale
					};
				}
			} else {
				return be::real_address{
						vaddr.constant, boost::none, boost::none, vaddr.scale
				};
			}
		}

		operand operator()(be::virtual_register reg)
		{
			if (be::is_argument_register(reg)) {
				auto num = number(reg);
				assert(num > 0);
				if (num < 7) {
					return get_argument_register(num);
				} else {
					auto addr = be::real_address{};
					addr.base = be::real_register::bp;
					addr.constant = (num - 6) * std::int64_t{8};
					return std::move(addr);
				}
			} else if (be::is_general_register(reg)) {
				auto addr = be::real_address{};
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

	private:

		std::vector<be::real_instruction>& _code;

	};


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
			for (auto const& block : virtasm.blocks) {
				auto real_block = basic_block<real_register>{block.label};
				auto visitor = op_visitor{real_block.code};
				for (auto const& instr : block.code) {
					// working with two addresses would break the address calculation in the visitor
					assert(get_address(instr.op1) == nullptr || get_address(instr.op2) == nullptr);
					switch (instr.code) {
					case opcode::mac_call_aligned:
					case opcode::op_call:
					{
						assert_args_complete();
						auto call_argc = static_cast<int>(next_call_args.size());
						auto saved_registers = std::min(6, argument_count);
						// save own argument registers (RTL)
						for (int i = saved_registers; i > 0; --i) {
							real_block.code.emplace_back(opcode::op_push, bit_width::lxiv, get_argument_register(i));
						}
						// push stack arguments (RTL)
						for (int i = call_argc; i > 6; --i) {
							real_block.code.emplace_back(opcode::op_push, bit_width::lxiv, next_call_args.at(i));
						}
						// set register arguments
						for (int i = std::min(call_argc, 6); i > 0; --i) {
							real_block.code.emplace_back(opcode::op_mov, bit_width::lxiv, next_call_args.at(i), get_argument_register(i));
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
						// restore own argument registers (LTR)
						for (int i = 1; i <= saved_registers; ++i) {
							real_block.code.emplace_back(opcode::op_pop, bit_width::lxiv, get_argument_register(i));
						}
						// reset state
						next_call_args.clear();
						break;
					}
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
					case opcode::op_movslq:
					{
						assert_args_empty();
						assert(!is_argument(instr.op2));
						auto op1 = instr.op1.apply_visitor(visitor);
						auto op2 = instr.op2.apply_visitor(visitor);
						add_instruction(
								real_block.code, opcode::op_mov,
								instr.width, std::move(op1),
								std::move(op2)
						);
						break;
					}
					case opcode::op_lea:
					{
						assert_args_empty();
						assert(!is_argument(instr.op2));
						auto op1 = instr.op1.apply_visitor(visitor);
						auto op2 = instr.op2.apply_visitor(visitor);
						add_instruction(
								real_block.code, instr.code, instr.width,
								std::move(op1), std::move(op2)
						);
						break;
					}
					case opcode::op_add:
					case opcode::op_sub:
					case opcode::op_mul:
					case opcode::mac_div:
					case opcode::mac_mod:
					{
						assert_args_empty();
						assert(!is_argument(instr.op2));
						auto op1 = instr.op1.apply_visitor(visitor);
						auto op2 = instr.op2.apply_visitor(visitor);
						add_instruction(
								real_block.code, instr.code, instr.width,
								std::move(op1), std::move(op2)
						);
						break;
					}
					case opcode::op_neg:
						assert_args_empty();
						assert(!is_argument(instr.op1));
						assert(empty(instr.op2));
						real_block.code.emplace_back(
								instr.code, instr.width,
								instr.op1.apply_visitor(visitor), boost::blank{}
						);
						break;
					case opcode::op_cmp:
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
					case opcode::op_seta:
					case opcode::op_setae:
					case opcode::op_setb:
					case opcode::op_setbe:
					case opcode::op_sete:
					case opcode::op_setne:
						assert_args_empty();
						assert(!is_argument(instr.op1));
						assert(empty(instr.op2));
						real_block.code.emplace_back(
								instr.code, instr.width,
								instr.op1.apply_visitor(visitor), boost::blank{}
						);
						break;
					case opcode::op_jmp:
					case opcode::op_je:
						assert_args_empty();
						assert(empty(instr.op2));
						real_block.code.emplace_back(
								instr.code, instr.width,
								instr.op1.apply_visitor(visitor), boost::blank{}
						);
						break;
					case opcode::op_ret:
						assert_args_empty();
						// epilogue
						real_block.code.emplace_back(opcode::op_mov, bit_width::lxiv, real_register::bp, real_register::sp);
						real_block.code.emplace_back(opcode::op_pop, bit_width::lxiv, real_register::bp);
						real_block.code.emplace_back(opcode::op_ret);
						break;
					default:
						MINIJAVA_THROW_ICE_MSG(
								minijava::not_implemented_error,
								mnemotic(instr.code)
						);
					}
				}
				realasm.blocks.push_back(std::move(real_block));
			}
			return realasm;
		}

	}  // namespace backend

}  // namespace minijava
