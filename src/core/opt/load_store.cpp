#include "opt/load_store.hpp"

using namespace minijava::opt;

namespace /* anonymous */
{
	void replace_node(firm::ir_node *node, firm::ir_node* replace) {
		for (auto &out_edge : get_out_edges_safe(node)) {
			const auto out_child = out_edge.first;
			if (firm::get_irn_mode(out_child) == firm::mode_M) {
				for (auto &child_edge : get_out_edges_safe(out_child)) {
					firm::set_irn_n(child_edge.first, child_edge.second, firm::get_irn_n(node, 0));
				}
			}else if (replace) {
				firm::exchange(out_child, replace);
			}
		}
	}

	void remove_node(firm::ir_node* node) {
		replace_node(node, nullptr);
	}

	// checks if two address nodes point to the same memory
	bool is_same_target(firm::ir_node* first, firm::ir_node* second) {
		if (first == second)
			return true;

		if(firm::is_Member(first) && is_Member(second)) {
			return firm::get_Member_entity(first) == firm::get_Member_entity(second)
			    && firm::get_Member_ptr(first) == firm::get_Member_ptr(second);
		}
		return false;
	}

	firm::ir_node *get_res_node(firm::ir_node *node) {
		for (auto &out : get_out_edges_safe(node)) {
			if (firm::get_irn_mode(out.first) != firm::mode_M) {
				return out.first;
			}
		}
		return nullptr;
	}

	bool handle_load_load(firm::ir_node* first, firm::ir_node* second) {
		// check if they both load the same target
		if (!is_same_target(firm::get_irn_n(first, 1), firm::get_irn_n(second, 1)))
			return false;

		// remove second load and wire all children to the result of the first load (if that exists)
		const auto res = get_res_node(first);
		if(!res)
			return false;
		replace_node(second, res);
		return true;

	}

	bool handle_store_load(firm::ir_node* store, firm::ir_node* load) {
		// check if the load loads the same target as the store
		if (!is_same_target(firm::get_irn_n(load, 1), firm::get_irn_n(store, 1)))
			return false;

		// remove load and wire all children to the input of the store
		const auto input = firm::get_irn_n(store, 2);
		replace_node(load, input);
		return true;
	}

	bool handle_load(firm::ir_node *node) {
		auto res_node = get_res_node(node);
		if (!res_node) {
			// load never used
			remove_node(node);
			return true;
		}

		const auto pred_proj = firm::get_irn_n(node, 0);
		const auto pred_node = firm::get_irn_n(pred_proj, 0);

		if (firm::is_Load(pred_node)) {
			return handle_load_load(pred_node, node);
		} else if (firm::is_Store(pred_node)) {
			return handle_store_load(pred_node, node);
		}

		return false;
	}

	bool handle_store_store(firm::ir_node* first, firm::ir_node* second) {
		// check if both store to the same memory location
		if (!is_same_target(firm::get_irn_n(first, 1), firm::get_irn_n(second, 1)))
			return false;

		const auto mem_origin = firm::get_irn_n(first, 0);

		// check that the memory is really memory
		if (firm::get_irn_mode(mem_origin) != firm::mode_M)
			return false;

		// it is indeed irrelevant what the first store stores,
		// because it is overwritten by the second store.

		// remove first store
		firm::set_irn_n(second, 0, mem_origin);
		return true;
	}

	bool handle_load_store(firm::ir_node* load, firm::ir_node* store) {
		// check if both store to the same memory location
		if (!is_same_target(firm::get_irn_n(load, 1), firm::get_irn_n(store, 1)))
			return false;

		// check if the store stores the value, that the load loaded
		const auto proj = firm::get_irn_n(store, 2);
		if(!firm::is_Proj(proj) || firm::get_irn_n(proj, 0) != load)
			return false;

		remove_node(store);
		return true;
	}

	bool handle_store(firm::ir_node* node) {
		const auto pred_proj = firm::get_irn_n(node, 0);
		const auto pred_node = firm::get_irn_n(pred_proj, 0);

		if (firm::is_Load(pred_node)) {
			return handle_load_store(pred_node, node);
		} else if (firm::is_Store(pred_node)) {
			return handle_store_store(pred_node, node);
		}
		return false;
	}

	bool handle_node(firm::ir_node* node) {
		if (firm::get_irn_arity(node) > 0 && firm::is_Proj(firm::get_irn_n(node, 0))) {
			if (firm::is_Load(node)) {
				return handle_load(node);
			} else if (firm::is_Store(node)) {
				return handle_store(node);
			}
		}
		return false;
	}
}

bool load_store::handle(firm::ir_node* node)
{
	bool result = handle_node(node);
	_changed = result || _changed;
	return result;
}
