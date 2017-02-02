#include <firm.hpp>
#include "opt/unroll.hpp"

#include <iostream>

using namespace minijava::opt;

bool is_in_loop(firm::ir_node *node, firm::ir_loop *loop)
{
	if (firm::is_Block(node)) {
		return firm::get_irn_loop(node) == loop;
	}
	return firm::get_irn_loop(firm::get_nodes_block(node)) == loop;
}

void find_inner_loops(firm::ir_loop *loop, std::vector<firm::ir_loop*> *list)
{
	for (size_t i = 0, n = firm::get_loop_n_elements(loop); i < n; i++) {
		auto element = firm::get_loop_element(loop, i);
		if (*element.kind == firm::k_ir_loop) {
			find_inner_loops(element.son, list);
		}
	}
	list->push_back(loop);
}

std::vector<firm::ir_loop*> find_loops(firm::ir_graph *irg)
{
	auto loops = std::vector<firm::ir_loop*>();
	auto loop = firm::get_irg_loop(irg);
	auto elements = firm::get_loop_n_elements(loop);
	for (size_t i = 0; i < elements; i++) {
		auto element = firm::get_loop_element(loop, i);
		if (*element.kind == firm::k_ir_loop) {
			find_inner_loops(element.son, &loops);
		}
	}
	return loops;
}

void collect_loop_info(firm::ir_node *node, void *env)
{
	auto info = (unroll::loop_info*)env;
	if (!is_in_loop(node, info->loop)) {
		return;
	}

	if (firm::is_Call(node)) {
		info->call_count++;
		info->node_count++;
	} else if (firm::is_Phi(node)) {
		info->phi_count++;
		info->node_count++;
	} else if (!is_nop(node)) {
		info->node_count++;
	}

	for (int i = 0, n = firm::get_irn_arity(node); i < n; i++) {

	}
}

void optimize_loop(firm::ir_graph *irg, firm::ir_loop *loop)
{
	auto info = unroll::loop_info();
	info.loop = loop;

	firm::irg_walk_graph(irg, collect_loop_info, nullptr, &info);
}

bool unroll::optimize(firm_ir &) {
	size_t n = firm::get_irp_n_irgs();
	for (size_t i = 0; i < n; i++) {
		auto irg = firm::get_irp_irg(i);
		firm::assure_irg_properties(irg, firm::IR_GRAPH_PROPERTY_CONSISTENT_LOOPINFO);


		firm::ir_reserve_resources(irg, firm::IR_RESOURCE_IRN_LINK | firm::IR_RESOURCE_PHI_LIST);
		firm::collect_phiprojs_and_start_block_nodes(irg);

		auto loops = find_loops(irg);
		for (auto loop : loops) {
			optimize_loop(irg, loop);
			assert(false);
		}

		firm::ir_free_resources(irg, firm::IR_RESOURCE_IRN_LINK | firm::IR_RESOURCE_PHI_LIST);
	}
	return false;
}