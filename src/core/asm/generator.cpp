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

#include <boost/variant/apply_visitor.hpp>

#include "exceptions.hpp"


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


			void set_irn_link_reg(firm::ir_node* irn, const virtual_register reg)
			{
				const auto n = static_cast<std::uintptr_t>(reg);
				const auto p = reinterpret_cast<void*>(n);
				firm::set_irn_link(irn, p);
			}

			virtual_register get_irn_link_reg(firm::ir_node* irn)
			{
				const auto p = firm::get_irn_link(irn);
				const auto n = reinterpret_cast<std::uintptr_t>(p);
				return static_cast<virtual_register>(n);
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

				void visit_parameters(firm::ir_graph* irg)
				{
					const auto params = firm::get_irg_args(irg);
					const auto arity = firm::get_irn_arity(params);
					auto argreg = virtual_register::argument;
					for (auto i = 0; i < arity; ++i) {
						const auto param = firm::get_irn_n(params, i);
						set_irn_link_reg(param, argreg);
						argreg = next_argument_register(argreg);
					}
				}

				void visit(firm::ir_node*const irn)
				{
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
						_visit_binop(irn, opcode::mac_div);
						break;
					case firm::iro_Mod:
						_visit_binop(irn, opcode::mac_mod);
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
				}

				virtual_assembly get() &&
				{
					return std::move(_assembly);
				}

			private:

				virtual_assembly _assembly;
				virtual_register _nextreg;
				std::map<const firm::ir_node*, std::size_t> _blockmap;
				std::size_t _current_blk_idx;

				virtual_register _next_register()
				{
					const auto current = _nextreg;
					_nextreg = next_general_register(current);
					return current;
				}

				std::size_t _provide_block_idx(firm::ir_node* blk)
				{
                    assert(firm::is_Block(blk));
					const auto pos = _blockmap.find(blk);
					if (pos != _blockmap.end()) {
						return pos->second;
					}
					const auto pointer = static_cast<void*>(blk);
					const auto address = reinterpret_cast<std::uintptr_t>(pointer);
					const auto index = _assembly.blocks.size();
					_assembly.blocks.emplace_back(".L" + std::to_string(address));
					_blockmap[blk] = index;
					return index;
				}

				template <typename... ArgTs>
				void _emplace_instruction(ArgTs&&... args)
				{
					_assembly.blocks[_current_blk_idx].code.emplace_back(
						std::forward<ArgTs>(args)...
					);
				}

				template <typename... ArgTs>
				void _emplace_instruction_before_jmp(firm::ir_node* blkfrom,
				                                     firm::ir_node* blkto,
				                                     ArgTs&&... args)
				{
                    assert(firm::is_Block(blkfrom));
                    assert(firm::is_Block(blkto));
					const auto idxfrom = _provide_block_idx(blkfrom);
					const auto idxto = _provide_block_idx(blkto);
					const auto label = _assembly.blocks[idxto].label.c_str();
					auto& code = _assembly.blocks[idxfrom].code;
					const auto pos = std::find_if(
						code.crbegin(), code.crend(),
						[label](auto&& instr){
							const auto namep = get_name(instr.op1);
							return (namep != nullptr) && (*namep == label);
						}
					);
					const auto fwdpos = code.cbegin() + (code.crend() - pos - 1);
					assert(*get_name(fwdpos->op1) == label);
					code.emplace(fwdpos, std::forward<ArgTs>(args)...);
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
					_current_blk_idx = _provide_block_idx(irn);
				}

				void _visit_const(firm::ir_node* irn)
				{
					assert(firm::is_Const(irn));
					const auto width = get_width(irn);
					const auto tarval = firm::get_Const_tarval(irn);
					const auto number = firm::get_tarval_long(tarval);
					const auto dstreg = _next_register();
					set_irn_link_reg(irn, dstreg);
					_emplace_instruction(opcode::op_mov, width, number, dstreg);
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
					const auto lhsreg = get_irn_link_reg(lhs);
					const auto rhsreg = get_irn_link_reg(rhs);
					set_irn_link_reg(irn, dstreg);
					_emplace_instruction(opcode::op_mov, width, lhsreg, dstreg);
					_emplace_instruction(binop, width, rhsreg, dstreg);
				}

				void _visit_minus(firm::ir_node* irn)
				{
					assert(firm::is_Minus(irn));
					const auto opirn = firm::get_Minus_op(irn);
					const auto opreg = get_irn_link_reg(opirn);
					const auto width = get_width(irn);
					assert(width == get_width(opirn));
					const auto reg = _next_register();
					_emplace_instruction(opcode::op_mov, width, opreg, reg);
					_emplace_instruction(opcode::op_neg, width, reg);
					set_irn_link_reg(irn, reg);
				}

				void _visit_conv(firm::ir_node* irn)
				{
					assert(firm::is_Conv(irn));
					const auto srcirn = firm::get_irn_n(irn, 0);
					const auto srcreg = get_irn_link_reg(srcirn);
					const auto srcmod = firm::get_irn_mode(srcirn);
					const auto dstmod = firm::get_irn_mode(irn);
					if (srcmod == dstmod) {
						set_irn_link_reg(irn, srcreg);
					} else if ((srcmod == firm::mode_Is) && (dstmod == firm::mode_Ls)) {
						const auto dstreg = _next_register();
						_emplace_instruction(opcode::op_movslq, bit_width{}, srcreg, dstreg);
						set_irn_link_reg(irn, dstreg);
					} else if ((srcmod == firm::mode_Ls) && (dstmod == firm::mode_Is)) {
						set_irn_link_reg(irn, srcreg);
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
					set_irn_link_reg(irn, reg);
				}

				void _visit_load(firm::ir_node* irn)
				{
                    assert(firm::is_Load(irn));
					const auto ptrirn = firm::get_Load_ptr(irn);
					const auto ptrreg = get_irn_link_reg(ptrirn);
					const auto valreg = _next_register();
					const auto width = get_width(irn);
					auto addr = virtual_address{};
					addr.base = ptrreg;
					_emplace_instruction(opcode::op_mov, width, std::move(addr), valreg);
					set_irn_link_reg(irn, valreg);
				}

				void _visit_store(firm::ir_node* irn)
				{
                    assert(firm::is_Store(irn));
					const auto ptrirn = firm::get_Store_ptr(irn);
					const auto valirn = firm::get_Store_value(irn);
					const auto ptrreg = get_irn_link_reg(ptrirn);
					const auto valreg = get_irn_link_reg(valirn);
					const auto width = get_width(valirn);
					auto addr = virtual_address{};
					addr.base = ptrreg;
					_emplace_instruction(opcode::op_mov, width, valreg, std::move(addr));
				}

				void _visit_call(firm::ir_node* irn)
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
						const auto srcreg = get_irn_link_reg(node);
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
					}
				}

				void _visit_return(firm::ir_node* irn)
				{
					assert(firm::is_Return(irn));
					const auto arity = firm::get_Return_n_ress(irn);
					if (arity) {
						assert(arity == 1);
						const auto resirn = firm::get_Return_res(irn, 0);
						const auto resreg = get_irn_link_reg(resirn);
						const auto width = get_width(resirn);
						_emplace_instruction(opcode::op_mov, width, resreg, virtual_register::result);
						fprintf(stderr, "%s returns %s\n", _assembly.ldname.c_str(), firm::get_irn_opname(resirn));
					}
					_emplace_instruction(opcode::op_ret);
				}

				void _visit_cmp(firm::ir_node* irn)
				{
                    assert(firm::is_Cmp(irn));
					const auto lhsirn = firm::get_Cmp_left(irn);
					const auto rhsirn = firm::get_Cmp_right(irn);
					const auto lhsreg = get_irn_link_reg(lhsirn);
					const auto rhsreg = get_irn_link_reg(rhsirn);
					const auto width = std::max(get_width(lhsirn), get_width(rhsirn));
					const auto resreg = _next_register();
					const auto setcc = get_cmp_set_cc(firm::get_Cmp_relation(irn));
					_emplace_instruction(opcode::op_cmp, width, lhsreg, rhsreg);
					_emplace_instruction(setcc, bit_width{}, resreg);
					set_irn_link_reg(irn, resreg);
				}

				void _visit_jmp(firm::ir_node* irn)
				{
                    assert(firm::is_Jmp(irn));
					assert(firm::get_irn_n_outs(irn) == 1);
					const auto targirn = firm::get_irn_out(irn, 0);
					const auto targidx = _provide_block_idx(targirn);
					_emplace_instruction(
						opcode::op_jmp, bit_width{},
						_assembly.blocks[targidx].label
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
					const auto thenidx = _provide_block_idx(thenirn);
					const auto elseidx = _provide_block_idx(elseirn);
					const auto condreg = get_irn_link_reg(condirn);
					_emplace_instruction(opcode::op_cmp, get_width(condirn), 0, condreg);
					_emplace_instruction(opcode::op_je, bit_width{}, _assembly.blocks[elseidx].label);
					_emplace_instruction(opcode::op_jmp, bit_width{}, _assembly.blocks[thenidx].label);
				}

				void _visit_phi(firm::ir_node* irn)
				{
                    assert(firm::is_Phi(irn));
					const auto phiblk = firm::get_nodes_block(irn);
					const auto phireg = _next_register();
					const auto arity = firm::get_Phi_n_preds(irn);
					const auto width = get_width(irn);
					for (auto i = 0; i < arity; ++i) {
						const auto predirn = firm::get_Phi_pred(irn, i);
						const auto predreg = get_irn_link_reg(predirn);
						const auto predblk = firm::get_Block_cfgpred_block(phiblk, i);
						_emplace_instruction_before_jmp(predblk, phiblk, opcode::op_mov, width, predreg, phireg);
					}
					set_irn_link_reg(irn, phireg);
				}

				void _visit_proj(firm::ir_node* irn)
				{
                    assert(firm::is_Proj(irn));
					const auto predirn = firm::get_Proj_pred(irn);
					const auto predreg = get_irn_link_reg(predirn);
					set_irn_link_reg(irn, predreg);
				}

			};  // class generator


			void visit_node_before(firm::ir_node* /*irn*/, void* /*env*/)
			{
				// Nothing to do?
			}

			void visit_node_after(firm::ir_node* irn, void* env)
			{
				auto genp = static_cast<generator*>(env);
				genp->visit(irn);
			}

			class ir_resource_guard final
			{
			public:

				ir_resource_guard(firm::ir_graph* irg, firm::ir_resources_t res)
					: _irg{irg}, _res{res}
				{
					firm::ir_reserve_resources(_irg, _res);
				}

				~ir_resource_guard()
				{
					firm::ir_free_resources(_irg, _res);
				}

				ir_resource_guard(const ir_resource_guard&) = delete;
				ir_resource_guard& operator=(const ir_resource_guard&) = delete;
				ir_resource_guard(ir_resource_guard&&) = delete;
				ir_resource_guard& operator=(ir_resource_guard&&) = delete;

			private:

				firm::ir_graph* _irg;
				firm::ir_resources_t _res;
			};

			std::unique_ptr<firm::ir_graph, void(*)(firm::ir_graph*)>
			make_backedge_guard(firm::ir_graph* irg)
			{
				firm::assure_irg_outs(irg);
				return {irg, &firm::free_irg_outs};
			}

		}  // namespace /* anonymous */

		virtual_assembly assemble_function(firm::ir_graph* irg)
		{
			assert(irg != nullptr);
			const ir_resource_guard resource_guard {irg, firm::IR_RESOURCE_IRN_LINK};
			const auto backedge_guard = make_backedge_guard(irg);
			const auto entity = firm::get_irg_entity(irg);
			const auto ldname = firm::get_entity_ld_name(entity);
			auto gen = generator{ldname};
			gen.visit_parameters(irg);
			firm::irg_walk_blkwise_graph(
				irg,
				visit_node_before,
				visit_node_after,
				&gen
			);
			return std::move(gen).get();
		}

	}  // namespace backend

}  // namespace minijava
