#include <firm.hpp>
#include "opt/tailrec.hpp"

#include <map>
#include <iostream>

using namespace minijava::opt;


namespace /* anonymous */
{

	void find_tail_recursion(firm::ir_graph *irg)
	{
		auto end_block = firm::get_irg_end_block(irg);
		for (int i = 0, n = firm::get_irn_arity(end_block); i < n; i++) {
			auto ret = firm::get_irn_n(end_block, i);
			// shouldn't happen.. just skip
			if (!firm::is_Return(ret)) continue;

			auto mem = firm::get_Return_mem(ret);
			if (!firm::is_Proj(mem)) continue;
			auto call = firm::get_Proj_pred(mem);
			if (!firm::is_Call(call)) continue;
			// at this point, we found an call node as return value
			// check, if we call us and we are in the same node
			if (firm::get_nodes_block(ret) != firm::get_nodes_block(call)) continue;
			auto callee = firm::get_Call_callee(call);
			if (callee == nullptr || firm::get_entity_linktime_irg(callee) != irg) continue;

			std::cout << "found tail rec in " << firm::get_entity_ident(firm::get_irg_entity(irg)) << std::endl;
		}
	}

}

bool tailrec::optimize(firm_ir &) {
	_changed = false;
	size_t n = firm::get_irp_n_irgs();
	for (size_t i = 0; i < n; i++) {
		auto irg = firm::get_irp_irg(i);

		firm::edges_activate(irg);

		firm::ir_reserve_resources(irg, firm::IR_RESOURCE_IRN_LINK | firm::IR_RESOURCE_PHI_LIST);
		firm::collect_phiprojs_and_start_block_nodes(irg);

		find_tail_recursion(irg);

		firm::edges_deactivate(irg);
		firm::ir_free_resources(irg, firm::IR_RESOURCE_IRN_LINK | firm::IR_RESOURCE_PHI_LIST);
	}
	return _changed;
}