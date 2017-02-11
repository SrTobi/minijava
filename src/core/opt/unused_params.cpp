#include "opt/unused_params.hpp"
#include <set>
#include <iostream>

using namespace minijava::opt;

namespace /* anonymous */
{
	struct search_calls_info {
		search_calls_info(firm::ir_entity *search) : search{search} {}

		firm::ir_entity *search;
		std::vector<firm::ir_node *> result;
	};

	void copy_nodes(firm::ir_node *node, void *env) {
		copy_irn_to_irg(node, (firm::ir_graph *) env);
	}

	void set_preds(firm::ir_node *node, void *env) {
		auto new_irg = (firm::ir_graph *) env;
		auto nn = (firm::ir_node *) firm::get_irn_link(node);

		if (firm::is_Block(node)) {
			auto irg = firm::get_irn_irg(node);
			auto end_block = firm::get_irg_end_block(irg);
			for (int i = firm::get_Block_n_cfgpreds(node); i-- > 0;) {
				auto pred = firm::get_Block_cfgpred(node, i);
				if (end_block == node) {
					firm::add_immBlock_pred(firm::get_irg_end_block(new_irg),
					                        (firm::ir_node *) firm::get_irn_link(pred));
				} else {
					firm::set_Block_cfgpred(nn, i, (firm::ir_node *) firm::get_irn_link(pred));
				}
			}
		} else {
			firm::set_nodes_block(nn, (firm::ir_node *) firm::get_irn_link(firm::get_nodes_block(node)));
			if (firm::is_End(node)) {
				for (int i = 0, nodes = firm::get_End_n_keepalives(node); i < nodes; ++i) {
					firm::add_End_keepalive(nn, (firm::ir_node *) firm::get_irn_link(firm::get_End_keepalive(node, i)));
				}
			} else {
				// #foreach_irn_in_r macro from irnode_t.h
				for (bool pred__b = true; pred__b;) {
					for (firm::ir_node *pred__irn = node; pred__b; pred__b = false) {
						for (int idx = firm::get_irn_arity(pred__irn); pred__b && idx-- != 0;) {
							for (firm::ir_node *pred = (pred__b = false, firm::get_irn_n(pred__irn,
							                                                             idx)); !pred__b; pred__b = true) {
								set_irn_n(nn, idx, (firm::ir_node *) firm::get_irn_link(pred));
							}
						}
					}
				}
			}
		}
	}

	void clone_irg(firm::ir_graph *from, firm::ir_graph *to) {
		firm::irg_walk_graph(from, copy_nodes, set_preds, to);
		firm::irg_finalize_cons(to);
	}

	void replace_call_node(firm::ir_node *call_node, std::vector<unsigned int> params_to_keep,
	                       firm::ir_entity *new_method_entity) {
		auto params = std::vector<firm::ir_node*>();
		//auto params = new firm::ir_node *[params_to_keep.size()];
		for (unsigned int i = 0; i < params_to_keep.size(); i++) {
			params.push_back(firm::get_Call_param(call_node, static_cast<int>(params_to_keep[i])));
		}
		auto new_call_node = firm::new_r_Call(
				firm::get_nodes_block(call_node),
				firm::get_Call_mem(call_node),
				firm::new_r_Address(firm::get_irn_irg(call_node), new_method_entity),
				static_cast<int>(params.size()),
				params.data(),
				firm::get_entity_type(new_method_entity)
		);
		firm::exchange(call_node, new_call_node);
	}

	/**
	 * rewires old param projs to new order
	 * @param irg
	 * @param params_to_keep
	 */
	void update_params(firm::ir_graph *irg, std::vector<unsigned int> params_to_keep) {
		auto args = firm::get_irg_args(irg);
		firm::edges_activate(irg);
		for (unsigned int num = 0; num < params_to_keep.size(); num++) {
			auto old_num = params_to_keep[num];
			for (auto &out : get_out_edges_safe(args)) {
				if (firm::is_Proj(out.first)) {
					if (firm::get_Proj_num(out.first) == old_num) {
						firm::set_Proj_num(out.first, num);
					}
				}
			}
		}
		firm::edges_deactivate(irg);
	}

}

void unused_params::remove_unused_params(firm::ir_entity* method, std::vector<unsigned int> params_to_keep)
{
	auto irg = firm::get_entity_irg(method);
	auto method_type = firm::get_entity_type(method);
	auto new_method_type = firm::new_type_method(
			params_to_keep.size(),
			firm::get_method_n_ress(method_type),
			0,
			cc_cdecl_set,
			firm::mtp_no_property);
	// create new method type with only used parameters
	for (size_t i = 0, j = 0; i < params_to_keep.size(); i++) {
		firm::set_method_param_type(
				new_method_type,
				j++,
				firm::get_method_param_type(method_type, params_to_keep[i]));
	}
	if (firm::get_method_n_ress(method_type) > 0) {
		firm::set_method_res_type(new_method_type, 0, firm::get_method_res_type(method_type, 0));
	}
	auto new_method_entity = firm::new_entity(
			firm::get_entity_owner(method),
			firm::id_unique(firm::get_entity_ident(method)),
			new_method_type);
	auto new_irg = firm::new_ir_graph(
			new_method_entity,
			firm::get_irg_n_locs(irg));
	clone_irg(irg, new_irg);
	update_params(new_irg, params_to_keep);
	firm::irg_verify(new_irg);

	// change usage of existing calls
	auto n = firm::get_irp_n_irgs();
	for (size_t i = 0; i < n; i++) {
		auto irg = firm::get_irp_irg(i);
		if (irg) {
			auto env = search_calls_info(method);
			//env.search = method;
			firm::irg_walk_topological(irg, [](firm::ir_node *node, void *env) {
				auto info = (search_calls_info *) env;
				if (firm::is_Call(node)) {
					auto callee = firm::get_Call_callee(node);
					if (callee == info->search) {
						info->result.push_back(node);
					}
				}
			}, &env);
			for (auto call_node : env.result) {
				replace_call_node(call_node, params_to_keep, new_method_entity);
				_changed = true;
			}
		}
	}
}

bool unused_params::optimize(firm_ir& /*ir*/)
{
	_changed = false;
	auto n = firm::get_irp_n_irgs();
	for (size_t i = 0; i < n; i++) {
		auto irg = firm::get_irp_irg(i);
		auto method_entity = firm::get_irg_entity(irg);
		auto method_type = firm::get_entity_type(method_entity);
		auto param_count = firm::get_method_n_params(method_type);
		auto param_set = std::set<unsigned int>();
		// active back edges (for get_out_edges_safe)
		firm::edges_activate(irg);
		firm::ir_reserve_resources(irg, firm::IR_RESOURCE_IRN_LINK);
		// search params
		auto args = firm::get_irg_args(irg);
		for (auto &out : get_out_edges_safe(args)) {
			if (firm::is_Proj(out.first)) {
				param_set.insert(firm::get_Proj_num(out.first));
			}
		}
		// unused params found?
		if (param_set.size() < param_count) {
			auto params_to_keep = std::vector<unsigned int>(param_set.begin(), param_set.end());
			std::sort(std::begin(params_to_keep), std::end(params_to_keep));
			remove_unused_params(method_entity, params_to_keep);
		}
		firm::edges_deactivate(irg);
		firm::ir_free_resources(irg, firm::IR_RESOURCE_IRN_LINK);
		firm::remove_unreachable_code(irg);
		firm::remove_bads(irg);
	}

	return _changed;
}