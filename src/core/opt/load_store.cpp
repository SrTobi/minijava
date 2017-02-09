#include "opt/load_store.hpp"

using namespace minijava::opt;

namespace /* anonymous */
{
	void remove_load_node(firm::ir_node *node) {
		for (auto &out_edge : get_out_edges_safe(node)) {
			if (firm::get_irn_mode(out_edge.first) == firm::mode_M) {
				for (auto &child_edge : get_out_edges_safe(out_edge.first)) {
					firm::set_irn_n(child_edge.first, child_edge.second, firm::get_irn_n(node, 0));
				}
			}
		}
	}

	firm::ir_node *get_res_node(firm::ir_node *node) {
		for (auto &out : get_out_edges_safe(node)) {
			if (firm::get_irn_mode(out.first) != firm::mode_M) {
				return out.first;
			}
		}
		return nullptr;
	}

	bool handle_load(firm::ir_node *node) {
		auto res_node = get_res_node(node);
		if (!res_node) {
			// load never used
			remove_load_node(node);
			return true;
		}
		return false;
	}
}

bool load_store::handle(firm::ir_node* node)
{
	if (firm::get_irn_arity(node) > 0 && firm::is_Proj(firm::get_irn_n(node, 0))) {
		if (firm::is_Load(node)) {
			return handle_load(node);
		} else if (firm::is_Store(node)) {
			// handle_store();
		}
	}
	return false;
}