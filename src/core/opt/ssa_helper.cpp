#include "ssa_helper.hpp"

#include <iostream>

using namespace minijava::opt;

bool ssa_helper::optimize(firm_ir &) {

	auto n = firm::get_irp_n_irgs();
	for (size_t i = 0; i < n; i++) {
		auto irg = firm::get_irp_irg(i);

		firm::irg_walk_graph(irg, [](firm::ir_node *node, void*) {
			if (!firm::is_Block(node)) {
				return;
			}

			auto n = firm::get_irn_arity(node);
			for (int i = 0; i < n; i++) {
				auto pred = firm::get_irn_n(node, i);
				if (firm::is_Proj(pred)) {
					auto cond = firm::get_Proj_pred(pred);
					assert(firm::is_Cond(cond));
					for (int j = i + 1; j < n; j++) {
						auto other = firm::get_irn_n(node, j);
						if (firm::is_Proj(other)) {
							if (firm::get_Proj_pred(other) == cond) {
								// insert a new block after the false proj (wew could also use true - doesn't matter..)
								auto false_proj = firm::get_Proj_num(pred) == firm::pn_Cond_false ? pred : other;
								auto new_block = firm::new_r_Block(firm::get_irn_irg(node), 1, &false_proj);
								auto jmp = firm::new_r_Jmp(new_block);
								firm::set_irn_n(node, j, jmp);
								break;
							}
						}
					}
				}
			}
		}, nullptr, nullptr);

	}


	return false;
}
