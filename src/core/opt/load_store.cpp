#include "opt/load_store.hpp"

#include <set>

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

	bool is_mem_access(firm::ir_node* node) {
		return firm::is_Load(node) || firm::is_Store(node);
	}

	// checks if two address nodes point to the same memory
	bool is_always_same_target(firm::ir_node* first, firm::ir_node* second) {
		if (first == second)
			return true;

		if(firm::is_Member(first) && is_Member(second)) {
			return firm::get_Member_entity(first) == firm::get_Member_entity(second)
			    && firm::get_Member_ptr(first) == firm::get_Member_ptr(second);
		}
		return false;
	}

	bool have_always_same_target(firm::ir_node* first, firm::ir_node* second) {
		assert(is_mem_access(first));
		assert(is_mem_access(second));

		return is_always_same_target(firm::get_irn_n(first, 1), firm::get_irn_n(second, 1));
	}

	bool is_always_different_target(firm::ir_node* first, firm::ir_node* second) {
		return firm::is_Member(first) && is_Member(second)
		    && firm::get_Member_entity(first) != firm::get_Member_entity(second);
	}

	bool have_always_different_target(firm::ir_node* first, firm::ir_node* second) {
		assert(is_mem_access(first));
		assert(is_mem_access(second));

		return is_always_different_target(firm::get_irn_n(first, 1), firm::get_irn_n(second, 1));
	}

	bool never_interferes_with_load_store(firm::ir_node* node) {
		if(!firm::is_Call(node))
			return false;

		static std::set<std::string> funcs = {
			"mj_runtime_new",
			"mj_runtime_id",
			"mj_runtime_exit",
			"mj_runtime_println",
			"mj_runtime_write",
			"mj_runtime_flush",
			"mj_runtime_read"
		};
		std::string name = firm::get_entity_name(firm::get_Call_callee(node));

		return funcs.count(name);
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
		if (!have_always_same_target(first, second))
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
		if (!have_always_same_target(store, load))
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

		// go the memory chain up and search a load or store that
		// maybe has the same memory target as node.
		auto pred_node = node;
		do {
			const auto pred_proj = firm::get_irn_n(pred_node, 0);
			if (!firm::is_Proj(pred_proj))
				return false;
			pred_node = firm::get_irn_n(pred_proj, 0);
		} while(never_interferes_with_load_store(pred_node)
		     || (is_mem_access(pred_node) && have_always_different_target(node, pred_node)));


		if (firm::is_Load(pred_node)) {
			return handle_load_load(pred_node, node);
		} else if (firm::is_Store(pred_node)) {
			return handle_store_load(pred_node, node);
		}

		return false;
	}

	bool handle_store_store(firm::ir_node* first, firm::ir_node* second) {
		// check if both store to the same memory location
		if (!have_always_same_target(first, second))
			return false;

		const auto mem_origin = firm::get_irn_n(first, 0);

		// check that the memory is really memory
		if (firm::get_irn_mode(mem_origin) != firm::mode_M)
			return false;

		// it is indeed irrelevant what the first store stores,
		// because it is overwritten by the second store.

		// remove first store
		remove_node(first);
		return true;
	}

	bool handle_load_store(firm::ir_node* load, firm::ir_node* store) {
		// check if both store to the same memory location
		if (!have_always_same_target(load, store))
			return false;

		// check if the store stores the value, that the load loaded
		const auto proj = firm::get_irn_n(store, 2);
		if(!firm::is_Proj(proj) || firm::get_irn_n(proj, 0) != load)
			return false;

		remove_node(store);
		return true;
	}

	bool has_multiple_mem_outputs(firm::ir_node* node) {
		return firm::get_irn_mode(node) == firm::mode_M && get_out_edges_safe(node).size() > 1;
	}

	bool handle_store(firm::ir_node* node) {
		// go the memory chain up and search a load or store that
		// maybe has the same memory target as node.
		bool mem_branched = false;
		auto pred_node = node;
		do {
			const auto pred_proj = firm::get_irn_n(pred_node, 0);
			if (!firm::is_Proj(pred_proj))
				return false;
			pred_node = firm::get_irn_n(pred_proj, 0);
			mem_branched = mem_branched || has_multiple_mem_outputs(pred_proj);
		} while(never_interferes_with_load_store(pred_node)
		     || (is_mem_access(pred_node) && have_always_different_target(node, pred_node)));

		if (firm::is_Load(pred_node)) {
			return handle_load_store(pred_node, node);
		} else if (firm::is_Store(pred_node) && !mem_branched) {
			// do not do try to optimize the first store,
			// when the control flow branched away in between both stores
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
