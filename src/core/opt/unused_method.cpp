#include "opt/unused_method.hpp"

using namespace minijava::opt;

struct unused_method_info {
	unused_method_info(firm::ir_entity* entity) : irg_entity{entity} {}

	firm::ir_entity* irg_entity;
	bool found{false};
};

bool unused_method::optimize(firm_ir& /*ir*/)
{
	auto changed = false;
	auto n = firm::get_irp_n_irgs() - 1;
	for (size_t i = 0; i <= n; i++) {
		auto irg = firm::get_irp_irg(n - i);
		std::string name(firm::get_entity_ident(firm::get_irg_entity(irg)));
		// never remove main!
		if (name == "minijava_main") {
			continue;
		}
		// search after call
		auto info = unused_method_info(firm::get_irg_entity(irg));
		firm::all_irg_walk([](firm::ir_node* node, void* env) {
			auto info = (unused_method_info*)env;
			if (firm::is_Call(node)) {
				auto callee = firm::get_Call_callee(node);
				if (callee == info->irg_entity) {
					info->found = true;
				}
			}
		}, nullptr, &info);
		// if no call was found -> remove irg
		if (!info.found) {
			firm::free_ir_graph(irg);
			changed = true;
		}
	}

	return changed;
}
