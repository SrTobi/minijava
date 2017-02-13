#include "asm/generator.hpp"

#include <algorithm>
#include <cassert>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <iostream>

#include <boost/variant/apply_visitor.hpp>
#include <iostream>

#include "exceptions.hpp"


#ifdef __GNUC__
#  define USELESS __attribute__((unused))
#else
#  define USELESS /* empty */
#endif

namespace minijava
{

	namespace backend
	{

		namespace /* anonymous */
		{

			USELESS std::string to_string(firm::ir_node*const irn)
			{
				char buffer[128];
				std::snprintf(
					buffer, sizeof(buffer), "%6lu %4s %p %s",
					firm::get_irn_node_nr(irn),
					firm::get_mode_name(firm::get_irn_mode(irn)),
					static_cast<void*>(irn),
					firm::get_irn_opname(irn)
				);
				return buffer;
			}

			bool const_mode_b_as_bool(firm::ir_tarval* tv)
			{
				assert(tv != nullptr);
				if (tv == firm::tarval_b_false) {
					return false;
				}
				if (tv == firm::tarval_b_true) {
					return true;
				}
				MINIJAVA_NOT_REACHED();
			}

			bool can_be_in_register(firm::ir_mode*const mode) noexcept
			{
				if (firm::mode_is_data(mode))      { return true; }
				if (firm::mode_is_reference(mode)) { return true; }
				if (mode == firm::mode_T)          { return true; }
				return false;
			}

			bool can_be_in_register(firm::ir_node*const irn) noexcept
			{
				const auto mode = firm::get_irn_mode(irn);
				return can_be_in_register(mode);
			}

			USELESS bool can_be_in_register(firm::ir_node*const irn, const virtual_register reg) noexcept
			{
				assert(reg != virtual_register::dummy);
				const auto mode = firm::get_irn_mode(irn);
				if (!can_be_in_register(irn)) {
					return false;
				}
				if (firm::is_Phi(irn)) {
					// Phis are always in data registers, regardless their mode.
					return (reg >= virtual_register::general);
				}
				const auto is_modeb = (mode == firm::mode_b);
				const auto is_flags = (reg == virtual_register::flags);
				return (is_modeb == is_flags);
			}

			bool is_flag(firm::ir_node*const irn) noexcept
			{
				return (firm::get_irn_mode(irn) == firm::mode_b);
			}

			USELESS bool is_exec(firm::ir_node*const irn) noexcept
			{
				return (firm::get_irn_mode(irn) == firm::mode_X);
			}

			bit_width get_width(firm::ir_mode*const irm)
			{
				assert(can_be_in_register(irm));
				const auto bits = firm::get_mode_size_bits(irm);
				const auto width = static_cast<bit_width>(bits);
				assert(!firm::mode_is_reference(irm) || (width == bit_width::lxiv));
				assert((width == bit_width::viii)
				    || (width == bit_width::xvi)
				    || (width == bit_width::xxxii)
				    || (width == bit_width::lxiv)
				);
				return width;
			}

			bit_width get_width(firm::ir_type*const irt)
			{
				return get_width(firm::get_type_mode(irt));
			}

			bit_width get_width(firm::ir_node*const irn)
			{
				if (firm::is_Call(irn)) {
					const auto method_entity = firm::get_Call_callee(irn);
					const auto method_type = firm::get_entity_type(method_entity);
					const auto method_results = firm::get_method_n_ress(method_type);
					switch (method_results) {
					case 0: return bit_width{};
					case 1: return get_width(firm::get_method_res_type(method_type, 0));
					default: MINIJAVA_NOT_REACHED_MSG(std::to_string(method_results));
					}
				}
				if (firm::is_Load(irn)) {
					return get_width(firm::get_Load_mode(irn));
				}
				return get_width(firm::get_irn_mode(irn));
			}

			struct cond_target_blocks
			{
				firm::ir_node* then_block{};
				firm::ir_node* else_block{};
			};

			cond_target_blocks get_cond_targets(firm::ir_node*const irn)
			{
				assert(firm::is_Cond(irn));
				assert(firm::get_irn_n_outs(irn) == 2);
				const auto lhsproj = firm::get_irn_out(irn, 0);
				const auto rhsproj = firm::get_irn_out(irn, 1);
				assert(firm::is_Proj(lhsproj));
				assert(is_exec(lhsproj));
				assert(firm::is_Proj(rhsproj));
				assert(is_exec(rhsproj));
				assert(firm::get_irn_n_outs(lhsproj) == 1);
				assert(firm::get_irn_n_outs(rhsproj) == 1);
				const auto lhsblk = firm::get_irn_out(lhsproj, 0);
				const auto rhsblk = firm::get_irn_out(rhsproj, 0);
				assert(firm::is_Block(lhsblk));
				assert(firm::is_Block(rhsblk));
				const auto lhsprojnum = firm::get_Proj_num(lhsproj);
				const auto rhsprojnum = firm::get_Proj_num(rhsproj);
				if ((lhsprojnum == firm::pn_Cond_true) && (rhsprojnum == firm::pn_Cond_false)) {
					return {lhsblk, rhsblk};
				}
				if ((lhsprojnum == firm::pn_Cond_false) && (rhsprojnum == firm::pn_Cond_true)) {
					return {rhsblk, lhsblk};
				}
				MINIJAVA_NOT_REACHED();
			}


			opcode get_conditional_jump_op(const firm::ir_relation rel)
			{
				switch (rel) {
				case firm::ir_relation_false:
					return opcode::op_nop;
				case firm::ir_relation_equal:
					return opcode::op_je;
				case firm::ir_relation_less:
					return opcode::op_jl;
				case firm::ir_relation_greater:
					return opcode::op_jg;
				case firm::ir_relation_less_equal:
					return opcode::op_jle;
				case firm::ir_relation_greater_equal:
					return opcode::op_jge;
				case firm::ir_relation_less_greater:  // a.k.a. "not equal"
					return opcode::op_jne;
				case firm::ir_relation_true:
					return opcode::op_jmp;
				default:
					MINIJAVA_NOT_IMPLEMENTED_MSG(firm::get_relation_string(rel));
				}
			}

			opcode get_conditional_set_op(const firm::ir_relation rel)
			{
				switch (rel) {
				case firm::ir_relation_equal:
					return opcode::op_sete;
				case firm::ir_relation_less:
					return opcode::op_setl;
				case firm::ir_relation_greater:
					return opcode::op_setg;
				case firm::ir_relation_less_equal:
					return opcode::op_setle;
				case firm::ir_relation_greater_equal:
					return opcode::op_setge;
				case firm::ir_relation_less_greater:  // a.k.a. "not equal"
					return opcode::op_setne;
				default:
					MINIJAVA_NOT_IMPLEMENTED_MSG(firm::get_relation_string(rel));
				}
			}


			class bb_meta
			{
			public:

				std::vector<virtual_instruction> phis_on_cond_branch{};
				std::vector<virtual_instruction> jump_on_cond_branch{};
				std::vector<virtual_instruction> phis_on_fall_through{};
				std::vector<virtual_instruction> jump_on_fall_through{};

				bb_meta(const std::size_t index) noexcept : _index{index}
				{
				}

				std::size_t index() const noexcept
				{
					return _index;
				}

				bb_meta& set_succ_cond_branch(firm::ir_node*const irn)
				{
					assert(_succ_cond_branch == nullptr);
					_succ_cond_branch = irn;
					return *this;
				}

				bb_meta& set_succ_fall_through(firm::ir_node*const irn)
				{
					assert(_succ_fall_through == nullptr);
					_succ_fall_through = irn;
					return *this;
				}

				firm::ir_node* get_succ_cond_branch() const noexcept
				{
					return _succ_cond_branch;
				}

				firm::ir_node* get_succ_fall_through() const noexcept
				{
					return _succ_fall_through;
				}

			private:

				std::size_t _index{};
				firm::ir_node* _succ_cond_branch{};
				firm::ir_node* _succ_fall_through{};
			};


			class generator final
			{
			public:

				generator(const char*const funcname)
					: _assembly{funcname}
					, _nextreg{virtual_register::general}
				{
					_assembly.blocks.emplace_back("");
				}

				void handle_parameters(firm::ir_graph*const irg)
				{
					const auto entity = firm::get_irg_entity(irg);
					const auto type = firm::get_entity_type(entity);
					assert(firm::is_Method_type(type));
					const auto arity = firm::get_method_n_params(type);
					auto argument_nodes = std::vector<firm::ir_node*>(arity);
					const auto start = firm::get_irg_start(irg);
					const auto n = firm::get_irn_n_outs(start);
					for (auto i = 0u; i < n; ++i) {
						const auto out = firm::get_irn_out(start, i);
						if (firm::is_Proj(out)) {
							const auto m = firm::get_irn_n_outs(out);
							for (auto j = 0u; j < m; ++j) {
								const auto irn = firm::get_irn_out(out, j);
								//std::clog << "\t\t[inner] " << to_string(irn) << std::endl;
								if (firm::is_Proj(irn)) {
									const auto idx = firm::get_Proj_num(irn);
									assert(idx < arity);
									argument_nodes[idx] = irn;
								}
							}
						}
					}
					auto argreg = virtual_register::argument;
					for (std::size_t i = 0; i < arity; ++i) {
						const auto irn = argument_nodes[i];
						if (irn != nullptr) {
							// parameter was used
							_set_register(irn, argreg);
						}
						argreg = next_argument_register(argreg);
					}
				}

				void visit_first_pass(firm::ir_node*const irn)
				{
					if (firm::is_Block(irn)) {
						_current_block = irn;
					} else {
						_current_block = firm::get_nodes_block(irn);
					}
					if (firm::is_Phi(irn) && can_be_in_register(irn)) {
						// Phi nodes are used before visited so we must
						// allocate a register now.  Phis are always in data
						// registers because the flags register cannot be
						// preserved.  Special code is needed to move the value
						// in and out again.
						const auto reg = _next_data_register();
						_set_register(irn, reg);
					} else if (firm::is_Jmp(irn)) {
						assert(firm::get_irn_n_outs(irn) == 1);
						const auto targirn = firm::get_irn_out(irn, 0);
						_provide_bb(_current_block).set_succ_fall_through(targirn);
					} else if (firm::is_Cond(irn)) {
						const auto targets = get_cond_targets(irn);
						_provide_bb(_current_block)
							.set_succ_cond_branch(targets.then_block)
							.set_succ_fall_through(targets.else_block);
					}
					_blockmap[irn] = _current_block;
				}

				void visit_second_pass(firm::ir_node*const irn)
				{
					_current_block = _blockmap.at(irn);
					_provide_bb(_current_block);
					switch (firm::get_irn_opcode(irn)) {
					case firm::iro_Start:
						_visit_start(irn);
						break;
					case firm::iro_End:
						_visit_end(irn);
						break;
					case firm::iro_Block:
						_visit_block(irn);
						break;
					case firm::iro_Const:
						_visit_const(irn);
						break;
					case firm::iro_Add:
						_visit_binop(irn, opcode::op_add);
						break;
					case firm::iro_Sub:
						_visit_binop(irn, opcode::op_sub);
						break;
					case firm::iro_Mul:
						_visit_binop(irn, opcode::op_imul);
						break;
					case firm::iro_Div:
						_visit_div(irn);
						break;
					case firm::iro_Mod:
						_visit_mod(irn);
						break;
					case firm::iro_Minus:
						_visit_minus(irn);
						break;
					case firm::iro_Conv:
						_visit_conv(irn);
						break;
					case firm::iro_Address:
						_visit_address(irn);
						break;
					case firm::iro_Load:
						_visit_load(irn);
						break;
					case firm::iro_Store:
						_visit_store(irn);
						break;
					case firm::iro_Call:
						_visit_call(irn);
						break;
					case firm::iro_Return:
						_visit_return(irn);
						break;
					case firm::iro_Cmp:
						_visit_cmp(irn);
						break;
					case firm::iro_Jmp:
						_visit_jmp(irn);
						break;
					case firm::iro_Cond:
						_visit_cond(irn);
						break;
					case firm::iro_Phi:
						_visit_phi(irn);
						break;
					case firm::iro_Proj:
						_visit_proj(irn);
						break;
					default:
						MINIJAVA_NOT_REACHED_MSG(firm::get_irn_opname(irn));
					}
				}

				virtual_assembly get() &&
				{
					_finalize();
					return std::move(_assembly);
				}

			private:

				std::map<firm::ir_node*, firm::ir_node*> _blockmap{};
				std::map<firm::ir_node*, bb_meta> _metamap{};
				std::map<firm::ir_node*, virtual_register> _registers{};
				std::map<firm::ir_entity*, virtual_register> _addresses{};
				firm::ir_node* _current_block{};
				virtual_assembly _assembly;
				virtual_register _nextreg;

				bb_meta& _provide_bb(firm::ir_node*const blk)
				{
					using namespace std::string_literals;
					assert(blk != nullptr);
					assert(firm::is_Block(blk));
					const auto pos = _metamap.find(blk);
					if (pos != _metamap.cend()) {
						return pos->second;
					}
					const auto idx = _assembly.blocks.size();
					auto label = ".L"s + std::to_string(firm::get_irn_node_nr(blk));
					_assembly.blocks.emplace_back(std::move(label));
					return _metamap.insert({blk, bb_meta{idx}}).first->second;
				}

				void _set_register(firm::ir_node*const irn, const virtual_register reg)
				{
					assert(can_be_in_register(irn, reg));
					assert(reg != virtual_register::dummy);
					const auto info = _registers.insert({irn, reg});
					if (!info.second) {
						assert(reg == info.first->second);
					}
				}

				virtual_register _get_register_or_dummy(firm::ir_node*const irn) const
				{
					const auto pos = _registers.find(irn);
					return (pos == _registers.cend())
						? virtual_register::dummy
						: pos->second;
				}

				virtual_register _get_data_register(firm::ir_node*const irn) const
				{
					const auto reg = _get_register_or_dummy(irn);
					assert(reg != virtual_register::dummy);
					assert(reg != virtual_register::flags);
					return reg;
				}

				virtual_register _get_flags_register(firm::ir_node*const irn) const
				{
					const auto reg = _get_register_or_dummy(irn);
					assert(reg != virtual_register::dummy);
					assert(reg == virtual_register::flags);
					return reg;
				}

				virtual_register _next_data_register()
				{
					const auto current = _nextreg;
					_nextreg = next_general_register(current);
					return current;
				}

				virtual_basic_block& _get_basic_block(firm::ir_node*const irn)
				{
					const auto blk = _blockmap.at(irn);
					const auto idx = _provide_bb(blk).index();
					return _assembly.blocks.at(idx);
				}

				template <typename... ArgTs>
				void _emplace_instruction(ArgTs&&... args)
				{
					const auto idx = _metamap.at(_current_block).index();
					_assembly.blocks[idx].code.emplace_back(
						std::forward<ArgTs>(args)...
					);
				}

				template <typename... ArgTs>
				void _emplace_cond_branch_jump(ArgTs&&... args)
				{
					_metamap.at(_current_block).jump_on_cond_branch.emplace_back(
						std::forward<ArgTs>(args)...
					);
				}

				template <typename... ArgTs>
				void _emplace_fall_through_jump(ArgTs&&... args)
				{
					_metamap.at(_current_block).jump_on_fall_through.emplace_back(
						std::forward<ArgTs>(args)...
					);
				}

				template <typename... ArgTs>
				void _emplace_instruction_before_jmp(const bool taken,
													 firm::ir_node*const blksrc,
				                                     USELESS firm::ir_node*const blkdst,
				                                     ArgTs&&... args)
				{
					assert(firm::is_Block(blksrc));
					assert(firm::is_Block(blkdst));
					auto& srcmeta = _provide_bb(blksrc);
					if (taken) {
						assert(srcmeta.get_succ_cond_branch() == blkdst);
						srcmeta.phis_on_cond_branch.emplace_back(std::forward<ArgTs>(args)...);
					} else {
						assert(srcmeta.get_succ_fall_through() == blkdst);
						srcmeta.phis_on_fall_through.emplace_back(std::forward<ArgTs>(args)...);
					}
				}

				virtual_operand _get_irn_as_operand(firm::ir_node*const irn)
				{
					assert(can_be_in_register(irn));
					if (is_flag(irn)) {
						if (firm::is_Phi(irn)) {
							MINIJAVA_NOT_IMPLEMENTED();
						}
						return _get_flags_register(irn);
					} else if (firm::is_Const(irn)) {
						const auto tarval = firm::get_Const_tarval(irn);
						const auto number = firm::get_tarval_long(tarval);
						return virtual_operand{number};
					} else {
						return virtual_operand{_get_data_register(irn)};
					}
				}

				virtual_register _get_irn_as_register_operand(firm::ir_node*const irn)
				{
					assert(can_be_in_register(irn));
					if (is_flag(irn)) {
						if (firm::is_Phi(irn)) {
							MINIJAVA_NOT_IMPLEMENTED();
						}
						return _get_flags_register(irn);
					}
					const auto oldreg = _get_register_or_dummy(irn);
					if (oldreg != virtual_register::dummy) {
						return oldreg;
					}
					const auto value = _get_irn_as_operand(irn);
					const auto width = get_width(irn);
					const auto newreg = _next_data_register();
					_emplace_instruction(opcode::op_mov, width, value, newreg);
					_set_register(irn, newreg);
					return newreg;
				}

				void _visit_start(USELESS firm::ir_node*const irn)
				{
					assert(firm::is_Start(irn));
					const auto& label = _get_basic_block(irn).label;
					_assembly.blocks.front().code.emplace_back(
						opcode::op_jmp, bit_width{}, label
					);
				}

				void _visit_end(USELESS firm::ir_node*const irn)
				{
					assert(firm::is_End(irn));
				}

				void _visit_block(USELESS firm::ir_node*const irn)
				{
					assert(firm::is_Block(irn));
					assert(irn == _current_block);
				}

				void _visit_const(USELESS firm::ir_node*const irn)
				{
					assert(firm::is_Const(irn));
					// Flag constants must be handled at their point of use
					// because there is only one flags register.
					if (!is_flag(irn)) {
					}
				}

				void _visit_binop(firm::ir_node*const irn, const opcode binop)
				{
					assert(firm::is_binop(irn));
					assert(!firm::is_Div(irn) && !firm::is_Mod(irn));
					const auto lhs = firm::get_binop_left(irn);
					const auto rhs = firm::get_binop_right(irn);
					const auto width = get_width(irn);
					assert(get_width(lhs) == width);
					assert(get_width(rhs) == width);
					const auto dstreg = _next_data_register();
					const auto lhsval = _get_irn_as_operand(lhs);
					const auto rhsval = _get_irn_as_operand(rhs);
					_emplace_instruction(opcode::op_mov, width, lhsval, dstreg);
					_emplace_instruction(binop, width, rhsval, dstreg);
					_set_register(irn, dstreg);
				}

				void _visit_div(firm::ir_node*const irn)
				{
					assert(firm::is_Div(irn));
					const auto lhs = firm::get_Div_left(irn);
					const auto rhs = firm::get_Div_right(irn);
					const auto width = get_width(firm::get_Div_resmode(irn));
					assert(get_width(lhs) == width);
					assert(get_width(rhs) == width);
					const auto lhsval = _get_irn_as_operand(lhs);
					const auto rhsval = _get_irn_as_operand(rhs);
					const auto divreg = _next_data_register();
					_emplace_instruction(opcode::op_mov, width, rhsval, divreg);
					_emplace_instruction(opcode::mac_div, width, lhsval, divreg);
					_set_register(irn, divreg);
				}

				void _visit_mod(firm::ir_node*const irn)
				{
					assert(firm::is_Mod(irn));
					const auto lhs = firm::get_Mod_left(irn);
					const auto rhs = firm::get_Mod_right(irn);
					const auto width = get_width(firm::get_Mod_resmode(irn));
					assert(get_width(lhs) == width);
					assert(get_width(rhs) == width);
					const auto lhsval = _get_irn_as_operand(lhs);
					const auto rhsval = _get_irn_as_operand(rhs);
					const auto modreg = _next_data_register();
					_emplace_instruction(opcode::op_mov, width, rhsval, modreg);
					_emplace_instruction(opcode::mac_mod, width, lhsval, modreg);
					_set_register(irn, modreg);
				}

				void _visit_minus(firm::ir_node*const irn)
				{
					assert(firm::is_Minus(irn));
					const auto posirn = firm::get_Minus_op(irn);
					const auto posval = _get_irn_as_operand(posirn);
					const auto width = get_width(irn);
					assert(width == get_width(posirn));
					const auto negreg = _next_data_register();
					_emplace_instruction(opcode::op_mov, width, posval, negreg);
					_emplace_instruction(opcode::op_neg, width, negreg);
					_set_register(irn, negreg);
				}

				void _visit_conv(firm::ir_node*const irn)
				{
					assert(firm::is_Conv(irn));
					const auto srcirn = firm::get_irn_n(irn, 0);
					const auto srcreg = _get_irn_as_register_operand(srcirn);
					const auto srcmod = firm::get_irn_mode(srcirn);
					const auto dstmod = firm::get_irn_mode(irn);
					if (get_width(srcmod) >= get_width(dstmod)) {
						_set_register(irn, srcreg);
					} else if ((srcmod == firm::mode_Is) && (dstmod == firm::mode_Ls)) {
						const auto dstreg = _next_data_register();
						_emplace_instruction(opcode::op_movslq, bit_width{}, srcreg, dstreg);
						_set_register(irn, dstreg);
					} else {
						MINIJAVA_NOT_IMPLEMENTED();
					}
				}

				void _visit_address(firm::ir_node*const irn)
				{
					assert(firm::is_Address(irn));
					const auto entity = firm::get_Address_entity(irn);
					if (firm::is_method_entity(entity)) {
						// We never need the address of a method.
						return;
					}
					const auto pos = _addresses.find(entity);
					if (pos != _addresses.cend()) {
						_set_register(irn, pos->second);
						return;
					}
					const auto ldname = firm::get_entity_ld_name(entity);
					const auto reg = _next_data_register();
					_emplace_instruction(opcode::op_lea, bit_width{}, ldname, reg);
					_addresses[entity] = reg;
					_set_register(irn, reg);
				}

				void _visit_load(firm::ir_node*const irn)
				{
					assert(firm::is_Load(irn));
					const auto ptrirn = firm::get_Load_ptr(irn);
					const auto ptrreg = _get_irn_as_register_operand(ptrirn);
					const auto memreg = _next_data_register();
					const auto width = get_width(firm::get_Load_mode(irn));
					auto addr = virtual_address{};
					addr.base = ptrreg;
					_emplace_instruction(opcode::op_mov, width, std::move(addr), memreg);
					_set_register(irn, memreg);
				}

				void _visit_store(firm::ir_node*const irn)
				{
					assert(firm::is_Store(irn));
					const auto ptrirn = firm::get_Store_ptr(irn);
					const auto valirn = firm::get_Store_value(irn);
					const auto ptrreg = _get_data_register(ptrirn);
					const auto memval = _get_irn_as_operand(valirn);
					const auto width = get_width(valirn);
					auto addr = virtual_address{};
					addr.base = ptrreg;
					_emplace_instruction(opcode::op_mov, width, memval, std::move(addr));
				}

				void _visit_call(firm::ir_node*const irn)
				{
					assert(firm::is_Call(irn));
					const auto method_entity = firm::get_Call_callee(irn);
					const auto method_type = firm::get_entity_type(method_entity);
					const auto arg_arity = firm::get_method_n_params(method_type);
					const auto res_arity = firm::get_method_n_ress(method_type);
					assert(arg_arity <= INT_MAX);  // libfirm's randomly chosen integer types...
					auto argreg = virtual_register::argument;
					for (auto i = 0; i < static_cast<int>(arg_arity); ++i) {
						const auto node = firm::get_Call_param(irn, i);
						const auto width = get_width(node);
						const auto srcval = _get_irn_as_operand(node);
						assert(can_be_in_register(node));
						_emplace_instruction(opcode::op_mov, width, srcval, argreg);
						argreg = next_argument_register(argreg);
					}
					const auto label = firm::get_entity_ld_name(method_entity);
					_emplace_instruction(opcode::mac_call_aligned, bit_width{}, label);
					if (res_arity) {
						assert(res_arity == 1);
						const auto resreg = _next_data_register();
						const auto reswidth = get_width(firm::get_method_res_type(method_type, 0));
						_emplace_instruction(opcode::op_mov, reswidth, virtual_register::result, resreg);
						_set_register(irn, resreg);
					}
				}

				void _visit_return(firm::ir_node*const irn)
				{
					assert(firm::is_Return(irn));
					const auto arity = firm::get_Return_n_ress(irn);
					if (arity) {
						assert(arity == 1);
						const auto resirn = firm::get_Return_res(irn, 0);
						const auto resval = _get_irn_as_operand(resirn);
						const auto width = get_width(resirn);
						_emplace_fall_through_jump(
							opcode::op_mov, width, resval, virtual_register::result
						);
					}
					_emplace_fall_through_jump(opcode::op_ret);
				}

				void _visit_cmp(firm::ir_node*const irn)
				{
					assert(firm::is_Cmp(irn));
					_set_register(irn, virtual_register::flags);
				}

				void _visit_jmp(firm::ir_node*const irn)
				{
					assert(firm::is_Jmp(irn));
					assert(firm::get_irn_n_outs(irn) == 1);
					const auto targirn = firm::get_irn_out(irn, 0);
					const auto targblk = _blockmap.at(targirn);
					_emplace_fall_through_jump(
						opcode::op_jmp, bit_width{},
						_get_basic_block(targblk).label
					);
				}

				void _visit_cond(firm::ir_node*const irn)
				{
					assert(firm::is_Cond(irn));
					const auto targets = get_cond_targets(irn);
					const auto thenlab = _get_basic_block(targets.then_block).label;
					const auto elselab = _get_basic_block(targets.else_block).label;
					assert(!thenlab.empty());
					assert(!elselab.empty());
					const auto selector = firm::get_Cond_selector(irn);
					if (firm::is_Cmp(selector)) {
						const auto lhsirn = firm::get_Cmp_left(selector);
						const auto rhsirn = firm::get_Cmp_right(selector);
						const auto lhsval = _get_irn_as_register_operand(lhsirn);
						const auto rhsval = _get_irn_as_operand(rhsirn);
						const auto width = std::max(get_width(lhsirn), get_width(rhsirn));
						_emplace_cond_branch_jump(opcode::op_cmp, width, rhsval, lhsval);
						const auto relation = firm::get_Cmp_relation(selector);
						const auto jumpop = get_conditional_jump_op(relation);
						_emplace_cond_branch_jump(jumpop, bit_width{}, thenlab);
						_emplace_fall_through_jump(opcode::op_jmp, bit_width{}, elselab);
					} else if (firm::is_Const(selector)) {
						// This is actually an unconditional jump.
						const auto tarval = firm::get_Const_tarval(irn);
						const auto jumpto = !firm::tarval_is_null(tarval) ? thenlab : elselab;
						_emplace_fall_through_jump(opcode::op_jmp, bit_width{}, jumpto);
					} else if (firm::is_Phi(selector)) {
						// Restore the result of a previous compare.
						const auto memoreg = _get_data_register(selector);
						_emplace_cond_branch_jump(opcode::op_test, bit_width::viii, memoreg, memoreg);
						_emplace_cond_branch_jump(opcode::op_jnz, bit_width{}, thenlab);
						_emplace_fall_through_jump(opcode::op_jmp, bit_width{}, elselab);
					} else {
						MINIJAVA_NOT_REACHED();
					}
				}

				void _visit_phi(firm::ir_node*const irn)
				{
					assert(firm::is_Phi(irn));
					const auto phireg = _get_register_or_dummy(irn);
					if (phireg != virtual_register::dummy) {
						if (is_flag(irn)) {
							_visit_flags_phi(irn);
						} else {
							_visit_data_phi(irn);
						}
					}
				}

				template <typename OpT>
				void _visit_phi_generic(firm::ir_node*const irn, OpT&& foreach)
				{
					assert(firm::is_Phi(irn));
					const auto phiblk = _blockmap.at(irn);
					const auto arity = firm::get_Phi_n_preds(irn);
					for (auto i = 0; i < arity; ++i) {
						const auto predirn = firm::get_Phi_pred(irn, i);
						const auto cfgpred = firm::get_Block_cfgpred(phiblk, i);
						const auto predblk = firm::get_Block_cfgpred_block(phiblk, i);
						const auto taken = firm::is_Proj(cfgpred)
							? (firm::get_Proj_num(cfgpred) == firm::pn_Cond_true)
							: false;
						foreach(taken, predblk, predirn);
					}
				}

				void _visit_data_phi(firm::ir_node*const irn)
				{
					assert(firm::is_Phi(irn) && !is_flag(irn));
					const auto foreach = [irn, me = this](auto taken, auto predblk, auto predirn){
						const auto phireg = me->_get_data_register(irn);
						const auto phiblk = me->_blockmap.at(irn);
						const auto predval = me->_get_irn_as_operand(predirn);
						const auto width = get_width(irn);
						me->_emplace_instruction_before_jmp(
							taken, predblk, phiblk,
							opcode::op_mov, width, predval, phireg
						);
					};
					_visit_phi_generic(irn, foreach);
				}

				void _visit_flags_phi(firm::ir_node*const irn)
				{
					assert(firm::is_Phi(irn) && is_flag(irn));
					const auto foreach = [irn, me = this](auto taken, auto predblk, auto predirn){
						const auto phireg = me->_get_data_register(irn);
						const auto phiblk = me->_blockmap.at(irn);
						if (firm::is_Const(predirn)) {
							const auto tarval = firm::get_Const_tarval(predirn);
							const auto byte = const_mode_b_as_bool(tarval);
							me->_emplace_instruction_before_jmp(
								taken, predblk, phiblk,
								opcode::op_mov, bit_width::viii, byte, phireg
							);
						} else if (firm::is_Cmp(predirn)) {
							const auto relation = firm::get_Cmp_relation(predirn);
							const auto lhsirn = firm::get_Cmp_left(predirn);
							const auto rhsirn = firm::get_Cmp_right(predirn);
							const auto lhsreg = me->_get_irn_as_register_operand(lhsirn);
							const auto rhsval = me->_get_irn_as_operand(rhsirn);
							const auto width = std::max(get_width(lhsirn), get_width(rhsirn));
							const auto setop = get_conditional_set_op(relation);
							me->_emplace_instruction_before_jmp(
								taken, predblk, phiblk,
								opcode::op_cmp, width, rhsval, lhsreg
							);
							me->_emplace_instruction_before_jmp(
								taken, predblk, phiblk,
								setop, bit_width{}, phireg
							);
						} else {
							MINIJAVA_NOT_REACHED_MSG(firm::get_irn_opname(predirn));
						}
					};
					_visit_phi_generic(irn, foreach);
				}

				void _visit_proj(firm::ir_node* irn)
				{
					assert(firm::is_Proj(irn));
					if (can_be_in_register(irn)) {
						const auto predirn = firm::get_Proj_pred(irn);
						const auto predreg = _get_register_or_dummy(predirn);
						if (predreg != virtual_register::dummy) {
							_set_register(irn, predreg);
						}
					}
				}

				void _finalize()
				{
					for (auto&& kv : _metamap) {
						auto& meta = kv.second;
						auto& code = _assembly.blocks[meta.index()].code;
						const auto docopy = [&code](auto& vec){
							std::copy(
								std::begin(vec),
								std::end(vec),
								std::back_inserter(code)
							);
						};
						docopy(meta.phis_on_cond_branch);
						docopy(meta.jump_on_cond_branch);
						docopy(meta.phis_on_fall_through);
						docopy(meta.jump_on_fall_through);
					}
				}

			};  // class generator


			void visit_first_pass_before(firm::ir_node*const irn, void*const env)
			{
				auto genp = static_cast<generator*>(env);
				genp->visit_first_pass(irn);
			}

			void visit_first_pass_after(firm::ir_node*const /*irn*/, void*const /*env*/)
			{
			}

			void visit_second_pass(firm::ir_node*const irn, void*const env)
			{
				auto genp = static_cast<generator*>(env);
				genp->visit_second_pass(irn);
			}

			std::unique_ptr<firm::ir_graph, void(*)(firm::ir_graph*)>
			make_backedge_guard(firm::ir_graph*const irg)
			{
				const auto del = [](firm::ir_graph* p){
					firm::free_irg_outs(p);
					firm::edges_deactivate(p);
				};
				firm::edges_activate(irg);
				firm::assure_irg_outs(irg);
				return {irg, del};
			}

		}  // namespace /* anonymous */


		virtual_assembly assemble_function(firm::ir_graph*const irg)
		{
			assert(irg != nullptr);
			firm::dump_ir_graph(irg, "asm");
			const auto backedge_guard = make_backedge_guard(irg);
			const auto entity = firm::get_irg_entity(irg);
			const auto ldname = firm::get_entity_ld_name(entity);
			auto gen = generator{ldname};
			firm::irg_walk_blkwise_graph(
				irg,
				visit_first_pass_before,
				visit_first_pass_after,
				&gen
			);
			gen.handle_parameters(irg);
			firm::irg_walk_topological(
				irg,
				visit_second_pass,
				&gen
			);
			return std::move(gen).get();
		}

	}  // namespace backend

}  // namespace minijava
