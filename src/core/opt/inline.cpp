#include "opt/inline.hpp"

using namespace minijava::opt;

bool try_inline(firm::ir_node* /*call_node*/, firm::ir_graph* irg)
{
	auto info = inliner::inline_info();
	// collect inline_info
	firm::irg_walk_topological(irg, [](firm::ir_node* node, void* env) {
		auto info = (inliner::inline_info*)env;
		if (firm::is_Return(node)) {
			auto mem_pred = firm::get_Return_mem(node);
			info->return_count++;
			// no mem needed? => mem node from start -> return
			if (firm::is_Proj(mem_pred) && firm::is_Start(firm::get_Proj_pred(mem_pred))) {
				info->no_return_mem_count++;
			}
			// tarval exists?
			if (firm::get_Return_n_ress(node) > 0) {
				auto return_val_node = firm::get_Return_res(node, 0);
				if (firm::is_Const(return_val_node)) {
					info->tarval = firm::get_Const_tarval(return_val_node);
				}
			}
		} else if (firm::is_Phi(node)) {
			info->phi_count++;
		}
	}, &info);
	return false;
}

bool inliner::handle(firm::ir_node *node)
{
	// treat Call nodes
	if (firm::get_irn_opcode(node) == firm::iro_Call) {
		auto callee = firm::get_Call_callee(node);
		auto irg = firm::get_entity_irg(callee);
		// only optimize calls to entities with an irg (our framework functions dont have one)
		if (irg) {
			// todo..
			return try_inline(node, irg);
		}
	}
	return _changed;
}

void inliner::cleanup(firm::ir_node* /*node*/)
{
}