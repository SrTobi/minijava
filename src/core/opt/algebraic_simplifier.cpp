#include "opt/algebraic_simplifier.hpp"
#include "opt/tarval.hpp"
#include <iostream>

using namespace minijava::opt;

firm::ir_tarval* get_tarval(firm::ir_node* node, int n)
{
	if (n < firm::get_irn_arity(node)) {
		return (firm::ir_tarval*)firm::get_irn_link(firm::get_irn_n(node, n));
	}
	return nullptr;
}

bool is_tarval_numeric(firm::ir_tarval* val)
{
	return firm::get_mode_arithmetic(firm::get_tarval_mode(val)) == firm::irma_twos_complement;
}

bool is_tarval_with_num(firm::ir_tarval* val, long num)
{
	return is_tarval_numeric(val) && firm::get_tarval_long(val) == num;
}

void minijava::opt::algebraic_simplifier::exchange_walker(firm::ir_node* node, void* env) {
	auto as = (algebraic_simplifier*)env;
	auto opcode = firm::get_irn_opcode(node);
	if (opcode != firm::iro_Const) {
		firm::ir_tarval* tv = (firm::ir_tarval*)firm::get_irn_link(node);
		if (tv) {
			auto new_node = firm::new_r_Const_long(as->_irg, firm::get_irn_mode(node), firm::get_tarval_long(tv));
			firm::exchange(node, new_node);
			// mark optimization as changed
			as->_changed = true;
		}
	}
}

void minijava::opt::algebraic_simplifier::algebraic_walker(firm::ir_node* node, void* /*env*/) {
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
	}
	firm::set_irn_link(node, ret_tv);
}

bool minijava::opt::algebraic_simplifier::optimize(firm_ir &ir) {
	(void)ir;
	auto n = firm::get_irp_n_irgs();
	for (size_t i = 0; i < n; i++) {
		_irg = firm::get_irp_irg(i);
		firm::ir_reserve_resources(_irg, firm::IR_RESOURCE_IRN_LINK);
		void* env = this;
		firm::irg_walk_topological(_irg, algebraic_walker, env);
		firm::irg_walk_topological(_irg, exchange_walker, env);
		firm::ir_free_resources(_irg, firm::IR_RESOURCE_IRN_LINK);
	}

	return _changed;
}
