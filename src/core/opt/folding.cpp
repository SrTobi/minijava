#include "opt/folding.hpp"

#include <iostream>

using namespace minijava::opt;

namespace /* anonymous */
{
	firm::ir_tarval* get_tarval(firm::ir_node* node, int n)
	{
		if (n < firm::get_irn_arity(node)) {
			return (firm::ir_tarval*)firm::get_irn_link(firm::get_irn_n(node, n));
		}
		return nullptr;
	}
}

void minijava::opt::folding::cleanup(firm::ir_node* node) {
	auto opcode = firm::get_irn_opcode(node);
	if (opcode == firm::iro_Const) {
		return;
	}

	firm::ir_tarval* tv = (firm::ir_tarval*)firm::get_irn_link(node);
	if (tv && is_tarval_numeric(tv)) {
		auto new_node = firm::new_r_Const_long(_irg, firm::get_tarval_mode(tv), firm::get_tarval_long(tv));
		firm::set_irn_link(new_node, tv);
		// keep memory edges of div/mod nodes
		if (opcode == firm::iro_Div || opcode == firm::iro_Mod) {
			for (auto &out_edge : get_out_edges_safe(node)) {
				if (firm::get_irn_mode(out_edge.first) == firm::mode_M) {
					for (auto &child_edge : get_out_edges_safe(out_edge.first)) {
						firm::set_irn_n(child_edge.first, child_edge.second, firm::get_irn_n(node, 0));
					}
				} else {
					firm::exchange(out_edge.first, new_node);
				}
			}
		} else {
			firm::exchange(node, new_node);
		}
		// mark optimization as changed
		_changed = true;
	}
}

bool minijava::opt::folding::handle(firm::ir_node* node) {
	auto opcode = firm::get_irn_opcode(node);
	firm::ir_tarval *ret_tv = nullptr;
	if (opcode == firm::iro_Const) {
		// set link of const nodes to their tarval
		ret_tv = firm::get_Const_tarval(node);
	} else if (opcode == firm::iro_Add ||
			opcode == firm::iro_Sub ||
			opcode == firm::iro_Mul
			) {
		auto left_tv = get_tarval(node, 0);
		auto right_tv = get_tarval(node, 1);

		if (opcode == firm::iro_Add) {
			if (is_tarval_numeric(left_tv) && is_tarval_numeric(right_tv)) {
				ret_tv = firm::tarval_add(left_tv, right_tv);
			}
		} else if (opcode == firm::iro_Sub) {
			if (is_tarval_numeric(left_tv) && is_tarval_numeric(right_tv)) {
				ret_tv = firm::tarval_sub(left_tv, right_tv);
			} else if (firm::get_irn_n(node, 0) == firm::get_irn_n(node, 1)) {
				// X - X => 0
				ret_tv = firm::new_tarval_from_long(0, firm::get_irn_mode(node));
			}
		} else if (opcode == firm::iro_Mul) {
			if (is_tarval_numeric(left_tv) && is_tarval_numeric(right_tv)) {
				ret_tv = firm::tarval_mul(left_tv, right_tv);
			} else if (is_tarval_with_num(left_tv, 0) || is_tarval_with_num(right_tv, 0)) {
				// x * 0 || 0 * x
				ret_tv = firm::new_tarval_from_long(0, firm::get_irn_mode(node));
			}
		}
	} else if (opcode == firm::iro_Div || opcode == firm::iro_Mod) {
		// first child is memory node
		auto divident = get_tarval(node, 1);
		auto divisor = get_tarval(node, 2);

		if (is_tarval_with_num(divident, 0) || is_tarval_with_num(divisor, 0)) {
			// x / 0 (ub) || 0 / x
			ret_tv = firm::new_tarval_from_long(0, firm::get_irn_mode(node));
		} else if (firm::get_irn_n(node, 1) == firm::get_irn_n(node, 2)) {
			// x / x
			ret_tv = firm::new_tarval_from_long(1, firm::get_irn_mode(node));
		} else if (opcode == firm::iro_Div) {
			if (is_tarval_numeric(divident) && is_tarval_numeric(divisor)) {
				ret_tv = firm::tarval_div(divident, divisor);
			}
		} else if (opcode == firm::iro_Mod) {
			if (is_tarval_numeric(divident) && is_tarval_numeric(divisor)) {
				ret_tv = firm::tarval_mod(divident, divisor);
			} else if (is_tarval_with_num(divisor, 1) || is_tarval_with_num(divisor, -1)) {
				// x % 1 || x % -1
				ret_tv = firm::new_tarval_from_long(0, firm::get_irn_mode(node));
			}
		}
	} else if (opcode == firm::iro_Minus) {
		auto tv = get_tarval(node, 0);
		if (is_tarval_numeric(tv)) {
			ret_tv = firm::new_tarval_from_long(-firm::get_tarval_long(tv), firm::get_irn_mode(node));
		}
	} else if (opcode == firm::iro_Phi) {
		auto child_count = firm::get_irn_arity(node);
		auto is_bad = false;
		auto found = false;
		long val = 0;
		auto mode = firm::get_irn_mode(node);
		for (int i = 0; i < child_count && !is_bad; i++) {
			auto tv = get_tarval(node, i);
			if (!tv) {
				found = false;
				break;
			} else if (tv == firm::tarval_bad) {
				is_bad = true;
			} else if (mode == firm::get_tarval_mode(tv)) {
				auto cur = firm::get_tarval_long(tv);
				if (found == false) {
					found = true;
					val = cur;
				} else if (val != cur) {
					is_bad = true;
				}
			}
		}
		if (is_bad) {
			ret_tv = firm::tarval_bad;
		} else if (found) {
			ret_tv = firm::new_tarval_from_long(val, mode);
		}
	}
	if (!ret_tv) {
		return false;
	}
	firm::set_irn_link(node, ret_tv);
	if (opcode == firm::iro_Const) {
		// const nodes doesn't matter
		return false;
	}
	if (ret_tv == firm::tarval_bad) {
		return false;
	}
	auto cur_tv = (firm::ir_tarval*)firm::get_irn_link(node);
	if (cur_tv) {
		return firm::get_tarval_long(cur_tv) != firm::get_tarval_long(ret_tv);
	}
	return true;
}
