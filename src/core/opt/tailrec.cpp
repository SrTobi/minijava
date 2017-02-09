#include <firm.hpp>
#include "opt/tailrec.hpp"

#include <map>
#include <iostream>

using namespace minijava::opt;


namespace /* anonymous */
{
	void find_tail_recursion(firm::ir_graph*) {

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