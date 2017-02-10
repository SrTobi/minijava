#include <firm.hpp>
#include "opt/tailrec.hpp"

using namespace minijava::opt;


namespace /* anonymous */
{

	struct tailrec_env {
		std::vector<firm::ir_node *> params;
		std::vector<firm::ir_node*> nodes_in_startblock;
	};

	std::vector<firm::ir_node*> find_tail_recursion(firm::ir_graph *irg)
	{
		auto end_block = firm::get_irg_end_block(irg);
		auto returns = std::vector<firm::ir_node*>();
		for (int i = 0, n = firm::get_irn_arity(end_block); i < n; i++) {
			auto ret = firm::get_irn_n(end_block, i);
			// shouldn't happen.. just skip
			if (!firm::is_Return(ret)) continue;

			auto mem = firm::get_Return_mem(ret);
			if (!firm::is_Proj(mem)) continue;
			auto call = firm::get_Proj_pred(mem);
			if (!firm::is_Call(call)) continue;
			// at this point, we found an call node as return value
			// check, if we call us and we are in the same node
			if (firm::get_nodes_block(ret) != firm::get_nodes_block(call)) continue;
			auto callee = firm::get_Call_callee(call);
			if (callee == nullptr || firm::get_entity_linktime_irg(callee) != irg) continue;

			// we only allow tailrec, if there is max 1 return value and this value
			// have to be connected directly to the call node(with projs)
			auto n_ret = firm::get_Return_n_ress(ret);
			if (n_ret > 1) continue;
			else if (n_ret == 1) {
				auto pred = firm::get_Return_res(ret, 0);
				if (!firm::is_Proj(pred)) continue;
				auto res_pred = firm::get_Proj_pred(pred);
				if (!firm::is_Proj(res_pred) || firm::get_Proj_pred(res_pred) != call) continue;
			}

			// found tailrec -> replace it
			returns.push_back(ret);
			// store a link to the call in the return node
			firm::set_irn_link(ret, call);
		}
		return returns;
	}

	firm::ir_node* fix_phi_mem(firm::ir_node *phi_mem, firm::ir_node *start_block)
	{
		auto irg = firm::get_irn_irg(phi_mem);
		auto mem = firm::get_irg_initial_mem(irg);
		auto phi_ins = std::vector<firm::ir_node*>();
		if (mem == phi_mem) {
			phi_ins.push_back(mem);
			auto arity = firm::get_irn_arity(phi_mem);
			for (int i = 0; i < arity; i++) {
				phi_ins.push_back(firm::new_r_Dummy(irg, firm::mode_M));
			}
			phi_mem = firm::new_r_Phi_loop(start_block, static_cast<int>(phi_ins.size()), phi_ins.data());
			for (int i = 0; i < arity; i++) {
				firm::set_Phi_pred(phi_mem, i + 1, phi_mem);
			}
		}

		return phi_mem;
	}
}

bool tailrec::optimize(firm_ir &) {
	_changed = false;
	size_t n = firm::get_irp_n_irgs();
	for (size_t i = 0; i < n; i++) {
		bool found = false;
		auto irg = firm::get_irp_irg(i);

		firm::edges_activate(irg);

		firm::ir_reserve_resources(irg, firm::IR_RESOURCE_IRN_LINK | firm::IR_RESOURCE_PHI_LIST);
		firm::collect_phiprojs_and_start_block_nodes(irg);

		auto list = find_tail_recursion(irg);
		// found returns to replace
		if (list.size() > 0) {
			auto ent = firm::get_irg_entity(irg);
			auto method_type = firm::get_entity_type(ent);

			auto info = tailrec_env();
			info.params.resize(static_cast<size_t>(firm::get_method_n_params(method_type)));
			firm::irg_walk_anchors(irg, [](firm::ir_node *node, void *env) {
				auto info = (tailrec_env*)env;
				auto skipped = node;
				if (firm::is_Proj(node)) {
					auto proj_pred = firm::get_Proj_pred(node);
					if (firm::is_Proj(proj_pred)) {
						auto start = firm::get_Proj_pred(proj_pred);
						if (firm::is_Start(start) && firm::get_Proj_num(proj_pred) == firm::pn_Start_T_args) {
							// node must be an method param
							info->params[static_cast<size_t>(firm::get_Proj_num(node))] = node;
						}
					}
					skipped = firm::skip_Proj(proj_pred);
				}
				// collect all nodes in start block
				if (firm::is_irn_start_block_placed(skipped)) {
					info->nodes_in_startblock.push_back(node);
				}
			}, nullptr, &info);


			// create a new start block and move the nodes
			auto start_block = firm::get_irg_start_block(irg);
			auto new_start_block = firm::new_r_Block(irg, 0, nullptr);
			for (auto start_node : info.nodes_in_startblock) {
				firm::set_nodes_block(start_node, new_start_block);
			}
			firm::set_irg_start_block(irg, new_start_block);

			// create the loop

			// start inputs and phi's
			auto start_ins = std::vector<firm::ir_node*>();
			auto mem_phi_ins = std::vector<firm::ir_node*>();
			// add initial values
			mem_phi_ins.push_back(firm::get_irg_initial_mem(irg));
			start_ins.push_back(firm::new_r_Jmp(new_start_block));
			for (auto &ret : list) {
				assert(firm::is_Return(ret));
				auto call = (firm::ir_node*)firm::get_irn_link(ret);
				assert(firm::is_Call(call));
				// add a jmp for every tail return
				start_ins.push_back(firm::new_r_Jmp(firm::get_nodes_block(ret)));
				// place the mems
				mem_phi_ins.push_back(firm::get_Call_mem(call));
			}
			firm::set_irn_in(start_block, static_cast<int>(start_ins.size()), start_ins.data());
			firm::add_End_keepalive(firm::get_irg_end(irg), start_block);

			// rewire the initial mem
			auto phi_mem = firm::new_r_Phi_loop(start_block, static_cast<int>(mem_phi_ins.size()), mem_phi_ins.data());
			auto mem = firm::get_irg_initial_mem(irg);
			phi_mem = fix_phi_mem(phi_mem, start_block);
			firm::exchange(mem, phi_mem);
			auto new_mem = firm::new_r_Proj(firm::get_irg_start(irg), firm::mode_M, firm::pn_Start_M);
			firm::set_Phi_pred(phi_mem, 0, new_mem);
			firm::set_irg_initial_mem(irg, new_mem);

			// add phis to
			auto args = firm::get_irg_args(irg);
			if (info.params.size() > 0) {
				int i = 0;
				for (auto param : info.params) {
					auto phi_in = std::vector<firm::ir_node*>();
					auto mode = firm::get_irn_mode(param);
					phi_in.push_back(param);
					for (auto &ret : list) {
						auto call = (firm::ir_node*)firm::get_irn_link(ret);
						phi_in.push_back(firm::get_Call_param(call, i));
					}
					auto phi = firm::new_r_Phi(start_block, static_cast<int>(phi_in.size()), phi_in.data(), mode);
					if (phi != param) {
						firm::exchange(param, phi);
						auto proj = firm::new_r_Proj(args, mode, static_cast<unsigned int>(i));
						firm::set_Phi_pred(phi, 0, proj);
					}
					i++;
				}
			}

			// finally replace old return nodes with bad nodes
			for (auto ret : list) {
				firm::exchange(ret, firm::new_r_Bad(irg, firm::mode_X));
			}

			_changed = true;
			found = true;
		}

		firm::edges_deactivate(irg);
		firm::ir_free_resources(irg, firm::IR_RESOURCE_IRN_LINK | firm::IR_RESOURCE_PHI_LIST);

		if (found) {
			firm::remove_bads(irg);
			assert(firm::irg_verify(irg));
		}
	}
	return _changed;
}