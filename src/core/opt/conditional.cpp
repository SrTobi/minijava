#include "opt/conditional.hpp"

#include <iostream>

using namespace minijava::opt;

void minijava::opt::conditional::cleanup(firm::ir_node* node) {
	auto opcode = firm::get_irn_opcode(node);
	if (opcode == firm::iro_Const) {
		return;
	}

	firm::ir_tarval* tv = (firm::ir_tarval*)firm::get_irn_link(node);
	if (opcode == firm::iro_Cmp) {
		// wait for following mux
	} else if (opcode == firm::iro_Mux) {
		if (tv) {
			if (tv == firm::tarval_b_true) {
				firm::exchange(node, firm::get_irn_n(node, firm::n_Mux_true));
			} else {
				firm::exchange(node, firm::get_irn_n(node, firm::n_Mux_false));
			}
		}
	}
}

bool minijava::opt::conditional::handle(firm::ir_node* node) {
	auto opcode = firm::get_irn_opcode(node);
	firm::ir_tarval *ret_tv = nullptr;

	bool changed{false};
	if (opcode == firm::iro_Cmp) {
		auto lhs = get_tarval(node, 0);
		auto rhs = get_tarval(node, 1);

		if (lhs && rhs) {
			ret_tv = firm::tarval_cmp(lhs, rhs) & firm::get_Cmp_relation(node) ?
			         firm::tarval_b_true : firm::tarval_b_false;
		}
	} else if (opcode == firm::iro_Mux) {
		// pass tv from sel node to mux node
		auto sel = get_tarval(node, 0);
		if (sel) {
			ret_tv = sel;
			changed = true;
		}
	}
	firm::set_irn_link(node, ret_tv);
	return changed;
}
