#include "opt/conditional.hpp"

#include <iostream>

using namespace minijava::opt;

void minijava::opt::conditional::cleanup(firm::ir_node* node) {
	auto opcode = firm::get_irn_opcode(node);
	if (opcode == firm::iro_Const) {
		return;
	}

	if (opcode == firm::iro_Cmp) {
		// wait for following mux
	} else if (opcode == firm::iro_Mux) {
		firm::ir_tarval* tv = (firm::ir_tarval*)firm::get_irn_link(node);
		if (tv == firm::tarval_b_true) {
			firm::exchange(node, firm::get_irn_n(node, firm::n_Mux_true));
		} else if (tv == firm::tarval_b_false) {
			firm::exchange(node, firm::get_irn_n(node, firm::n_Mux_false));
		}
	} else if (opcode == firm::iro_Phi) {
		//if (tv) {
		//	// todo?
		//}
	} else if (opcode == firm::iro_Proj) {
		auto cond = firm::get_Proj_pred(node);
		if (firm::is_Cond(cond)) {
			auto block = firm::get_nodes_block(get_irn_n(cond, 0));
			auto tv = (firm::ir_tarval*)firm::get_irn_link(cond);
			//(void)block;
			if (tv == firm::tarval_b_false || tv == firm::tarval_b_true) {
				firm::ir_node *new_node;
				if ((firm::get_Proj_num(node) == firm::pn_Cond_true && tv == firm::tarval_b_true) ||
						(firm::get_Proj_num(node) == firm::pn_Cond_false && tv == firm::tarval_b_false)) {
					new_node = firm::new_r_Jmp(block);
				} else {
					new_node = firm::new_r_Bad(firm::get_irn_irg(node), firm::mode_X);
				}
				firm::exchange(node, new_node);
			}
		}
	}
}

bool minijava::opt::conditional::handle(firm::ir_node* node) {
	auto opcode = firm::get_irn_opcode(node);
	firm::ir_tarval *ret_tv = nullptr;

	bool changed{false};
	if (opcode == firm::iro_Cmp) {
		auto lhs = firm::get_irn_n(node, 0);
		auto rhs = firm::get_irn_n(node, 1);
		if (firm::is_Const(lhs) && firm::is_Const(rhs)) {
			auto lhs_tv = firm::get_Const_tarval(lhs);
			auto rhs_tv = firm::get_Const_tarval(rhs);

			ret_tv = firm::tarval_cmp(lhs_tv, rhs_tv) & firm::get_Cmp_relation(node) ?
			         firm::tarval_b_true : firm::tarval_b_false;
		} else if (lhs == rhs) {
			auto rel = firm::get_Cmp_relation(node);
			if (rel == firm::ir_relation_equal
				    || rel == firm::ir_relation_greater_equal
					|| rel == firm::ir_relation_greater_equal) {
				ret_tv = firm::tarval_b_true;
			} else if (rel == firm::ir_relation_less_greater) {
				ret_tv = firm::tarval_b_false;
			}
		} else if ((firm::is_Mux(rhs) && firm::is_Const(lhs)) ||
				(firm::is_Mux(lhs) && firm::is_Const(rhs))) {
			auto mux = firm::is_Mux(lhs) ? lhs : rhs;
			//auto cons = firm::is_Const(lhs) ? lhs : rhs;
			auto sel = firm::get_Mux_sel(mux);
			if (firm::is_Cmp(sel) && firm::get_Cmp_relation(node) == firm::ir_relation_equal) {
				// TODO? Additional checks needed?
				// in this case, we should replace node directly with our sel compare node
				// the current node is just for conversion
				// dont wait for cleanup and do it directly here, because we just store tv in link
				firm::exchange(node, sel);
			}
		}
	} else if (opcode == firm::iro_Mux) {
		// pass tv from sel node to mux node
		auto sel = get_tarval(node, 0);
		if (sel) {
			ret_tv = sel;
			changed = true;
		}
	} else if (opcode == firm::iro_Cond) {
		auto cmp = firm::get_Cond_selector(node);
		if (firm::is_Cmp(cmp)) {
			auto tv = (firm::ir_tarval*)firm::get_irn_link(cmp);
			if (tv) {
				ret_tv = tv;
			}
		}
	} else if (opcode == firm::iro_Phi) {
		auto is_bad = false;
		auto found = false;
		firm::ir_tarval *val = nullptr;
		for (int i = 0, n = firm::get_irn_arity(node); i < n; i++) {
			auto tv = get_tarval(node, i);
			if (!tv) {
				found = false;
				break;
			} else if (tv == firm::tarval_bad) {
				is_bad = true;
				break;
			} else {
				if (found == false) {
					found = true;
					val = tv;
				} else if (val != tv) {
					is_bad = true;
				}
			}
		}
		if (is_bad) {
			ret_tv = firm::tarval_bad;
		} else if (found) {
			ret_tv = val;
		}
	}
	firm::set_irn_link(node, ret_tv);
	return changed;
}
