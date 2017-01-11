#include "opt/unused_params.hpp"
#include <set>
#include <iostream>

using namespace minijava::opt;

struct search_calls_info {
	search_calls_info(firm::ir_entity* search) : search{search} {}

	firm::ir_entity* search;
	std::vector<firm::ir_node*> result;
};

void replace_call_node(firm::ir_node* call_node, std::vector<unsigned int> params_to_keep, firm::ir_type* new_method_type)
{
	(void)call_node;
	(void)params_to_keep;
	(void)new_method_type;
}

void remove_unused_params(firm::ir_entity* method, std::vector<unsigned int> params_to_keep)
{
	auto irg = firm::get_entity_irg(method);
	std::cout << std::endl << firm::get_entity_ident(method) << std::endl;

	// todo: create new type and entity (remove old ones maybe not possible..)
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
	//clone_irg(irg, new_irg);

	firm::dump_ir_graph(new_irg, "cloned");
	firm::irg_verify(new_irg);
	std::cout << "created new entity " << firm::get_entity_ident(new_method_entity) << " from " << firm::get_entity_ident(method) << std::endl;

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
				replace_call_node(call_node, params_to_keep, new_method_type);
			}
		}
	}
}

bool unused_params::optimize(firm_ir& /*ir*/)
{
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

	return false;
}