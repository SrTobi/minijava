#include "opt/inline.hpp"
#include <iostream>

using namespace minijava::opt;

bool inliner::handle(firm::ir_node *node)
{
	// treat Call nodes
	if (firm::get_irn_opcode(node) == firm::iro_Call) {
		auto callee = firm::get_Call_callee(node);
		auto irg = firm::get_entity_irg(callee);
		// only optimize calls to entities with an irg (our framework functions dont have one)
		if (irg) {
			std::cout << "optimize call to:" << firm::get_entity_name(callee) << std::endl;
			// todo..
		}
	}
	return _changed;
}

void inliner::cleanup(firm::ir_node* /*node*/)
{
}