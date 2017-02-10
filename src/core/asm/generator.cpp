#include "asm/generator.hpp"

#include <cassert>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <boost/variant/apply_visitor.hpp>

#include "exceptions.hpp"


extern "C" void set_break_point_here(){}

namespace minijava
{

	namespace backend
	{

		namespace /* anonymous */
		{

			bit_width get_width(const firm::ir_mode* irm)
			{
				const auto bits = firm::get_mode_size_bits(irm);
				return static_cast<bit_width>(bits);
			}

			bit_width get_width(const firm::ir_node* irn)
			{
				return get_width(firm::get_irn_mode(irn));
			}

			bit_width get_width(const firm::ir_type* irt)
			{
				return get_width(firm::get_type_mode(irt));
			}

			opcode get_cmp_set_cc(const firm::ir_relation rel)
			{
				switch (rel) {
				case firm::ir_relation_greater:
					return opcode::op_seta;
				case firm::ir_relation_greater_equal:
					return opcode::op_setae;
				case firm::ir_relation_less:
					return opcode::op_setb;
				case firm::ir_relation_less_equal:
					return opcode::op_setbe;
				case firm::ir_relation_equal:
					return opcode::op_sete;
				case firm::ir_relation_less_greater:
					return opcode::op_setne;
				default:
					MINIJAVA_NOT_IMPLEMENTED();
				}
			}

			class generator final
			{
			public:

				generator(const char* funcname)
					: _assembly{funcname}
					, _nextreg{virtual_register::general}
				{
				}

				void handle_parameters(firm::ir_graph* irg)
				{
					const auto entity = firm::get_irg_entity(irg);
					const auto type = firm::get_entity_type(entity);
					const auto arity = firm::get_method_n_params(type);
					const auto start = firm::get_irg_start(irg);
					switch (const auto n = firm::get_irn_n_outs(start)) {
					case 1: return;
					case 2: break;
					default: MINIJAVA_NOT_REACHED_MSG(std::to_string(n));
					}
					const auto argv = firm::get_irn_out(start, 1);
					const auto argc = firm::get_irn_n_outs(argv);
					// const auto ldname = firm::get_entity_ld_name(entity);
					// std::fprintf(stderr, "%s (%zu parameters, %u used):\n", ldname, arity, argc);
					assert(argc <= arity);
					auto argument_nodes = std::vector<firm::ir_node*>(arity);
					for (auto i = 0u; i < argc; ++i) {
						const auto irn = firm::get_irn_out(argv, i);
						// TODO: How can this be not a Proj node?
						if (firm::is_Proj(irn)) {
							const auto idx = firm::get_Proj_num(irn);
							assert(idx < arity);
							argument_nodes[idx] = irn;
						}
					}
					auto argreg = virtual_register::argument;
					for (auto irn : argument_nodes) {
						if (irn != nullptr) {
							_set_register(irn, argreg);
						}
						argreg = next_argument_register(argreg);
					}
				}

				void visit_first_pass(firm::ir_node*const irn)
				{
					using namespace std::string_literals;
					if (firm::is_Block(irn)) {
						_current_block = irn;
						const auto idx = _assembly.blocks.size();
						auto label = ".L"s + _assembly.ldname + "." + std::to_string(idx);
						_assembly.blocks.emplace_back(std::move(label));
						_indexmap[irn] = idx;
					} else if (firm::is_Phi(irn)) {
						// Phi nodes are used before visited so we must
						// allocate a register now.
						const auto reg = _next_register();
						_set_register(irn, reg);
					}
					_blockmap[irn] = _current_block;
				}

				void visit_second_pass(firm::ir_node*const irn)
				{
					_current_block = _blockmap.at(irn);
					// std::fprintf(
					//  stderr,
					//  "%10lu %p %10s  -->  ",
					//  firm::get_irn_node_nr(irn),
					//  static_cast<void*>(irn),
					//  firm::get_irn_opname(irn)
					// );
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
						_visit_binop(irn, opcode::op_mul);
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
						MINIJAVA_THROW_ICE_MSG(
							internal_compiler_error,
							firm::get_irn_opname(irn)
						);
					}
					// std::fprintf(stderr, "%4d\n", static_cast<int>(_get_register(irn, true)));
				}

				virtual_assembly get() &&
				{
					return std::move(_assembly);
				}

			private:

				std::map<firm::ir_node*, firm::ir_node*> _blockmap{};
				std::map<firm::ir_node*, std::size_t> _indexmap{};
				std::map<firm::ir_node*, virtual_register> _registers{};
				firm::ir_node* _current_block{};
				virtual_assembly _assembly;
				virtual_register _nextreg;

				void _set_register(firm::ir_node* irn, const virtual_register reg)
				{
					assert(reg != virtual_register::dummy);
					const auto info = _registers.insert({irn, reg});
					if (!info.second) {
						MINIJAVA_THROW_ICE(internal_compiler_error);
					}
				}

				virtual_register _get_register(firm::ir_node* irn, const bool dummyok = false) const
				{
					const auto pos = _registers.find(irn);
					if (pos == _registers.cend()) {
						if (dummyok) {
							return virtual_register::dummy;
						}
						// std::fprintf(
						//  stderr,
						//  "%s node %lu (%p) has dummy register!\n",
						//  firm::get_irn_opname(irn),
						//  firm::get_irn_node_nr(irn),
						//  static_cast<void*>(irn)
						// );
						MINIJAVA_THROW_ICE(internal_compiler_error);
					}
					return pos->second;
				}

				virtual_register _next_register()
				{
					const auto current = _nextreg;
					_nextreg = next_general_register(current);
					return current;
				}

				virtual_basic_block& _get_basic_block(firm::ir_node* irn)
				{
					const auto blk = _blockmap.at(irn);
					const auto idx = _indexmap.at(blk);
					return _assembly.blocks.at(idx);
				}

				template <typename... ArgTs>
				void _emplace_instruction(ArgTs&&... args)
				{
					const auto idx = _indexmap.at(_current_block);
					_assembly.blocks[idx].code.emplace_back(
						std::forward<ArgTs>(args)...
					);
				}

				template <typename... ArgTs>
				void _emplace_instruction_before_jmp(firm::ir_node* blksrc,
				                                     firm::ir_node* blkdst,
				                                     ArgTs&&... args)
				{
					assert(blksrc != nullptr);
					assert(blkdst != nullptr);
					assert(firm::is_Block(blksrc));
					assert(firm::is_Block(blkdst));
					auto& bbsrc = _get_basic_block(blksrc);
					auto& bbdst = _get_basic_block(blkdst);
					const auto label = bbdst.label.c_str();
					const auto pos = std::find_if(
						bbsrc.code.crbegin(), bbsrc.code.crend(),
						[label](auto&& instr){
							const auto namep = get_name(instr.op1);
							return (namep != nullptr) && (*namep == label);
						}
					);
					if (pos != bbsrc.code.crend()) {
						const auto fwdpos = bbsrc.code.cbegin() + (bbsrc.code.crend() - pos - 1);
						assert(*get_name(fwdpos->op1) == label);
						bbsrc.code.emplace(fwdpos, std::forward<ArgTs>(args)...);
					} else {
						bbsrc.code.emplace_back(std::forward<ArgTs>(args)...);
					}
				}

				void _visit_start(firm::ir_node* irn)
				{
                    assert(firm::is_Start(irn));
				}

				void _visit_end(firm::ir_node* irn)
				{
                    assert(firm::is_End(irn));
				}

				void _visit_block(firm::ir_node* irn)
				{
                    assert(firm::is_Block(irn));
				}

				void _visit_const(firm::ir_node* irn)
				{
					assert(firm::is_Const(irn));
					const auto width = get_width(irn);
					const auto tarval = firm::get_Const_tarval(irn);
					const auto number = firm::get_tarval_long(tarval);
					const auto dstreg = _next_register();
					_emplace_instruction(opcode::op_mov, width, number, dstreg);
					_set_register(irn, dstreg);
				}

				void _visit_binop(firm::ir_node* irn, const opcode binop)
				{
                    assert(firm::is_binop(irn));
                    assert(!firm::is_Div(irn) && !firm::is_Mod(irn));
					const auto lhs = firm::get_binop_left(irn);
					const auto rhs = firm::get_binop_right(irn);
					const auto width = get_width(irn);
					assert(get_width(lhs) == width);
					assert(get_width(rhs) == width);
					const auto dstreg = _next_register();
					const auto lhsreg = _get_register(lhs);
					const auto rhsreg = _get_register(rhs);
					_emplace_instruction(opcode::op_mov, width, lhsreg, dstreg);
					_emplace_instruction(binop, width, rhsreg, dstreg);
					_set_register(irn, dstreg);
				}

				void _visit_div(firm::ir_node* irn)
				{
					assert(firm::is_Div(irn));
					const auto lhs = firm::get_binop_left(irn);
					const auto rhs = firm::get_binop_right(irn);
					const auto width = get_width(firm::get_Div_resmode(irn));
					assert(get_width(lhs) == width);
					assert(get_width(rhs) == width);
					const auto lhsreg = _get_register(lhs);
					const auto rhsreg = _get_register(rhs);
					const auto divreg = _next_register();
					_emplace_instruction(opcode::op_mov, width, rhsreg, divreg);
					_emplace_instruction(opcode::mac_div, width, lhsreg, divreg);
					_set_register(irn, divreg);
				}

				void _visit_mod(firm::ir_node* irn)
				{
					assert(firm::is_Mod(irn));
					const auto lhs = firm::get_binop_left(irn);
					const auto rhs = firm::get_binop_right(irn);
					const auto width = get_width(firm::get_Mod_resmode(irn));
					assert(get_width(lhs) == width);
					assert(get_width(rhs) == width);
					const auto lhsreg = _get_register(lhs);
					const auto rhsreg = _get_register(rhs);
					const auto modreg = _next_register();
					_emplace_instruction(opcode::op_mov, width, rhsreg, modreg);
					_emplace_instruction(opcode::mac_mod, width, lhsreg, modreg);
					_set_register(irn, modreg);
				}

				void _visit_minus(firm::ir_node* irn)
				{
					assert(firm::is_Minus(irn));
					const auto opirn = firm::get_Minus_op(irn);
					const auto opreg = _get_register(opirn);
					const auto width = get_width(irn);
					assert(width == get_width(opirn));
					const auto reg = _next_register();
					_emplace_instruction(opcode::op_mov, width, opreg, reg);
					_emplace_instruction(opcode::op_neg, width, reg);
					_set_register(irn, reg);
				}

				void _visit_conv(firm::ir_node* irn)
				{
					assert(firm::is_Conv(irn));
					const auto srcirn = firm::get_irn_n(irn, 0);
					const auto srcreg = _get_register(srcirn);
					const auto srcmod = firm::get_irn_mode(srcirn);
					const auto dstmod = firm::get_irn_mode(irn);
					if (srcmod == dstmod) {
						_set_register(irn, srcreg);
					} else if ((srcmod == firm::mode_Is) && (dstmod == firm::mode_Ls)) {
						const auto dstreg = _next_register();
						_emplace_instruction(opcode::op_movslq, bit_width{}, srcreg, dstreg);
						_set_register(irn, dstreg);
					} else if ((srcmod == firm::mode_Ls) && (dstmod == firm::mode_Is)) {
						_set_register(irn, srcreg);
					} else {
						MINIJAVA_NOT_IMPLEMENTED();
					}
				}

				void _visit_address(firm::ir_node* irn)
				{
                    assert(firm::is_Address(irn));
					const auto entity = firm::get_Address_entity(irn);
					const auto ldname = firm::get_entity_ld_name(entity);
					const auto reg = _next_register();
					_emplace_instruction(opcode::op_lea, bit_width{}, ldname, reg);
					_set_register(irn, reg);
				}

				void _visit_load(firm::ir_node* irn)
				{
                    assert(firm::is_Load(irn));
					const auto ptrirn = firm::get_Load_ptr(irn);
					const auto ptrreg = _get_register(ptrirn);
					const auto valreg = _next_register();
					const auto width = get_width(firm::get_Load_mode(irn));
					auto addr = virtual_address{};
					addr.base = ptrreg;
					_emplace_instruction(opcode::op_mov, width, std::move(addr), valreg);
					_set_register(irn, valreg);
				}

				void _visit_store(firm::ir_node* irn)
				{
                    assert(firm::is_Store(irn));
					const auto ptrirn = firm::get_Store_ptr(irn);
					const auto valirn = firm::get_Store_value(irn);
					const auto ptrreg = _get_register(ptrirn);
					const auto valreg = _get_register(valirn);
					const auto width = get_width(valirn);
					auto addr = virtual_address{};
					addr.base = ptrreg;
					_emplace_instruction(opcode::op_mov, width, valreg, std::move(addr));
				}

				void _visit_call(firm::ir_node* irn)
				{
					assert(firm::is_Call(irn));
					const auto method_entity = firm::get_Call_callee(irn);
					//std::fprintf(stderr, "Visiting %s node for %s\n", firm::get_irn_opname(irn), firm::get_entity_ld_name(method_entity));
					const auto method_type = firm::get_entity_type(method_entity);
					const auto arg_arity = firm::get_method_n_params(method_type);
					const auto res_arity = firm::get_method_n_ress(method_type);
					assert(arg_arity <= INT_MAX);  // libfirm's randomly chosen integer types...
					auto argreg = virtual_register::argument;
					for (auto i = 0; i < static_cast<int>(arg_arity); ++i) {
						const auto node = firm::get_Call_param(irn, i);
						//std::fprintf(stderr, "\tHandling argument #%d (%s) %p\n", i, firm::get_irn_opname(node), (void*)node);
						const auto width = get_width(node);
						const auto srcreg = _get_register(node);
						_emplace_instruction(opcode::op_mov, width, srcreg, argreg);
						argreg = next_argument_register(argreg);
					}
					const auto label = firm::get_entity_ld_name(method_entity);
					_emplace_instruction(opcode::mac_call_aligned, bit_width{}, label);
					if (res_arity) {
						assert(res_arity == 1);
						const auto resreg = _next_register();
						const auto reswidth = get_width(firm::get_method_res_type(method_type, 0));
						_emplace_instruction(opcode::op_mov, reswidth, virtual_register::result, resreg);
						_set_register(irn, resreg);
					}
				}

				void _visit_return(firm::ir_node* irn)
				{
					assert(firm::is_Return(irn));
					const auto arity = firm::get_Return_n_ress(irn);
					if (arity) {
						assert(arity == 1);
						const auto resirn = firm::get_Return_res(irn, 0);
						const auto resreg = _get_register(resirn);
						const auto width = get_width(resirn);
						_emplace_instruction(opcode::op_mov, width, resreg, virtual_register::result);
						_set_register(irn, resreg);  // ok?
					}
					_emplace_instruction(opcode::op_ret);
				}

				void _visit_cmp(firm::ir_node* irn)
				{
                    assert(firm::is_Cmp(irn));
					const auto lhsirn = firm::get_Cmp_left(irn);
					const auto rhsirn = firm::get_Cmp_right(irn);
					const auto lhsreg = _get_register(lhsirn);
					const auto rhsreg = _get_register(rhsirn);
					const auto width = std::max(get_width(lhsirn), get_width(rhsirn));
					const auto resreg = _next_register();
					const auto setcc = get_cmp_set_cc(firm::get_Cmp_relation(irn));
					_emplace_instruction(opcode::op_cmp, width, lhsreg, rhsreg);
					_emplace_instruction(setcc, bit_width{}, resreg);
					_set_register(irn, resreg);
				}

				void _visit_jmp(firm::ir_node* irn)
				{
                    assert(firm::is_Jmp(irn));
					assert(firm::get_irn_n_outs(irn) == 1);
					const auto targirn = firm::get_irn_out(irn, 0);
					const auto targblk = _blockmap.at(targirn);
					_emplace_instruction(
						opcode::op_jmp, bit_width{},
						_get_basic_block(targblk).label
					);
				}

				void _visit_cond(firm::ir_node* irn)
				{
                    assert(firm::is_Cond(irn));
					assert(firm::get_irn_n_outs(irn) == 2);
					const auto condirn = firm::get_Cond_selector(irn);
					const auto thenproj = firm::get_irn_out(irn, firm::pn_Cond_true);
					const auto elseproj = firm::get_irn_out(irn, firm::pn_Cond_false);
					assert(firm::is_Proj(thenproj));
					assert(firm::get_irn_mode(thenproj) == firm::get_modeX());
					assert(firm::is_Proj(elseproj));
					assert(firm::get_irn_mode(elseproj) == firm::get_modeX());
					const auto thenirn = firm::get_irn_out(thenproj, 0);
					const auto elseirn = firm::get_irn_out(elseproj, 0);
					assert(firm::is_Block(thenirn));
					assert(firm::is_Block(elseirn));
					const auto condreg = _get_register(condirn);
					_emplace_instruction(opcode::op_cmp, get_width(condirn), 0, condreg);
					_emplace_instruction(opcode::op_je, bit_width{}, _get_basic_block(thenirn).label);
					_emplace_instruction(opcode::op_jmp, bit_width{}, _get_basic_block(elseirn).label);
				}

				void _visit_phi(firm::ir_node* irn)
				{
					set_break_point_here();
					assert(firm::is_Phi(irn));
					const auto phiblk = _blockmap.at(irn);
					const auto phireg = _get_register(irn);
					const auto arity = firm::get_Phi_n_preds(irn);
					const auto width = get_width(irn);
					//std::fprintf(stderr, "Assiging register %d to Phi node %p\n", number(phireg), (void*)irn);
					for (auto i = 0; i < arity; ++i) {
						const auto predirn = firm::get_Phi_pred(irn, i);
						const auto predblk = _blockmap.at(predirn);
						const auto predreg = _get_register(predirn, true);
						if (predreg != virtual_register::dummy) {
							_emplace_instruction_before_jmp(predblk, phiblk, opcode::op_mov, width, predreg, phireg);
						}
						//std::fprintf(stderr, "\tValue %d comes from register %d\n", i, number(predreg));
					}
				}

				void _visit_proj(firm::ir_node* irn)
				{
                    assert(firm::is_Proj(irn));
					const auto predirn = firm::get_Proj_pred(irn);
					const auto predreg = _get_register(predirn, true);
					if (predreg != virtual_register::dummy) {
						_set_register(irn, predreg);
					}
				}

			};  // class generator


			void visit_first_pass_before(firm::ir_node* irn, void* env)
			{
				auto genp = static_cast<generator*>(env);
				genp->visit_first_pass(irn);
			}

			void visit_first_pass_after(firm::ir_node* /*irn*/, void* /*env*/)
			{
			}

			void visit_second_pass(firm::ir_node* irn, void* env)
			{
				auto genp = static_cast<generator*>(env);
				genp->visit_second_pass(irn);
			}

			std::unique_ptr<firm::ir_graph, void(*)(firm::ir_graph*)>
			make_backedge_guard(firm::ir_graph* irg)
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

		virtual_assembly assemble_function(firm::ir_graph* irg)
		{
			assert(irg != nullptr);
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
