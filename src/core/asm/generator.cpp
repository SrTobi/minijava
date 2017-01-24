#include "asm/generator.hpp"

#include <cassert>
#include <climits>
#include <cstdint>
#include <cstdio>

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

			class generator final
			{
			public:

				generator(virtual_assembly& va)
					: _virtasm{&va}
					, _nextreg{virtual_register::general}
				{
				}

				void visit(firm::ir_node*const irn)
				{
					switch (firm::get_irn_opcode(irn)) {
					case firm::iro_Start:
						_visit_start(irn);
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
					case firm::iro_Address:
						_visit_address(irn);
						break;
					case firm::iro_Call:
						_visit_call(irn);
						break;
					case firm::iro_Cmp:
						break;
					case firm::iro_Cond:
						break;
					case firm::iro_Conv:
						break;
					case firm::iro_End:
						break;
					case firm::iro_Jmp:
						break;
					case firm::iro_Load:
						break;
					case firm::iro_Member:
						break;
					case firm::iro_Minus:
						break;
					case firm::iro_Mux:
						break;
					case firm::iro_Phi:
						break;
					case firm::iro_Proj:
						break;
					case firm::iro_Return:
						_visit_return(irn);
						break;
					case firm::iro_Sel:
						break;
					case firm::iro_Store:
						break;
					default:
						MINIJAVA_THROW_ICE_MSG(
							internal_compiler_error,
							firm::get_irn_opname(irn)
						);
					}
				}

			private:

				virtual_assembly* _virtasm;
				virtual_register _nextreg;

				virtual_assembly& _assembly() noexcept
				{
					return *_virtasm;
				}

				virtual_register _next_register()
				{
					const auto current = _nextreg;
					_nextreg = next(current);
					return current;
				}

				void _visit_block(firm::ir_node* irn)
				{
					// TODO: Generate unique labels.
                    assert(firm::is_Block(irn));
					_assembly().emplace_back();
					_assembly().back().label = ".L";
				}

				void _visit_start(firm::ir_node* irn)
				{
                    assert(firm::is_Start(irn));
					// Nothing to do?
				}

				void _visit_const(firm::ir_node* irn)
				{
					assert(firm::is_Const(irn));
					const auto width = get_width(irn);
					const auto tarval = firm::get_Const_tarval(irn);
					const auto number = firm::get_tarval_long(tarval);
					const auto dstreg = _next_register();
					set_irn_link_reg(irn, dstreg);
					_assembly().emplace_back(opcode::op_mov, width, number, dstreg);
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
					_assembly().emplace_back(opcode::op_mov, width, lhsreg, dstreg);
					_assembly().emplace_back(binop, width, rhsreg, dstreg);
				}

				void _visit_div(firm::ir_node* irn)
				{
                    assert(firm::is_Div(irn));
					const auto lhs = firm::get_binop_left(irn);
					const auto rhs = firm::get_binop_right(irn);
					const auto width = get_width(firm::get_Div_resmode(irn));
					assert(get_width(lhs) == width);
					assert(get_width(rhs) == width);
					const auto lhsreg = get_irn_link_reg(lhs);
					const auto rhsreg = get_irn_link_reg(rhs);
					const auto divreg = _next_register();
					const auto modreg = _next_register();
					_assembly().emplace_back(opcode::op_mov, width, lhsreg, divreg);
					_assembly().emplace_back(opcode::op_mov, width, rhsreg, modreg);
					_assembly().emplace_back(opcode::mac_divmod, width, divreg, modreg);
					set_irn_link_reg(irn, divreg);
				}

				void _visit_mod(firm::ir_node* irn)
				{
                    assert(firm::is_Mod(irn));
					const auto lhs = firm::get_binop_left(irn);
					const auto rhs = firm::get_binop_right(irn);
					const auto width = get_width(firm::get_Mod_resmode(irn));
					assert(get_width(lhs) == width);
					assert(get_width(rhs) == width);
					const auto lhsreg = get_irn_link_reg(lhs);
					const auto rhsreg = get_irn_link_reg(rhs);
					const auto divreg = _next_register();
					const auto modreg = _next_register();
					_assembly().emplace_back(opcode::op_mov, width, lhsreg, divreg);
					_assembly().emplace_back(opcode::op_mov, width, rhsreg, modreg);
					_assembly().emplace_back(opcode::mac_divmod, width, divreg, modreg);
					set_irn_link_reg(irn, modreg);
				}

				void _visit_address(firm::ir_node* irn)
				{
                    assert(firm::is_Address(irn));
					// TODO: What now?
				}

				void _visit_call(firm::ir_node* irn)
				{
					assert(firm::is_Call(irn));
					const auto method_entity = firm::get_Call_callee(irn);
					const auto method_type = firm::get_entity_type(method_entity);
					const auto arg_arity = firm::get_method_n_params(method_type);
					const auto res_arity = firm::get_method_n_ress(method_type);
					assert(arg_arity <= INT_MAX);  // libfirm's randomly chosen integer types...
					for (auto i = 0; i < static_cast<int>(arg_arity); ++i) {
						const auto node = firm::get_Call_param(irn, i);
						const auto width = get_width(node);
						const auto srcreg = get_irn_link_reg(node);
						const auto dstreg = static_cast<virtual_register>(-1 - i);  // TODO: Use proper functions
						_assembly().emplace_back(opcode::op_mov, width, srcreg, dstreg);
					}
					const auto label = firm::get_entity_ld_name(method_entity);
					_assembly().emplace_back(opcode::mac_call_aligned, bit_width{}, label);
					if (res_arity) {
						assert(res_arity == 1);
						const auto resreg = _next_register();
						const auto reswidth = get_width(firm::get_method_res_type(method_type, 0));
						_assembly().emplace_back(opcode::op_mov, reswidth, virtual_register::result, resreg);
					}
				}

				void _visit_return(firm::ir_node* irn)
				{
					assert(firm::is_Return(irn));
					const auto arity = firm::get_Return_n_ress(irn);
					if (arity) {
						assert(arity == 1);
						const auto resarg = firm::get_Return_res(irn, 0);
						const auto resreg = get_irn_link_reg(resarg);
						const auto width = get_width(resarg);
						_assembly().emplace_back(opcode::op_mov, width, resreg, virtual_register::result);
					}
					_assembly().emplace_back(opcode::op_ret);
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

		}  // namespace /* anonymous */

		void assemble_function(firm::ir_graph* irg, virtual_assembly& virtasm)
		{
			assert(irg != nullptr);
			const ir_resource_guard guard {irg, firm::IR_RESOURCE_IRN_LINK};
			auto gen = generator{virtasm};
			const auto entity = firm::get_irg_entity(irg);
			const auto ldname = firm::get_entity_ld_name(entity);
			virtasm.emplace_back();
			virtasm.back().label = ldname;
			// TODO: Assign parameters to registers.
			firm::irg_walk_blkwise_graph(
				irg,
				visit_node_before,
				visit_node_after,
				&gen
			);
		}

	}  // namespace backend

}  // namespace minijava
